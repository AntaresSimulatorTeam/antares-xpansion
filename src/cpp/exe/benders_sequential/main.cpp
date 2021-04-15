// projet_benders.cpp : définit le point d'entrée pour l'application console.
//

#include "glog/logging.h"

#include "launcher.h"
#include "Benders.h"
#include "BendersOptions.h"

#if defined(WIN32) || defined(_WIN32) 
#include <direct.h>
#define GetCurrentDir _getcwd
#define PATH_SEPARATOR "\\" 
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#define PATH_SEPARATOR "/" 
#endif

int main(int argc, char** argv)
{
	//options.print(std::cout);
	usage(argc);
	BendersOptions options(build_benders_options(argc, argv));

	google::InitGoogleLogging(argv[0]);
	std::string path_to_log = options.OUTPUTROOT + PATH_SEPARATOR + "benderssequentialLog";
	google::SetLogDestination(google::GLOG_INFO, path_to_log.c_str());
	LOG(INFO) << "starting Benders Sequential" << std::endl;

	std::ostringstream oss_l;
	options.print(oss_l);
	LOG(INFO) << oss_l.str() << std::endl;

	LOG(INFO) << "Launching Benders Sequential" << std::endl;
	sequential_launch(options);

	char buff[FILENAME_MAX];
	GetCurrentDir(buff, FILENAME_MAX);

	std::stringstream str;
	str << "Optimization results available in : " << buff <<  PATH_SEPARATOR 
		<< options.OUTPUTROOT << PATH_SEPARATOR << options.JSON_NAME + ".json";
	LOG_INFO_AND_COUT(str.str());

	return 0;
}
