// projet_benders.cpp: defines the entry point  for the console application
//

#include "glog/logging.h"
#include "gflags/gflags.h"

#include "Worker.h"
#include "Timer.h"
#include "launcher.h"
#include "BendersSequential.h"
#include "BendersMPI.h"
#include "OutputWriter.h"
#include "JsonWriter.h"
#include "VoidWriter.h"
#include "logger/User.h"
#include "logger/UserFile.h"
#include "logger/Master.h"
#include "helpers/Path.h"

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

    if (world.rank() > options.SLAVE_NUMBER + 1 && options.SLAVE_NUMBER != -1)
    {
        std::cout << "You need to have at least one slave by thread" << std::endl;
        exit(1);
    }

    auto masterLogger = std::make_shared<xpansion::logger::Master>();
    Logger loggerUser = std::make_shared<xpansion::logger::User>(std::cout);
    std::string loggerFileName = (Path(options.OUTPUTROOT) / "reportbendersmpi.txt").get_str();
    Writer writer;

    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    auto path_to_log = (Path(options.OUTPUTROOT) / ("bendersmpiLog-rank" + std::to_string(world.rank()) + "-")).get_str();
    google::SetLogDestination(google::GLOG_INFO, path_to_log.c_str());

    if (world.rank() == 0)
    {
        Logger loggerFile = std::make_shared<xpansion::logger::UserFile>(loggerFileName);
        masterLogger->addLogger(loggerFile);
        masterLogger->addLogger(loggerUser);
        writer = std::make_shared<Output::JsonWriter>();

        LOG(INFO) << "starting bendersmpi" << std::endl;
        std::ostringstream oss_l;
        options.print(oss_l);
        LOG(INFO) << oss_l.str() << std::endl;
    }
    else
    {
        writer = std::make_shared<Output::VoidWriter>();
    }
    Logger logger = masterLogger;

    writer->initialize(options);

    world.barrier();
    Timer timer;
    pBendersBase benders;
    if (world.size() == 1)
    {
        std::cout << "Sequential launch" << std::endl;
        LOG(INFO) << "Size is 1. Launching in sequential mode..." << std::endl;

        benders = std::make_shared<BendersSequential>(options, logger, writer);
    }
    else
    {
        benders = std::make_shared<BendersMpi>(options, logger, writer, env, world);
    }

    benders->launch();
    logger->log_total_duration(timer.elapsed());
    writer->updateEndTime();
    return (0);
}
