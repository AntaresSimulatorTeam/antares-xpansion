#include "MasterGeneration.h"

#include <algorithm>

#include "LauncherHelpers.h"
#include "MasterProblemBuilder.h"
#include "helpers/Path.h"
#include "multisolver_interface/SolverAbstract.h"
/**
 * \fn string get_name(string const & path)
 * \brief Get the correct path from a string
 *
 * \param path String corresponding to a path with mistakes
 * \return The correct path
 */
std::string get_name(std::string const &path) {
  size_t last_sep(path.find(Path::mSep));

  if (last_sep == std::string::npos) {
    return path;
  }

  while (true) {
    size_t next_sep = path.find(Path::mSep, last_sep + 1);
    if (next_sep == std::string::npos) {
      break;
    } else {
      last_sep = next_sep;
    }
  }

  std::string name(path.substr(last_sep + 1));
  name = name.substr(0, name.size() - 4);
  return name;
}
MasterGeneration::MasterGeneration(
    const std::string &rootPath, const std::vector<ActiveLink> &links,
    const AdditionalConstraints &additionalConstraints_p, Couplings &couplings,
    std::string const &master_formulation, std::string const &solver_name) {
  add_candidates(links);
  write_master_mps(rootPath, master_formulation, solver_name,
                   additionalConstraints_p);
  write_structure_file(rootPath, couplings);
}

void MasterGeneration::add_candidates(const std::vector<ActiveLink> &links) {
  for (const auto &link : links) {
    const auto &candidateFromLink = link.getCandidates();
    candidates.insert(candidates.end(), candidateFromLink.begin(),
                      candidateFromLink.end());
  }
  std::sort(candidates.begin(), candidates.end(),
            [](const Candidate &cand1, const Candidate &cand2) {
              return cand1.get_name() < cand2.get_name();
            });
}

void MasterGeneration::write_master_mps(
    const std::string &rootPath, std::string const &master_formulation,
    std::string const &solver_name,
    const AdditionalConstraints &additionalConstraints_p) const {
  SolverAbstract::Ptr master_l =
      MasterProblemBuilder(master_formulation).build(solver_name, candidates);
  treatAdditionalConstraints(master_l, additionalConstraints_p);

  std::string const &lp_name = "master";
  master_l->write_prob_mps(
      (Path(rootPath) / "lp" / (lp_name + ".mps")).get_str());
}

void MasterGeneration::write_structure_file(const std::string &rootPath,
                                            const Couplings &couplings) const {
  std::map<std::string, std::map<std::string, unsigned int>> output;
  for (const auto &[candidate_name_and_mps_filePath, colId] : couplings) {
    output[get_name(candidate_name_and_mps_filePath.second)]
          [candidate_name_and_mps_filePath.first] = colId;
  }
  unsigned int i = 0;
  for (auto const &candidate : candidates) {
    output["master"][candidate.get_name()] = i;
    ++i;
  }

  std::ofstream coupling_file(
      (Path(rootPath) / "lp" / STRUCTURE_FILE).get_str().c_str());
  for (auto const &mps : output) {
    for (auto const &pmax : mps.second) {
      coupling_file << std::setw(50) << mps.first;
      coupling_file << std::setw(50) << pmax.first;
      coupling_file << std::setw(10) << pmax.second;
      coupling_file << std::endl;
    }
  }
  coupling_file.close();
}