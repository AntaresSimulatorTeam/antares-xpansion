// projet_benders.cpp : définit le point d'entrée pour l'application console.
//

#include "glog/logging.h"

#include "launcher.h"
#include "Worker.h"
#include "BendersOptions.h"
#include "JsonWriter.h"
#include "Timer.h"

#include "solver_utils.h"
#include "logger/User.h"
#include "helpers/Path.h"

//@suggest: create and move to standardlp.cpp
// Initialize static member
size_t StandardLp::appendCNT = 0;

int main(int argc, char **argv)
{
    usage(argc);
    BendersOptions options(build_benders_options(argc, argv));
    options.print(std::cout);

    Logger logger = std::make_shared<xpansion::logger::User>(std::cout);

    google::InitGoogleLogging(argv[0]);
    auto path_to_log = (Path(options.OUTPUTROOT) / "merge_mpsLog").get_str();
    google::SetLogDestination(google::GLOG_INFO, path_to_log.c_str());
    LOG(INFO) << "starting merge_mps" << std::endl;

    JsonWriter jsonWriter_l;
    jsonWriter_l.initialize(options);

    try
    {
        CouplingMap input = build_input(options);

        SolverFactory factory;
        std::string solver_to_use = (options.SOLVER_NAME == "COIN") ? "CBC" : options.SOLVER_NAME;
        SolverAbstract::Ptr mergedSolver_l = factory.create_solver(solver_to_use);
        mergedSolver_l->init();
        mergedSolver_l->set_output_log_level(options.LOG_LEVEL);

        int ncols(0);
        int nslaves = input.size() - 1;
        CouplingMap x_mps_id;
        int cntProblems_l(0);

        LOG(INFO) << "Merging problems..." << std::endl;
        for (auto const &kvp : input)
        {

            auto problem_name((Path(options.INPUTROOT) / (kvp.first + ".mps")).get_str());
            ncols = mergedSolver_l->get_ncols();

            SolverAbstract::Ptr solver_l = factory.create_solver(solver_to_use);
            solver_l->init();
            solver_l->set_output_log_level(options.LOG_LEVEL);
            solver_l->read_prob_mps(problem_name);

            if (kvp.first != options.MASTER_NAME)
            {

                int mps_ncols(solver_l->get_ncols());

                DblVector o(mps_ncols);
                IntVector sequence(mps_ncols);
                for (int i(0); i < mps_ncols; ++i)
                {
                    sequence[i] = i;
                }
                solver_getobj(solver_l, o, 0, mps_ncols - 1);
                double const weigth = options.slave_weight(nslaves, kvp.first);
                for (auto &c : o)
                {
                    c *= weigth;
                }
                solver_l->chg_obj(sequence, o);
            }
            StandardLp lpData(solver_l);
            std::string varPrefix_l = "prob" + std::to_string(cntProblems_l) + "_";

            lpData.append_in(mergedSolver_l, varPrefix_l);

            for (auto const &x : kvp.second)
            {
                int col_index = mergedSolver_l->get_col_index(varPrefix_l + x.first);
                if (col_index == -1)
                {
                    std::cerr << "missing variable " << x.first << " in " << kvp.first << " supposedly renamed to " << varPrefix_l + x.first << ".";
                    mergedSolver_l->write_prob_lp((Path(options.OUTPUTROOT) / "mergeError.lp").get_str());
                    mergedSolver_l->write_prob_mps((Path(options.OUTPUTROOT) / "mergeError.mps").get_str());
                    std::exit(1);
                }
                else
                {
                    x_mps_id[x.first][kvp.first] = col_index;
                }
            }

            ++cntProblems_l;
        }

        IntVector mstart;
        IntVector cindex;
        DblVector values;
        int nrows(0);
        int neles(0);
        size_t neles_reserve(0);
        size_t nrows_reserve(0);
        for (auto const &kvp : x_mps_id)
        {
            neles_reserve += kvp.second.size() * (kvp.second.size() - 1);
            nrows_reserve += kvp.second.size() * (kvp.second.size() - 1) / 2;
        }
        LOG(INFO) << "About to add " << nrows_reserve << " coupling constraints" << std::endl;
        values.reserve(neles_reserve);
        cindex.reserve(neles_reserve);
        mstart.reserve(nrows_reserve + 1);

        // adding coupling constraints
        for (auto const &kvp : x_mps_id)
        {
            std::string const var_name(kvp.first);
            LOG(INFO) << var_name << std::endl;
            bool is_first(true);
            int id1(-1);
            std::string first_mps;
            for (auto const &mps : kvp.second)
            {
                if (is_first)
                {
                    is_first = false;
                    first_mps = mps.first;
                    id1 = mps.second;
                }
                else
                {
                    int id2 = mps.second;
                    mstart.push_back(neles);
                    cindex.push_back(id1);
                    values.push_back(1);
                    ++neles;

                    cindex.push_back(id2);
                    values.push_back(-1);
                    ++neles;
                    ++nrows;
                }
            }
        }
        mstart.push_back(neles);

        DblVector rhs(nrows, 0);
        CharVector sense(nrows, 'E');
        solver_addrows(mergedSolver_l, sense, rhs, {}, mstart, cindex, values);

        LOG(INFO) << "Problems merged." << std::endl;
        LOG(INFO) << "Writting mps file" << std::endl;
        mergedSolver_l->write_prob_mps((Path(options.OUTPUTROOT) / "log_merged.mps").get_str());
        LOG(INFO) << "Writting lp file" << std::endl;
        mergedSolver_l->write_prob_lp((Path(options.OUTPUTROOT) / "log_merged.lp").get_str());

        mergedSolver_l->set_threads(16);

        logger->display_message("Solving...");
        Timer timer;
        int status_l = 0;
        if (mergedSolver_l->get_n_integer_vars() > 0)
        {
            status_l = mergedSolver_l->solve_mip();
        }
        else
        {
            status_l = mergedSolver_l->solve_lp();
        }

        logger->log_total_duration(timer.elapsed());

        jsonWriter_l.updateEndTime();

        Point x0;
        DblVector ptr(mergedSolver_l->get_ncols());
        double investCost_l(0);
        if (mergedSolver_l->get_n_integer_vars() > 0)
        {
            mergedSolver_l->get_mip_sol(ptr.data());
        }
        else
        {
            mergedSolver_l->get_lp_sol(ptr.data(), NULL, NULL);
        }

        std::vector<double> obj_coef(mergedSolver_l->get_ncols());
        mergedSolver_l->get_obj(obj_coef.data(), 0, mergedSolver_l->get_ncols() - 1);
        for (auto const &pairNameId : input[options.MASTER_NAME])
        {
            int varIndexInMerged_l = x_mps_id[pairNameId.first][options.MASTER_NAME];
            x0[pairNameId.first] = ptr[varIndexInMerged_l];
            investCost_l += x0[pairNameId.first] * obj_coef[varIndexInMerged_l];
        }

        double overallCost_l;
        if (mergedSolver_l->get_n_integer_vars() > 0)
        {
            overallCost_l = mergedSolver_l->get_mip_value();
        }
        else
        {
            overallCost_l = mergedSolver_l->get_lp_value();
        }
        double operationalCost_l = overallCost_l - investCost_l;

        bool optimality_l = (status_l == SOLVER_STATUS::OPTIMAL);
        jsonWriter_l.write(input.size(), overallCost_l,
                           overallCost_l, investCost_l, operationalCost_l,
                           overallCost_l, x0, optimality_l);
        jsonWriter_l.dump();
    }
    catch (std::exception &ex)
    {
        std::string error = "Exception raised and program stopped : " + std::string(ex.what());
        LOG(WARNING) << error << std::endl;
        logger->display_message(error);
        exit(1);
    }

    return 0;
}
