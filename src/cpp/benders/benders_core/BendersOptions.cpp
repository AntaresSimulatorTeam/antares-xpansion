#include "BendersOptions.h"
#include "helpers/Path.h"

/*!
 *  \brief Constructor of Benders Options
 *
 */
BendersOptions::BendersOptions() : _weights()
{
	_initNameToMember();
}
void BendersOptions::_initNameToMember()
{
	insert("LOG_LEVEL", types::integer,  &LOG_LEVEL);
	insert("MAX_ITERATIONS", types::integer, &MAX_ITERATIONS);
	insert("SLAVE_NUMBER", types::integer, &SLAVE_NUMBER);
	insert("ABSOLUTE_GAP", types::doublep, &ABSOLUTE_GAP);
	insert("RELATIVE_GAP", types::doublep, &RELATIVE_GAP);
	insert("SLAVE_WEIGHT_VALUE", types::doublep, &SLAVE_WEIGHT_VALUE);
	insert("TIME_LIMIT", types::doublep, &TIME_LIMIT);
	insert("AGGREGATION", types::boolean, &AGGREGATION);
	insert("TRACE", types::boolean, &TRACE);
	insert("BOUND_ALPHA", types::boolean, &BOUND_ALPHA);
	insert("OUTPUTROOT", types::std_string, &OUTPUTROOT);
	insert("SLAVE_WEIGHT", types::std_string, &SLAVE_WEIGHT);
	insert("MASTER_NAME", types::std_string, &MASTER_NAME);
	insert("STRUCTURE_FILE", types::std_string, &STRUCTURE_FILE);
	insert("INPUTROOT", types::std_string, &INPUTROOT);
	insert("CSV_NAME", types::std_string, &CSV_NAME);
	insert("SOLVER_NAME", types::std_string, &SOLVER_NAME);
	insert("JSON_FILE", types::std_string, &JSON_FILE);
}

/*!
 *  \brief Write default options in "options_default" txt file
 */
void BendersOptions::write_default()
{
	std::ofstream file("options_default.txt");
	print(file);
	file.close();
}

/*!
 *  \brief Get path to master problem mps file from options
 */
std::string BendersOptions::get_master_path() const
{
	return (Path(INPUTROOT) / (MASTER_NAME + ".mps")).get_str();
}

/*!
 *  \brief Get path to structure txt file from options
 */
std::string BendersOptions::get_structure_path() const
{
	return (Path(INPUTROOT) / STRUCTURE_FILE).get_str();
}

/*!
 *  \brief Get path to slave problem mps file from options
 */
std::string BendersOptions::get_slave_path(std::string const &slave_name) const
{
	return (Path(INPUTROOT) / (slave_name + ".mps")).get_str();
}

/*!
 *  \brief Read Benders options from file path
 *
 *  \param file_name : path to options txt file
 */
void BendersOptions::read(std::string const &file_name)
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
			auto it = __nameToMember.find(name);
			if (it != __nameToMember.end())
			{
				switch (std::get<1>(it->second))
				{
				case types::integer:
					buffer >> *((int*)std::get<0>(it->second));
					break;
				case types::doublep:
					buffer >> *((double*)std::get<0>(it->second));
					break;
				case types::std_string:
					buffer >> *((std::string*)std::get<0>(it->second));
					break;
				case types::boolean:
					buffer >> *((bool*)std::get<0>(it->second));
					break;
				
				default:
					break;
				}

			}
		}
		if (SLAVE_WEIGHT != "UNIFORM" && SLAVE_WEIGHT != "CONSTANT")
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
				if (problem_name == "WEIGHT_SUM")
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
	else
	{
		std::cout << "setting option to default" << std::endl;
		write_default();
	}
}

/*!
 *  \brief Print all Benders options
 *
 *  \param stream : output stream
 */
void BendersOptions::print(std::ostream &stream) const
{
	for (const auto &it : __nameToMember)
	{
		switch (std::get<1>(it.second))
				{
				case types::integer:
					stream << std::setw(30) << it.first << std::setw(50) << *((int*)std::get<0>(it.second)) << std::endl;
					break;
				case types::doublep:
					stream << std::setw(30) << it.first << std::setw(50) << *((double*)std::get<0>(it.second)) << std::endl;
					break;
				case types::std_string:
					stream << std::setw(30) << it.first << std::setw(50) << *((std::string*)std::get<0>(it.second)) << std::endl;
					break;
				case types::boolean:
					stream << std::setw(30) << it.first << std::setw(50) << *((bool*)std::get<0>(it.second)) << std::endl;
					break;
				
				default:
					break;
				}
	}
	stream << std::endl;

	if (SLAVE_NUMBER == 1)
	{
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
double BendersOptions::slave_weight(int nslaves, std::string const &name) const
{
	if (SLAVE_WEIGHT == "UNIFORM")
	{
		return 1 / static_cast<double>(nslaves);
	}
	else if (SLAVE_WEIGHT == "CONSTANT")
	{
		double const weight(SLAVE_WEIGHT_VALUE);
		return 1 / weight;
	}
	else
	{
		return _weights.find(name)->second;
	}
}
