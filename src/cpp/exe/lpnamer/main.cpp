/**
 * \file lpnamer/main.cpp
 * \brief POC Antares Xpansion V2
 * \author {Manuel Ruiz; Luc Di Gallo}
 * \version 0.1
 * \date 07 aout 2019
 *
 * POC Antares Xpansion V2: create inputs for the solver version 2
 *
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/program_options.hpp>

#include "ActiveLinks.h"
#include "Candidates.h"
#include "AdditionalConstraints.h"
#include "LauncherHelpers.h"
#include "CandidatesInitializer.h"

#include "solver_utils.h"

namespace po = boost::program_options;

 /**
  * \fn string get_name(string const & path)
  * \brief Get the correct path from a string
  *
  * \param path String corresponding to a path with mistakes
  * \return The correct path
  */
std::string get_name(std::string const & path) {
	size_t last_sep(path.find(PATH_SEPARATOR));

	if( last_sep == std::string::npos)
	{
		return path;
	}

	while(true)
	{
		size_t next_sep = path.find(PATH_SEPARATOR, last_sep+1);
		if(next_sep == std::string::npos)
		{
			break;
		}
		else
		{
			last_sep = next_sep;
		}
	}

	std::string name(path.substr(last_sep + 1));
	name = name.substr(0, name.size() - 4);
	return name;
}

/**
 * \fn void masterGeneration()
 * \brief Generate the master ob the optimization problem
 *
 * \param rootPath String corresponding to the path where are located input data
 * \param candidates Structure which contains the list of candidates
 * \param couplings map pairs and integer which give the correspondence between optim variable and antares variable
 * \return void
 */
void masterGeneration(std::string rootPath,
					Candidates candidates,
					AdditionalConstraints additionalConstraints_p,
					std::map< std::pair<std::string, std::string>, int> couplings,
					std::string const &master_formulation,
					std::string const& solver_name)
{
	SolverFactory factory;
	SolverAbstract::Ptr master_l = factory.create_solver(solver_name);

	int ninterco = candidates.size();
	std::vector<int> mstart(ninterco, 0);
	std::vector<double> obj_interco(ninterco, 0);
	std::vector<double> lb_interco(ninterco, -1e20);
	std::vector<double> ub_interco(ninterco, 1e20);
	std::vector<char> coltypes_interco(ninterco, 'C');
	std::vector<std::string> interco_names(ninterco);

	int i(0);
	std::vector<std::string> pallier_names;
	std::vector<int> pallier; //capacity variables indices
	std::vector<int> pallier_i; //number of units variables indices
	std::vector<double> unit_size;
	std::vector<double> max_unit;

	for (auto const & candidate_i : candidates) {
		obj_interco[i] = candidate_i.obj();
		lb_interco[i] = candidate_i.lb();
		ub_interco[i] = candidate_i.ub();
		interco_names[i] = candidate_i._data.name;

		if (candidate_i.is_integer()) {
			pallier.push_back(i);
			int new_id = ninterco + pallier_i.size();
			pallier_i.push_back(new_id);
			unit_size.push_back(candidate_i.unit_size());
			max_unit.push_back(candidate_i.max_unit());
		}
		++i;
	}

    solver_addcols(master_l, obj_interco, mstart, {}, {}, lb_interco, ub_interco, coltypes_interco, interco_names);

	// integer constraints
	int n_integer = pallier.size();
	if(n_integer>0 && master_formulation=="integer"){
		std::vector<double> zeros(n_integer, 0.0);
		std::vector<int> mstart(n_integer, 0);
		std::vector<char> integer_type(n_integer, 'I');
		// Empty colNames
		std::vector<std::string> colNames(0);
        solver_addcols(master_l, zeros, mstart, {}, {}, zeros, max_unit, integer_type, colNames);

		std::vector<double> dmatval;
		std::vector<int> colind;
		std::vector<char> rowtype;
		std::vector<double> rhs;
		std::vector<int> rstart;

		dmatval.reserve(2*n_integer);
		colind.reserve(2 * n_integer);
		rowtype.reserve(n_integer);
		rhs.reserve(n_integer);
		rstart.reserve(n_integer + 1);

		for (i = 0; i < n_integer; ++i) {
			// pMax  - n unit_size = 0
			rstart.push_back(dmatval.size());
			rhs.push_back(0);
			rowtype.push_back('E');

			colind.push_back(pallier[i]);
			dmatval.push_back(1);

			colind.push_back(pallier_i[i]);
			dmatval.push_back(-unit_size[i]);
		}
		rstart.push_back(dmatval.size());

        solver_addrows(master_l, rowtype, rhs, {}, rstart, colind, dmatval);
	}

	treatAdditionalConstraints(master_l, additionalConstraints_p);

	std::string const lp_name = "master";
	master_l->write_prob_mps((rootPath + PATH_SEPARATOR + "lp" + PATH_SEPARATOR + lp_name + ".mps"));

	std::map<std::string, std::map<std::string, int> > output;
	for (auto const & coupling : couplings) {
		output[get_name(coupling.first.second)][coupling.first.first] = coupling.second;
	}
	i = 0;
	for (auto const & name : interco_names) {
		output["master"][name] = i;
		++i;
	}

	std::ofstream coupling_file((rootPath + PATH_SEPARATOR + "lp" + PATH_SEPARATOR + STRUCTURE_FILE).c_str());
	for (auto const & mps : output) {
		for (auto const & pmax : mps.second) {
			coupling_file << std::setw(50) << mps.first;
			coupling_file << std::setw(50) << pmax.first;
			coupling_file << std::setw(10) << pmax.second;
			coupling_file << std::endl;
		}
	}
	coupling_file.close();
}

