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
	mpi::environment env(argc, argv);
	mpi::communicator world;

	JsonWriter jsonWriter_l;

	if (world.rank() == 0)
		usage(argc);

	BendersOptions options(build_benders_options(argc, argv));
	if (world.rank() > options.SLAVE_NUMBER + 1 && options.SLAVE_NUMBER != -1) {
		std::cout << "You need to have at least one slave by thread" << std::endl;
		exit(0);
	}
	if (world.rank() == 0) {
		jsonWriter_l.write(options);
		options.print(std::cout);
		jsonWriter_l.updateBeginTime();
	}

	world.barrier();//@FIXME here to wait for all processes to respect the beginTime ==> All Processes will start after the initialised beginTime
	if (world.size() == 1) {
		std::cout << "Sequential launch" << std::endl;
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

		bendersMpi.run(env, world, std::cout);
		world.barrier();

		if (world.rank() == 0) {
			jsonWriter_l.updateEndTime();
			jsonWriter_l.write(bendersMpi._trace, bendersMpi._data);
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
