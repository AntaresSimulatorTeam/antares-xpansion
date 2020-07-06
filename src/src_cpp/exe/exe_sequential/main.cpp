// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "launcher.h"
#include "BendersOptions.h"
#include <glog/logging.h>


int main(int argc, char** argv)
{
	google::InitGoogleLogging(argv[0]);
	google::SetLogDestination(google::GLOG_INFO, "./benderssequentialLog");
	LOG(INFO) << "starting Benders Sequential" << std::endl;

	//options.print(std::cout);
	usage(argc);
	BendersOptions options(build_benders_options(argc, argv));
	std::ostringstream oss_l;
	options.print(oss_l);
	std::cout << oss_l.str();
	LOG(INFO) << oss_l.str() << std::endl;

	LOG(INFO) << "Launching Benders Sequential" << std::endl;
	sequential_launch(options);

	return 0;
}
