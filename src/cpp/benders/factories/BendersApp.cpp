
#include "antares-xpansion/benders/factories/BendersApp.h"

#include <antares-xpansion/benders/factories/BendersFactory.h>
#include <filesystem>
#include <fmt/format.h>

#include "antares-xpansion/benders/benders_by_batch/BendersByBatch.h"
#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/benders_core/MasterUpdate.h"
#include "antares-xpansion/benders/benders_core/StartUp.h"
#include "antares-xpansion/benders/benders_mpi/OuterLoopBenders.h"
#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/benders/factories/WriterFactories.h"
#include "antares-xpansion/core/ProblemFormatStream.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

void BendersApp::SetupLoggerAndOutputWriter(const BendersBaseOptions& benders_options)
{
    auto benders_log_console = benders_options.LOG_LEVEL > 0;
    if (pworld_->rank() == 0)
    {
        auto logger_factory = FileAndStdoutLoggerFactory(LogReportsName(), benders_log_console);
        logger_ = logger_factory.get_logger();
        math_log_driver_ = BuildMathLogger(benders_log_console);
        writer_ = build_json_writer(options_.JSON_FILE, options_.RESUME);
    }
    else
    {
        logger_ = build_void_logger();
        writer_ = build_void_writer();
        math_log_driver_ = MathLoggerFactory::get_void_logger();
    }
    benders_loggers_.AddLogger(logger_);
    benders_loggers_.AddLogger(math_log_driver_);
    writer_->write_log_level(options_.LOG_LEVEL);
    writer_->write_master_name(options_.MASTER_NAME);
    writer_->write_solver_name(options_.SOLVER_NAME);
    writer_->WriteProblemFormat(fmt::format("{}", options_.PROBLEMS_FORMAT));
}

bool BendersApp::isCriterionListEmpty() const
{
    return std::visit([](auto&& the_variant) { return the_variant.Criteria().empty(); },
                      criterion_input_holder_);
}

std::shared_ptr<MathLoggerDriver> BendersApp::BuildMathLogger(bool benders_log_console) const
{
    const std::filesystem::path output_root(options_.OUTPUTROOT);
    auto math_logs_file = output_root / "benders_solver.log";

    auto math_log_factory = MathLoggerFactory(method_, benders_log_console, math_logs_file);

    auto math_log_driver = math_log_factory.get_logger();

    return math_log_driver;
}

void BendersApp::AddCriterionOutputs()
{
    const std::filesystem::path output_root(options_.OUTPUTROOT);

    const auto& headers = std::visit([](auto&& the_variant) { return the_variant.PatternBodies(); },
                                     criterion_input_holder_);
    math_log_driver_->add_logger(output_root / LOLD_FILE,
                                 headers,
                                 &CriteriaCurrentIterationData::criteria);

    positive_unsupplied_file_ = std::visit([](auto&& the_variant)
                                           { return the_variant.PatternsPrefix() + ".txt"; },
                                           criterion_input_holder_);
    math_log_driver_->add_logger(output_root / positive_unsupplied_file_,
                                 headers,
                                 &CriteriaCurrentIterationData::patterns_values);
}

int BendersApp::RunBenders()
{
    try
    {
        SetupLoggerAndOutputWriter(options_.get_benders_options());
        BendersFactory factory(options_,
                               logger_,
                               writer_,
                               math_log_driver_,
                               pworld_->rank(),
                               penv_,
                               pworld_,
                               benders_loggers_);
        auto env = factory.PrepareForExecution(false);
        // context =
        // method =
        if (env)
        {
            auto&& environment = env.value();
            benders_ = std::move(environment.benders);
            criterion_input_holder_ = environment.criterion_input_data;
            method_ = environment.method;
            if (pworld_->rank() == 0)
            {
                if (!isCriterionListEmpty())
                {
                    AddCriterionOutputs();
                }
            }
            if (benders_)
            {
                StartMessage();
                benders_->launch();
                EndMessage(benders_->execution_time());
            }
        }
    }
    catch (std::exception& e)
    {
        std::ostringstream msg;
        msg << "error: " << e.what() << std::endl;
        benders_loggers_.display_message(msg.str());
        mpi::environment::abort(1);
    }
    catch (...)
    {
        std::ostringstream msg;
        msg << "Exception of unknown type!" << std::endl;
        benders_loggers_.display_message(msg.str());
        mpi::environment::abort(1);
    }
    return 0;
}

