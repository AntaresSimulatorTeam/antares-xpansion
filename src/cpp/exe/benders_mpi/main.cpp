// projet_benders.cpp: defines the entry point  for the console application
//

#include "glog/logging.h"
#include "gflags/gflags.h"

#include "Worker.h"
#include "Timer.h"
#include "SequentialLaunch.h"
#include "BendersMPI.h"
#include "launcher.h"
#include "JsonWriter.h"
#include "logger/User.h"
#include "logger/UserFile.h"
#include "logger/Master.h"
#include "helpers/Path.h"

#if defined(WIN32) || defined(_WIN32)
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

int main(int argc, char **argv)
{
    mpi::environment env(argc, argv);
    mpi::communicator world;

    // First check usage (options are given)
    if (world.rank() == 0)
    {
        usage(argc);
    }

    // Read options, needed to have options.OUTPUTROOT
    BendersOptions options(build_benders_options(argc, argv));

    auto masterLogger = std::make_shared<xpansion::logger::Master>();
    Logger loggerUser = std::make_shared<xpansion::logger::User>(std::cout);
    auto loggerFileName = (Path(options.OUTPUTROOT) / "reportbendersmpi.txt").get_str();
    if (world.rank() == 0)
    {
        Logger loggerFile = std::make_shared<xpansion::logger::UserFile>(loggerFileName);
        masterLogger->addLogger(loggerFile);
    }
    masterLogger->addLogger(loggerUser);

    Logger logger = masterLogger;

    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    auto path_to_log = (Path(options.OUTPUTROOT) / ("bendersmpiLog-rank" + std::to_string(world.rank()) + "-")).get_str();
    google::SetLogDestination(google::GLOG_INFO, path_to_log.c_str());

    JsonWriter jsonWriter_l;
    if (world.rank() == 0)
    {
        LOG(INFO) << "starting bendersmpi" << std::endl;
        jsonWriter_l.write_failure();
        jsonWriter_l.dump(options.JSON_FILE);
    }

    if (world.rank() > options.SLAVE_NUMBER + 1 && options.SLAVE_NUMBER != -1)
    {
        std::cout << "You need to have at least one slave by thread" << std::endl;
        exit(1);
    }
    if (world.rank() == 0)
    {
        std::ostringstream oss_l;
        options.print(oss_l);
        LOG(INFO) << oss_l.str() << std::endl;

        jsonWriter_l.write(options);
        jsonWriter_l.updateBeginTime();
    }

    world.barrier();
    if (world.size() == 1)
    {
        std::cout << "Sequential launch" << std::endl;
        LOG(INFO) << "Size is 1. Launching in sequential mode..." << std::endl;

        sequential_launch(options, logger);
    }
    else
    {
        Timer timer;
        CouplingMap input = build_input(options);
        world.barrier();

        BendersMpi bendersMpi(options, logger, env, world);
        bendersMpi.load(input);
        world.barrier();

        bendersMpi.run();
        world.barrier();

        if (world.rank() == 0)
        {

            LogData logData = defineLogDataFromBendersDataAndTrace(bendersMpi._data, bendersMpi._trace);
            logData.optimality_gap = options.ABSOLUTE_GAP;
            logData.relative_gap = options.RELATIVE_GAP;
            logData.max_iterations = options.MAX_ITERATIONS;

            logger->log_at_ending(logData);

            jsonWriter_l.updateEndTime();
            jsonWriter_l.write(input.size(), bendersMpi._trace, bendersMpi._data, options.ABSOLUTE_GAP, options.RELATIVE_GAP, options.MAX_ITERATIONS);
            jsonWriter_l.dump(options.JSON_FILE);

            char buff[FILENAME_MAX];
            GetCurrentDir(buff, FILENAME_MAX);

            std::stringstream str;
            str << "Optimization results available in : "
                << options.JSON_FILE;
            logger->display_message(str.str());
        }
        bendersMpi.free();
        world.barrier();

        if (world.rank() == 0)
        {
            logger->log_total_duration(timer.elapsed());
            jsonWriter_l.updateEndTime();
        }
    }

    return (0);
}