/**
 * \fn int main (void)
 * \brief Main program
 *
 * \param  argc An integer argument count of the command line arguments
 * \param  argv Path to input data which is the 1st argument vector of the command line argument.
 * \return an integer 0 corresponding to exit success
 */
int main(int argc, char** argv) {

	try {
		
		std::string root;
		std::string master_formulation;
		std::string additionalConstraintFilename_l;

		po::options_description desc("Allowed options");

		desc.add_options()
			("help,h", "produce help message")
			("output,o", po::value<std::string>(&root)->required(), "antares-xpansion study output")
			("formulation,f", po::value<std::string>(&master_formulation)->default_value("relaxed"), "master formulation (relaxed or integer)")
			("exclusion-files,e", po::value<std::string>(&additionalConstraintFilename_l), "path to exclusion files")
			;

		po::variables_map opts;
		po::store(po::parse_command_line(argc, argv, desc), opts);

		if (opts.count("help")) {
			std::cout << desc << std::endl;
			return 0;
		}

		po::notify(opts);

        std::string const area_file_name	= root + PATH_SEPARATOR + "area.txt";
        std::string const interco_file_name	= root + PATH_SEPARATOR + "interco.txt";

        CandidatesINIReader candidateReader(interco_file_name,area_file_name);
        LinkProfileReader profileReader;

        CandidatesInitializer initializer = CandidatesInitializer(profileReader,candidateReader);

        // Get all mandatory path
        std::string const xpansion_user_dir = root + PATH_SEPARATOR + ".." + PATH_SEPARATOR + ".." + PATH_SEPARATOR + "user" + PATH_SEPARATOR + "expansion";
        std::string const candidates_file_name = xpansion_user_dir + PATH_SEPARATOR + CANDIDATES_INI;
        std::string const capacity_folder = xpansion_user_dir + PATH_SEPARATOR + "capa";

        // Instantiation of candidates
		Candidates candidates = initializer.initializedCandidates(candidates_file_name, capacity_folder);

		const auto& candidatesDatas = candidateReader.readCandidateData(candidates_file_name);
		const auto& mapLinkProfile	= LinkProfileReader::getLinkProfileMap(capacity_folder, candidatesDatas);

		ActiveLinksBuilder links(candidatesDatas, mapLinkProfile);
		
		if ((master_formulation != "relaxed") && (master_formulation != "integer"))
		{
			std::cout << "Invalid formulation argument : argument must be \"integer\" or \"relaxed\"" 
				<< std::endl;
			std::exit(1);
		}


		AdditionalConstraints additionalConstraints;
		if (!additionalConstraintFilename_l.empty())
		{
			additionalConstraints = AdditionalConstraints(additionalConstraintFilename_l);
		}

		std::map< std::pair<std::string, std::string>, int> couplings;
		std::string solver_name = "CBC";
		candidates.treatloop(root, couplings, solver_name);
		masterGeneration(root, candidates, additionalConstraints, couplings, 
			master_formulation, solver_name);

		return 0;		
	}
	catch (std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "Exception of unknown type!" << std::endl;
	}

	return 0;
}

