#pragma once

#include "ActiveLinks.h"
#include "Candidate.h"
#include "ProblemModifier.h"
#include "common_lpnamer.h"
#include <memory>
#include <multisolver_interface/SolverAbstract.h>

const std::string CANDIDATES_INI{"candidates.ini"};
const std::string STRUCTURE_FILE{"structure.txt"};
const std::string MPS_TXT{"mps.txt"};
const std::string STUDY_FILE{"study.antares"};

struct ProblemData {
  ProblemData(const std::string &problem_mps, const std::string &variables_txt);
  std::string _problem_mps;
  std::string _variables_txt;
};

class LinkProblemsGenerator {

public:
  LinkProblemsGenerator(const std::vector<ActiveLink> &links,
                        const std::string &solver_name)
      : _links(links), _solver_name(solver_name) {}

  void treatloop(
      std::string const &root,
      std::map<std::pair<std::string, std::string>, int> &couplings) const;

private:
  std::vector<ProblemData> readMPSList(std::string const &mps_filePath_p) const;

  void
  treat(std::string const &root, ProblemData const &,
        std::map<std::pair<std::string, std::string>, int> &couplings) const;

  const std::vector<ActiveLink> &_links;
  std::string _solver_name;
};
