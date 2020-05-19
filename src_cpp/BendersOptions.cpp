#include "BendersOptions.h"

/*!
*  \brief Constructor of Benders Options
*
*/
BendersOptions::BendersOptions() {

#define BENDERS_OPTIONS_MACRO(name__, type__, default__) name__ = default__;
#include "BendersOptions.hxx"
#undef BENDERS_OPTIONS_MACRO

}

/*!
*  \brief Write default options in "options_default" txt file
*/
void BendersOptions::write_default(){
std::ofstream file("options_default.txt");
print(file);
file.close();
}

/*!
*  \brief Get path to master problem mps file from options
*/
std::string BendersOptions::get_master_path() const {
	return INPUTROOT + PATH_SEPARATOR + MASTER_NAME;
}

/*!
*  \brief Get path to structure txt file from options
*/
std::string BendersOptions::get_structure_path() const {
	return INPUTROOT + PATH_SEPARATOR + STRUCTURE_FILE;
}

/*!
*  \brief Get path to slave problem mps file from options
*/
std::string BendersOptions::get_slave_path(std::string const & slave_name) const {
	return INPUTROOT + PATH_SEPARATOR + slave_name;
}

/*!
*  \brief Read Benders options from file path
*  
*  \param file_name : path to options txt file
*/
void BendersOptions::read(std::string const & file_name) {
	std::ifstream file(file_name.c_str());
	if (file.good()) {
		std::string line;
		std::string name;
		while (std::getline(file, line))
		{
			std::stringstream buffer(line);
			buffer >> name;
#define BENDERS_OPTIONS_MACRO(name__, type__, default__) if(#name__ == name) buffer >> name__;
#include "BendersOptions.hxx"
#undef BENDERS_OPTIONS_MACRO
		}

		if (SLAVE_WEIGHT != "UNIFORM" && SLAVE_WEIGHT != "CONSTANT") {
			std::string line;
			std::string filename = INPUTROOT + PATH_SEPARATOR + SLAVE_WEIGHT;
			std::ifstream file(filename);
			if (!file) {
				std::cout << "Cannot open file " << filename << std::endl;
			}
			while (std::getline(file, line))
			{
				std::stringstream buffer(line);
				std::string problem_name;
				buffer >> problem_name;
				buffer >> _weights[problem_name];
			}
		}
	}
	else {
		std::cout << "setting option to default" << std::endl;
		write_default();
	}
}

/*!
*  \brief Print all Benders options
*
*  \param stream : output stream
*/
void BendersOptions::print(std::ostream & stream)const {
#define BENDERS_OPTIONS_MACRO(name__, type__, default__) stream<<std::setw(30)<<#name__<<std::setw(50)<<name__<<std::endl;
#include "BendersOptions.hxx"
#undef BENDERS_OPTIONS_MACRO
	stream << std::endl;

	if (SLAVE_NUMBER == 1) {
		stream << "Sequential launch (only one slave problem)" << std::endl;
	}

}

/*!
*  \brief Return slave weight value
*
*  \param nslaves : total number of slaves
*
*  \param name : slave name
*/
double BendersOptions::slave_weight(int nslaves, std::string const & name)const
{
	if (SLAVE_WEIGHT == "UNIFORM") {
		return 1 / static_cast<double>(nslaves);
	}
	else if (SLAVE_WEIGHT == "CONSTANT") {
		double const weight(SLAVE_WEIGHT_VALUE);
		return 1 / weight;
	}
	else {
		return _weights.find(name)->second;
	}
}