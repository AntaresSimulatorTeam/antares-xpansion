// projet_benders.cpp : définit le point d'entrée pour l'application console.
//

#include "glog/logging.h"

#include "launcher.h"
#include "Benders.h"
#include "BendersOptions.h"
#include "logger/Master.h"
#include "logger/UserFile.h"
#include "logger/User.h"
#include "SequentialLaunch.h"
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
	// options.print(std::cout);
	usage(argc);
	BendersOptions options(build_benders_options(argc, argv));

	google::InitGoogleLogging(argv[0]);
	auto path_to_log = (Path(options.OUTPUTROOT) / "benderssequentialLog").get_str();
	google::SetLogDestination(google::GLOG_INFO, path_to_log.c_str());
	LOG(INFO) << "starting Benders Sequential" << std::endl;

	std::ostringstream oss_l;
	options.print(oss_l);
	LOG(INFO) << oss_l.str() << std::endl;

	LOG(INFO) << "Launching Benders Sequential" << std::endl;
	auto masterLogger = std::make_shared<xpansion::logger::Master>();

	const std::string &loggerFileName = (Path(options.OUTPUTROOT) / "reportbenderssequential").get_str();
	Logger loggerUser = std::make_shared<xpansion::logger::User>(std::cout);
	Logger loggerFile = std::make_shared<xpansion::logger::UserFile>(loggerFileName);
	masterLogger->addLogger(loggerUser);
	masterLogger->addLogger(loggerFile);

	Logger logger = masterLogger;

	sequential_launch(options, logger);

	char buff[FILENAME_MAX];
	GetCurrentDir(buff, FILENAME_MAX);

	std::stringstream str;
	str << "Optimization results available in : " << buff << Path::mSep
		<< Path(options.OUTPUTROOT) / (options.JSON_NAME + ".json");
	logger->display_message(str.str());

	return 0;
}
