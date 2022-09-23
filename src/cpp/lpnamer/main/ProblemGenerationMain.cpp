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

#include "ProblemGenerationMain.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "ActiveLinks.h"
#include "AdditionalConstraints.h"
#include "Candidate.h"
#include "CandidatesINIReader.h"
#include "LauncherHelpers.h"
#include "LinkProblemsGenerator.h"
#include "LinkProfileReader.h"
#include "MasterGeneration.h"
#include "MasterProblemBuilder.h"
#include "solver_utils.h"

namespace po = boost::program_options;
void RunProblemGeneration(const std::filesystem::path& root,
                          const std::string& master_formulation,
                          const std::string& additionalConstraintFilename_l) {
  ActiveLinksBuilder linkBuilder = get_link_builders(root);

  if ((master_formulation != "relaxed") && (master_formulation != "integer")) {
    std::cout << "Invalid formulation argument : argument must be "
                 "\"integer\" or \"relaxed\""
              << std::endl;
    std::exit(1);
  }

  AdditionalConstraints additionalConstraints;
  if (!additionalConstraintFilename_l.empty()) {
    additionalConstraints =
        AdditionalConstraints(additionalConstraintFilename_l);
  }

  Couplings couplings;
  std::string solver_name = "CBC";
  std::vector<ActiveLink> links = linkBuilder.getLinks();
  LinkProblemsGenerator linkProblemsGenerator(links, solver_name);
  linkProblemsGenerator.treatloop(root, couplings);

  MasterGeneration master_generation(root, links, additionalConstraints,
                                     couplings, master_formulation,
                                     solver_name);
}
ProblemGenerationExeOptions::ProblemGenerationExeOptions() {
  desc_.add_options()("help,h", "produce help message")(
      "output,o", po::value<std::filesystem::path>(&root_)->required(),
      "antares-xpansion study output")(
      "formulation,f",
      po::value<std::string>(&master_formulation_)->default_value("relaxed"),
      "master formulation (relaxed or integer)")(
      "exclusion-files,e",
      po::value<std::string>(&additional_constraintFilename_l_),
      "path to exclusion files");
}

void ProblemGenerationExeOptions::parse(int argc, char** argv) {
  po::variables_map opts;
  po::store(po::parse_command_line(argc, argv, desc_), opts);

  if (opts.count("help")) {
    std::cout << desc_ << std::endl;
    return std::exit(0);
  }

  po::notify(opts);
}

/**
 * \fn int main (void)
 * \brief Main program
 *
 * \param  argc An integer argument count of the command line arguments
 * \param  argv Path to input data which is the 1st argument vector of the
 * command line argument. \return an integer 0 corresponding to exit success
 */
int ProblemGenerationMain(int argc, char** argv) {
  try {
    auto options_parser = ProblemGenerationExeOptions();
    options_parser.parse(argc, argv);

    auto root = options_parser.root();
    auto master_formulation = options_parser.master_formulation();
    auto additionalConstraintFilename_l =
        options_parser.additional_constraintFilename_l();
    RunProblemGeneration(root, master_formulation,
                         additionalConstraintFilename_l);

    return 0;
  } catch (std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Exception of unknown type!" << std::endl;
  }

  return 0;
}