#include "SequentialLaunch.h"
#include "JsonWriter.h"
#include "glog/logging.h"
#include "Benders.h"
#include "launcher.h"
#include "helpers/Path.h"

/*!
*  \brief Execute the Benders algorithm in sequential
*
*  \param options : set of Benders options
*/
void sequential_launch(BendersOptions const & options,  Logger & logger) {
	Timer timer;

	LOG(INFO) << "Building input" << std::endl;
    CouplingMap input = build_input(options);

	JsonWriter jsonWriter_l;
	jsonWriter_l.write_failure();
	jsonWriter_l.dump( static_cast<std::string>( Path(options.OUTPUTROOT) / (options.JSON_NAME + ".json") ) );

	jsonWriter_l.write(options);
	jsonWriter_l.updateBeginTime();

	LOG(INFO) << "Constructing workers..." << std::endl;

    Benders benders(options, logger);
	LOG(INFO) << "Running solver..." << std::endl;
    try {
	    benders.run(input);
	    LOG(INFO) << "Benders solver terminated." << std::endl;
    }catch (std::exception& ex) {
        std::string error = "Exception raised : " + std::string(ex.what());
        LOG(WARNING) << error << std::endl;
        logger->display_message(error);
    }

    LogData logData = defineLogDataFromBendersDataAndTrace(benders._data, benders._trace);
	logData.optimality_gap = options.ABSOLUTE_GAP;
	logData.relative_gap = options.RELATIVE_GAP;
	logData.max_iterations = options.MAX_ITERATIONS;

    logger->log_at_ending(logData);
	jsonWriter_l.updateEndTime();
	jsonWriter_l.write(input.size(), benders._trace, benders._data, options.ABSOLUTE_GAP, options.RELATIVE_GAP, options.MAX_ITERATIONS);
	jsonWriter_l.dump( static_cast<std::string>( Path(options.OUTPUTROOT) / (options.JSON_NAME + ".json") ) );

	benders.free();
	logger->log_total_duration(timer.elapsed());
}