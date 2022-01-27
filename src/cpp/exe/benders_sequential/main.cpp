// projet_benders.cpp : définit le point d'entrée pour l'application console.
//

#include "glog/logging.h"

#include "launcher.h"
#include "BendersSequential.h"
#include "BendersOptions.h"
#include "logger/Master.h"
#include "logger/UserFile.h"
#include "logger/User.h"
#include "OutputWriter.h"
#include "JsonWriter.h"
#include "helpers/Path.h"
#include "LoggerFactories.h"
#include "WriterFactories.h"

int main(int argc, char **argv)
{
	// options.print(std::cout);
	usage(argc);
	BendersOptions options(build_benders_options(argc, argv));

	google::InitGoogleLogging(argv[0]);
	auto path_to_log = (Path(options.OUTPUTROOT) / "benderssequentialLog.txt.").get_str();
	google::SetLogDestination(google::GLOG_INFO, path_to_log.c_str());

	std::ostringstream oss_l = start_message(options, "Sequential");
	LOG(INFO) << oss_l.str() << std::endl;

	const std::string &loggerFileName = (Path(options.OUTPUTROOT) / "reportbenderssequential.txt").get_str();
	Logger logger = build_stdout_and_file_logger(loggerFileName);
	Writer writer = build_json_writer(options.JSON_FILE);
	Timer timer;

	BendersSequential benders(options, logger, writer);
	benders.launch();
	std::stringstream str;
	str << "Optimization results available in : "
		<< options.JSON_FILE;
	logger->display_message(str.str());
	logger->log_total_duration(timer.elapsed());

	return 0;
}
