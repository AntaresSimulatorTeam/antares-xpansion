
#include "launch_mpi.h"
#include "launcher.h"
#include "benders_sequential_core/Benders.h"
#include "Timer.h"

#include "BendersMPI.h"
#include "BendersOptions.h"

#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#if defined(WIN32) || defined(_WIN32)
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

void run_mpibenders(mpi::communicator& world, mpi::environment& env, const BendersOptions& options, Logger& logger, JsonWriter& jsonWriter_l)
{
	Timer timer;
	CouplingMap input = build_input(options);
	world.barrier();

	BendersMpi bendersMpi(env, world, options,logger);
	bendersMpi.load(input, env, world);
	world.barrier();

	bendersMpi.run(env, world);
	world.barrier();

	if (world.rank() == 0) {

		LogData logData = defineLogDataFromBendersDataAndTrace(bendersMpi._data, bendersMpi._trace);
		logData.optimality_gap = options.ABSOLUTE_GAP;
		logData.relative_gap = options.RELATIVE_GAP;
		logData.max_iterations = options.MAX_ITERATIONS;

		logger->log_at_ending(logData);

		jsonWriter_l.updateEndTime();
		jsonWriter_l.write(input.size(), bendersMpi._trace, bendersMpi._data, options.ABSOLUTE_GAP, options.RELATIVE_GAP, options.MAX_ITERATIONS);
		jsonWriter_l.dump(options.OUTPUTROOT + PATH_SEPARATOR + options.JSON_NAME + ".json");

		char buff[FILENAME_MAX];
		GetCurrentDir(buff, FILENAME_MAX);

		std::stringstream str;
		str << "Optimization results available in : " << buff << PATH_SEPARATOR
			<< options.OUTPUTROOT + PATH_SEPARATOR + options.JSON_NAME + ".json";
		logger->display_message(str.str());
	}
	bendersMpi.free(env, world);
	world.barrier();

	if (world.rank() == 0) {
		logger->log_total_duration(timer.elapsed());
		jsonWriter_l.updateEndTime();
	}	
}


/*!
*  \brief wrap sequential_launch in a thread to control execution time
*
*  \param options : set of Benders options
*/
// void run_mpibenders_wrapper(mpi::communicator& world, mpi::environment& env, const BendersOptions& options, Logger& logger, JsonWriter& jsonWriter_l)
// {
// 	if (options.TIME_LIMIT > 0)
// 	{
// 		std::mutex m;
// 		std::condition_variable cv;

// 		std::thread t([&cv,&world, &env, &options, &logger, &jsonWriter_l]() 
// 		{
// 			run_mpibenders(world, env, options, logger, jsonWriter_l);
// 			cv.notify_one();
// 		});

// 		t.detach();

// 		{
// 			std::unique_lock<std::mutex> l(m);
// 			if(cv.wait_for(l,  std::chrono::seconds( options.TIME_LIMIT) ) == std::cv_status::timeout) 
// 			{

// 				throw std::runtime_error("Exit on timeout!");
// 			}
// 		}
// 	}
// 	else
// 	{
// 		run_mpibenders(world, env, options, logger, jsonWriter_l);
// 	}

// }