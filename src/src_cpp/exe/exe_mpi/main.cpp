// projet_benders.cpp�: d�finit le point d'entr�e pour l'application console.
//
#include "Worker.h"
#include "Timer.h"
#include "Benders.h"
#include "BendersMPI.h"
#include "launcher.h"

int main(int argc, char** argv)
{
	google::InitGoogleLogging(argv[0]);
	google::SetLogDestination(google::INFO, "./bendersmpiLog");
	LOG(INFO) << "starting bendersmpi" << std::endl;

	mpi::environment env(argc, argv);
	mpi::communicator world;

	if (world.rank() == 0)
		usage(argc);

	BendersOptions options(build_benders_options(argc, argv));
	if (world.rank() > options.SLAVE_NUMBER + 1 && options.SLAVE_NUMBER != -1) {
		std::cout << "You need to have at least one slave by thread" << std::endl;
		exit(0);
	}
	if (world.rank() == 0) {
		std::ostringstream oss_l;
		options.print(oss_l);
		std::cout << oss_l.str();
		LOG(INFO) << oss_l.str() << std::endl;
	}
	if (world.size() == 1) {
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
		bendersMpi.free(env, world);
		world.barrier();
		if (world.rank() == 0) {
			std::cout << "Problem ran in " << timer.elapsed() << " seconds" << std::endl;
		}
	}
	return(0);
}
