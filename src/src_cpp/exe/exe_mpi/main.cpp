// projet_benders.cpp�: d�finit le point d'entr�e pour l'application console.
//
#include "Worker.h"
#include "Timer.h"
#include "Benders.h"
#include "BendersMPI.h"
#include "launcher.h"
#include "JsonWriter.h"

int main(int argc, char** argv)
{
	google::InitGoogleLogging(argv[0]);
	google::SetLogDestination(google::GLOG_INFO, "./bendersmpiLog");
	LOG(INFO) << "starting bendersmpi" << std::endl;

	mpi::environment env(argc, argv);
	mpi::communicator world;

	JsonWriter jsonWriter_l;

	if (world.rank() == 0)
		usage(argc);

	BendersOptions options(build_benders_options(argc, argv));
	if (world.rank() > options.SLAVE_NUMBER + 1 && options.SLAVE_NUMBER != -1) {
		std::cout << "You need to have at least one slave by thread" << std::endl;
		exit(1);
	}
	if (world.rank() == 0) {
		std::ostringstream oss_l;
		options.print(oss_l);
		std::cout << oss_l.str();
		LOG(INFO) << oss_l.str() << std::endl;

		jsonWriter_l.write(options);
		jsonWriter_l.updateBeginTime();

	}

	world.barrier();//@FIXME here to wait for all processes to respect the beginTime ==> All Processes will start after the initialised beginTime
	if (world.size() == 1) {
		std::cout << "Sequential launch" << std::endl;
		LOG(INFO) << "Size is 1. Launching in sequential mode..." << std::endl;
		sequential_launch(options);
	}
	else {
		Timer timer;
		CouplingMap input;

		LOG(INFO) << "executor " << world.rank() << ": " << "Building input..." << std::endl;
		build_input(options, input);
		world.barrier();

		BendersMpi bendersMpi(env, world, options);
		LOG(INFO) << "executor " << world.rank() << ": " << "Loading benders..." << std::endl;
		bendersMpi.load(input, env, world);
		world.barrier();

		LOG(INFO) << "executor " << world.rank() << ": " << "running benders..." << std::endl;
		bendersMpi.run(env, world, std::cout);
		world.barrier();

		if (world.rank() == 0) {
			jsonWriter_l.updateEndTime();
			jsonWriter_l.write(input.size(), bendersMpi._trace, bendersMpi._data);
			jsonWriter_l.dump("out.json");
		}
		bendersMpi.free(env, world);
		world.barrier();

		if (world.rank() == 0) {
			std::cout << "Problem ran in " << timer.elapsed() << " seconds" << std::endl;
			jsonWriter_l.updateEndTime();
		}
	}
	return(0);
}