void BendersApp::StartMessage()
{
    std::ostringstream oss_l;
    oss_l << "Starting " << context_ << std::endl;
    options_.print(oss_l);
    oss_l << std::endl;
    benders_loggers_.display_message(oss_l.str());
}

void BendersApp::EndMessage(const double execution_time)
{
    std::ostringstream str;
    str << "Optimization results available in : " << options_.JSON_FILE << std::endl;
    benders_loggers_.display_message(str.str(), LogUtils::LOGLEVEL::INFO, context_);

    str.str("");

    str << context_ << " ran in " << execution_time << " s" << std::endl;
    benders_loggers_.display_message(str.str(), LogUtils::LOGLEVEL::INFO, context_);
}

int BendersApp::RunExternalLoop()
{
    try
    {
        SetupLoggerAndOutputWriter(options_.get_benders_options());
        BendersFactory factory(options_,
                               logger_,
                               writer_,
                               math_log_driver_,
                               pworld_->rank(),
                               penv_,
                               pworld_,
                               benders_loggers_);
        auto env = factory.PrepareForExecution(true);
        if (!env)
        {
            throw std::runtime_error(
              "Could not initialize benders. Please see above messages for actual error.");
        }
        auto&& environment = env.value();
        benders_ = std::move(environment.benders);
        criterion_input_holder_ = environment.criterion_input_data;
        method_ = environment.method;
        if (pworld_->rank() == 0)
        {
            if (!isCriterionListEmpty())
            {
                AddCriterionOutputs();
            }
        }
        double tau = 0.5;
        const auto& outer_loop_inputs = std::get<Benders::Criterion::OuterLoopCriterionInputData>(
          criterion_input_holder_);
        std::shared_ptr<Outerloop::IMasterUpdate> master_updater = std::make_shared<
          Outerloop::MasterUpdateBase>(benders_, tau, outer_loop_inputs.StoppingThreshold());
        std::shared_ptr<Outerloop::ICutsManager>
          cuts_manager = std::make_shared<Outerloop::CutsManagerRunTime>();

        Outerloop::OuterLoopBenders ext_loop(outer_loop_inputs.Criteria(),
                                             master_updater,
                                             cuts_manager,
                                             benders_,
                                             *pworld_);
        StartMessage();
        ext_loop.Run();
        EndMessage(ext_loop.Runtime());
    }
    catch (std::exception& e)
    {
        std::ostringstream msg;
        msg << "error: " << e.what() << std::endl;
        benders_loggers_.display_message(msg.str());
        mpi::environment::abort(1);
    }
    catch (...)
    {
        std::ostringstream msg;
        msg << "Exception of unknown type!" << std::endl;
        benders_loggers_.display_message(msg.str());
        mpi::environment::abort(1);
    }
    return 0;
}

BendersApp::BendersApp(const std::filesystem::path& options_file,
                       mpi::environment& env,
                       mpi::communicator& world,
                       const SOLVER& solver):
    penv_(&env),
    pworld_(&world),
    solver_(solver),
    options_(options_file)
{
}

std::filesystem::path BendersApp::LogReportsName() const
{
    return std::filesystem::path(options_.OUTPUTROOT) / "reportbenders.txt";
}

int BendersApp::Run()
{
    if (solver_ == SOLVER::BENDERS)
    {
        return RunBenders();
    }
    return RunExternalLoop();
}
