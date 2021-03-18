// projet_benders.cpp�: d�finit le point d'entr�e pour l'application console.
//

#include "glog/logging.h"

#include "Worker.h"
#include "Timer.h"
#include "Benders.h"
#include "BendersMPI.h"
#include "launcher.h"
#include "JsonWriter.h"

#if defined(WIN32) || defined(_WIN32)
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

int main(int argc, char** argv)
{
	mpi::environment env(argc, argv);
	mpi::communicator world;

	JsonWriter jsonWriter_l;

	if (world.rank() == 0)
	{
		gflags::ParseCommandLineFlags(&argc, &argv, true);

		google::InitGoogleLogging(argv[0]);

		google::SetLogDestination(google::GLOG_INFO, "./bendersmpiLog");
		LOG(INFO) << "starting bendersmpi" << std::endl;

		usage(argc);
	}

	BendersOptions options(build_benders_options(argc, argv));
	if (world.rank() > options.SLAVE_NUMBER + 1 && options.SLAVE_NUMBER != -1) {
		std::cout << "You need to have at least one slave by thread" << std::endl;
		exit(1);
	}
	if (world.rank() == 0) {
		std::ostringstream oss_l;
		options.print(oss_l);
		LOG(INFO) << oss_l.str() << std::endl;

		jsonWriter_l.write(options);
		jsonWriter_l.updateBeginTime();

	}

	world.barrier();
	if (world.size() == 1) {
		std::cout << "Sequential launch" << std::endl;
		LOG(INFO) << "Size is 1. Launching in sequential mode..." << std::endl;
		sequential_launch(options);
	}
	else {
		Timer timer;
		CouplingMap input;

		build_input(options, input);
		world.barrier();

		BendersMpi bendersMpi(env, world, options);
		bendersMpi.load(input, env, world);
		world.barrier();

		bendersMpi.run(env, world);
		world.barrier();

		if (world.rank() == 0) {
			last_solution_log(bendersMpi._data, options.GAP);
			jsonWriter_l.updateEndTime();
			jsonWriter_l.write(input.size(), bendersMpi._trace, bendersMpi._data);
			jsonWriter_l.dump("out.json");

			char buff[FILENAME_MAX];
			GetCurrentDir(buff, FILENAME_MAX);

			std::stringstream str;
			str << "Optimization results available in : " << buff << PATH_SEPARATOR << "out.json";
			LOG_INFO_AND_COUT(str.str());
		}
		bendersMpi.free(env, world);
		world.barrier();

		if (world.rank() == 0) {
			std::stringstream str;
			str << "Problem ran in " << timer.elapsed() << " seconds";
			LOG_INFO_AND_COUT(str.str());
			jsonWriter_l.updateEndTime();
		}
	}
	return(0);
}
