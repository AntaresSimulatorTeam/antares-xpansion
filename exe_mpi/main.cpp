// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "Worker.h"
#include "Timer.h"
#include "Benders.h"
#include "BendersMPI.h"
#include "launcher.h"

int main(int argc, char** argv)
{
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
		options.print(std::cout);
	}
	if (world.size() == 1) {
		sequential_launch(options);
	}
	else {
		Timer timer;
		XPRSinit("");
		CouplingMap input;
		build_input(options, input);
		world.barrier();
		
		BendersMpi bendersMpi(env, world, options);
		bendersMpi.load(input, env, world);
		world.barrier();
		bendersMpi.run(env, world, std::cout);
		world.barrier();
		bendersMpi.free(env, world);
		XPRSfree();
		world.barrier();
		if (world.rank() == 0) {
			std::cout << "Problem ran in " << timer.elapsed() << " seconds" << std::endl;
		}
	}
	return(0);
}
