#include "SimulationOptions.h"
#include "helpers/Path.h"

/*!
 *  \brief Constructor of Benders Options
 *
 */
SimulationOptions::SimulationOptions() :
#define BENDERS_OPTIONS_MACRO(name__, type__, default__) name__(default__),
#include "SimulationOptions.hxx"
#undef BENDERS_OPTIONS_MACRO
										 _weights()
{
}

/*!
 *  \brief Write default options in "options_default" txt file
 */
void SimulationOptions::write_default()
{
	std::ofstream file("options_default.txt");
	print(file);
	file.close();
}

/*!
 *  \brief Read Benders options from file path
 *
 *  \param file_name : path to options txt file
 */
void SimulationOptions::read(std::string const &file_name)
{
	std::ifstream file(file_name.c_str());
	if (file.good())
	{
		std::string line;
		std::string name;
		while (std::getline(file, line))
		{
			std::stringstream buffer(line);
			buffer >> name;
#define BENDERS_OPTIONS_MACRO(name__, type__, default__) \
	if (#name__ == name)                                 \
		buffer >> name__;
#include "SimulationOptions.hxx"
#undef BENDERS_OPTIONS_MACRO
		}
		set_weights();
	}
	else
	{
		std::cout << "setting option to default" << std::endl;
		write_default();
	}
}

void SimulationOptions::set_weights()
{
	if (SLAVE_WEIGHT != SLAVE_WEIGHT_UNIFORM_CST_STR && SLAVE_WEIGHT != SLAVE_WEIGHT_CST_STR)
	{
		std::string line;
		std::string filename(Path(INPUTROOT) / SLAVE_WEIGHT);
		std::ifstream file(filename);

		if (!file)
		{
			std::cout << "Cannot open file " << filename << std::endl;
		}
		double weights_sum = -1;
		while (std::getline(file, line))
		{
			std::stringstream buffer(line);
			std::string problem_name;

			buffer >> problem_name;
			if (problem_name == WEIGHT_SUM_CST_STR)
			{
				buffer >> weights_sum;
			}
			else
			{
				buffer >> _weights[problem_name];
			}
		}

		if (weights_sum == -1)
		{
			std::cout << "ERROR : Invalid weight file format : Key WEIGHT_SUM not found." << std::endl;
			std::exit(1);
		}
		else
		{
			for (auto &kvp : _weights)
			{
				_weights[kvp.first] /= weights_sum;
			}
		}
	}
}

/*!
 *  \brief Print all Benders options
 *
 *  \param stream : output stream
 */
void SimulationOptions::print(std::ostream &stream) const
{
#define BENDERS_OPTIONS_MACRO(name__, type__, default__) stream << std::setw(30) << #name__ << std::setw(50) << name__ << std::endl;
#include "SimulationOptions.hxx"
#undef BENDERS_OPTIONS_MACRO
	stream << std::endl;

	if (SLAVE_NUMBER == 1)
	{
		stream << "Sequential launch (only one slave problem)" << std::endl;
	}
}
