
#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/program_options.hpp>

#include "StudyUpdater.h"
#include "ActiveLinks.h"
#include "CandidatesINIReader.h"
#include "LinkProfileReader.h"
#include "helpers/Path.h"
#include "LauncherHelpers.h"

namespace po = boost::program_options;

/*!
 * \brief update links in the antares study directory
 *
 * \param rootPath_p path corresponding to the path to the simulation output directory containing the lp directory
 * \param links_p Structure which contains the list of links
 * \param solutionFilename_p name of the json output file to retrieve in rootPath_p/lp to be used to update the study
 * \return void
 */
void updateStudy(std::string const& rootPath_p, const std::vector<ActiveLink>& links_p, std::string const& solutionFilename_p)
{
	auto linksPath_l = static_cast<std::string>( Path(rootPath_p) / ".." / ".." );
	auto jsonPath_l	= static_cast<std::string>( Path(rootPath_p) / "lp" / solutionFilename_p );

	StudyUpdater studyUpdater(linksPath_l);
	int updateFailures_l = studyUpdater.update(links_p, jsonPath_l);

	if (updateFailures_l)
	{
		std::cout << "Error : Failed to update " << updateFailures_l << " files."
			<< links_p.size() - updateFailures_l << " files were updated\n";
	}
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
		std::string solutionFile_l;

		po::options_description desc("Allowed options");

		desc.add_options()
			("help,h", "produce help message")
			("study-output,o", po::value<std::string>(&root)->required(), "antares-xpansion study output")
			("solution,s", po::value<std::string>(&solutionFile_l)->required(), "path to json solution file")
			;

		po::variables_map opts;
		store(parse_command_line(argc, argv, desc), opts);

		if (opts.count("help")) {
			std::cout << desc << std::endl;
			return 0;
		}

		po::notify(opts);

		ActiveLinksBuilder linksBuilder = get_link_builders(root);

		const std::vector<ActiveLink> links = linksBuilder.getLinks();

		updateStudy(root, links, solutionFile_l);

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

