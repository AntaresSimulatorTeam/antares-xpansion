// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "launcher.h"
#include "BendersOptions.h"


int main(int argc, char** argv)
{
	//options.print(std::cout);
	usage(argc);
	BendersOptions options(build_benders_options(argc, argv));
	options.print(std::cout);
	sequential_launch(options);

	return 0;
}