// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "Worker.h"
#include "Timer.h"
#include "Benders.h"
#include "SlaveCut.h"

int main(int argc, char** argv)
{
	std::string const root(argv[1]);
	std::string const summary_name(root + PATH_SEPARATOR + argv[2] + PATH_SEPARATOR + "problem_list.txt");
	std::ofstream output(root + PATH_SEPARATOR + argv[2] + PATH_SEPARATOR + "structure.txt");
	if (output.good()) {
		std::ifstream file(summary_name);
		if (file.good()) {
			std::string line;
			std::string name;
			while (std::getline(file, line))
			{
				std::stringstream buffer(line);
				buffer >> name;
				std::string coupling(root + PATH_SEPARATOR + "structure.txt");
				std::ifstream couplingfile(coupling);
				if (couplingfile.good()) {
					std::string coupling_line;
					while (std::getline(couplingfile, coupling_line)) {
						std::stringstream buffer2(coupling_line);
						std::string name_problem_id;
						buffer2 >> name_problem_id;
						if (name == name_problem_id) {
							output << coupling_line << std::endl;
						}
					}
				}
				else { std::cout << "Can't open coupling file" << std::endl; }

			}
		}
		else { std::cout << "Can't open " << summary_name << std::endl; }
	}
	else { std::cout << "Can't open output file" << std::endl; }
}