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
	__nameToMember.insert(std::pair<std::string, int *>("LOG_LEVEL", &LOG_LEVEL));
	__nameToMember.insert(std::pair<std::string, int *>("MAX_ITERATIONS", &MAX_ITERATIONS));
	__nameToMember.insert(std::pair<std::string, int *>("SLAVE_NUMBER", &SLAVE_NUMBER));
	__nameToMember.insert(std::pair<std::string, double *>("ABSOLUTE_GAP", &ABSOLUTE_GAP));
	__nameToMember.insert(std::pair<std::string, double *>("RELATIVE_GAP", &RELATIVE_GAP));
	__nameToMember.insert(std::pair<std::string, double *>("SLAVE_WEIGHT_VALUE", &SLAVE_WEIGHT_VALUE));
	__nameToMember.insert(std::pair<std::string, double *>("TIME_LIMIT", &TIME_LIMIT));
	__nameToMember.insert(std::pair<std::string, bool *>("AGGREGATION", &AGGREGATION));
	__nameToMember.insert(std::pair<std::string, bool *>("TRACE", &TRACE));
	__nameToMember.insert(std::pair<std::string, bool *>("BOUND_ALPHA", &BOUND_ALPHA));
	__nameToMember.insert(std::pair<std::string, std::string *>("OUTPUTROOT", &OUTPUTROOT));
	__nameToMember.insert(std::pair<std::string, std::string *>("SLAVE_WEIGHT", &SLAVE_WEIGHT));
	__nameToMember.insert(std::pair<std::string, std::string *>("MASTER_NAME", &MASTER_NAME));
	__nameToMember.insert(std::pair<std::string, std::string *>("STRUCTURE_FILE", &STRUCTURE_FILE));
	__nameToMember.insert(std::pair<std::string, std::string *>("INPUTROOT", &INPUTROOT));
	__nameToMember.insert(std::pair<std::string, std::string *>("CSV_NAME", &CSV_NAME));
	__nameToMember.insert(std::pair<std::string, std::string *>("SOLVER_NAME", &SOLVER_NAME));
	__nameToMember.insert(std::pair<std::string, std::string *>("JSON_FILE", &JSON_FILE));
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
			// #define BENDERS_OPTIONS_MACRO(name__, type__, default__) \
// 	if (#name__ == name)                                 \
// 		buffer >> name__;
			// #include "BendersOptions.hxx"
			// #undef BENDERS_OPTIONS_MACRO
			// 	if (#name__ == name)                                 \
// 		buffer >> name__;
			auto it = __nameToMember.find(name);
			if (it != __nameToMember.end())
			{
				int *integer = nullptr;
				double *db = nullptr;
				std::string *str = nullptr;
				bool *boolean = nullptr;
				std::cout << "******* it->first = " << it->first << " ********\n";
				if (is_Tptr(it->second, integer))
					buffer >> *integer;
				else if (is_Tptr(it->second, boolean))
					buffer >> *boolean;
				else if (is_Tptr(it->second, str))
					buffer >> *str;
				else if (is_Tptr(it->second, db))
					buffer >> *db;
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
		int *integer = nullptr;
		double *db = nullptr;
		std::string *str = nullptr;
		bool *boolean = nullptr;
		std::cout << "******* it.first = " << it.first << " ********\n";
		if (is_Tptr(it.second, integer))
			stream << std::setw(30) << it.first << std::setw(50) << *integer << std::endl;
		if (is_Tptr(it.second, db))
			stream << std::setw(30) << it.first << std::setw(50) << *db << std::endl;
		if (is_Tptr(it.second, boolean))
			stream << std::setw(30) << it.first << std::setw(50) << *boolean << std::endl;
		if (is_Tptr(it.second, str))
			stream << std::setw(30) << it.first << std::setw(50) << *str << std::endl;
	}
	// #define BENDERS_OPTIONS_MACRO(name__, type__, default__) stream << std::setw(30) << #name__ << std::setw(50) << name__ << std::endl;
	// #include "BendersOptions.hxx"
	// #undef BENDERS_OPTIONS_MACRO
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
