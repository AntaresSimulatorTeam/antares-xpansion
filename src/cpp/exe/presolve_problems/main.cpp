// projet_benders.cpp : définit le point d'entrée pour l'application console.
//

#include <common.h>

#include <filesystem>
#include <typeinfo>
#include <unordered_set>

#include "multisolver_interface/../../SolverXpress.h"
#include "multisolver_interface/environment.h"

void XPRS_CC Message(XPRSprob my_prob, void *object, const char *msg, int len,
                     int msgtype) {
  switch (msgtype) {
    case 4: /* error */
    case 3: /* warning */
    case 2: /* not used */
    case 1: /* information */
      printf("%s\n", msg);
      break;
    default: /* exiting - buffers need flushing */
      fflush(stdout);
      break;
  }
}

int main(int argc, char **argv) {
  // Initialize Xpress;
  LoadXpress::XpressLoader xpressLoader;
  xpressLoader.initXpressEnv();
  XPRSprob xprsProb;
  LoadXpress::XPRSinit(NULL);
  LoadXpress::XPRScreateprob(&xprsProb);
  LoadXpress::XPRSaddcbmessage(xprsProb, Message, NULL, 0);
  LoadXpress::XPRSsetintcontrol(xprsProb, XPRS_OUTPUTLOG,
                                XPRS_OUTPUTLOG_FULL_OUTPUT);

  // Read full problem MPS
  std::filesystem::path lpDir(argv[1]);

  // Parse structure and get candidates id
  std::filesystem::path structureFilePath = lpDir / "structure.txt";
  CouplingMap couplings = build_input(structureFilePath);
  std::map<std::string, std::vector<int>> pbNameTocandidatesId;
  for (const auto &[pbName, varNameAndCandidateId] : couplings) {
    for (const auto &[varName, id] : varNameAndCandidateId) {
      pbNameTocandidatesId[pbName].emplace_back(id);
    }
  }
  // Rename structure, so that structure with presolved pbs can be named
  // structure.txt and benders can run without modification
  std::filesystem::rename(structureFilePath,
                          structureFilePath.parent_path() /
                              (structureFilePath.stem().string() + "-full" +
                               structureFilePath.extension().string()));

  CouplingMap presolvedCouplings;
  std::string presolvedPrefix = "presolved-";
  // Configure presolve and solve with 0 iteration
  for (const auto &[pbName, candidates] : pbNameTocandidatesId) {
    if (pbName == "master") {
      // Copy couplings for master
      for (auto &[varName, id] : couplings[pbName]) {
        presolvedCouplings[pbName][varName] = id;
      }
    } else {
      // Do the actual presolve work for subproblems
      std::filesystem::path fullMpsPath = lpDir / pbName;
      LoadXpress::XPRSreadprob(xprsProb, fullMpsPath.c_str(), "");
      LoadXpress::XPRSloadsecurevecs(xprsProb, 0, candidates.size(), nullptr,
                                     candidates.data());
      LoadXpress::XPRSsetintcontrol(xprsProb, XPRS_LPITERLIMIT, 0);
      LoadXpress::XPRSlpoptimize(xprsProb, "");

      // Write presolved problem MPS
      std::filesystem::path presolvedFilename =
          presolvedPrefix + fullMpsPath.filename().string();
      std::filesystem::path presolvedPath =
          fullMpsPath.parent_path() / presolvedFilename;
      LoadXpress::XPRSwriteprob(xprsProb, presolvedPath.c_str(), "");

      // Get candidates id in presolved problem
      int nbCols(0);
      int nbRows(0);
      LoadXpress::XPRSgetintattrib(xprsProb, XPRS_COLS, &nbCols);
      LoadXpress::XPRSgetintattrib(xprsProb, XPRS_ROWS, &nbRows);
      std::vector<int> colMap(nbCols);
      std::vector<int> rowMap(nbRows);
      LoadXpress::XPRSgetpresolvemap(xprsProb, rowMap.data(), colMap.data());

      std::unordered_set<int> initCandidatesIdSet(
          pbNameTocandidatesId[pbName].begin(),
          pbNameTocandidatesId[pbName].end());
      // Use an unordered_map to store the indices of found values
      std::unordered_map<int, int> initIdToPresolvedId;

      for (int i = 0; i < colMap.size(); ++i) {
        if (initCandidatesIdSet.find(colMap[i]) != initCandidatesIdSet.end()) {
          initIdToPresolvedId[colMap[i]] = i;
          initCandidatesIdSet.erase(colMap[i]);
          if (initCandidatesIdSet.empty()) {
            break;
          }
        }
      }
      for (auto &[varName, id] : couplings[pbName]) {
        presolvedCouplings[presolvedFilename.string()][varName] =
            initIdToPresolvedId[id];
      }
    }
  }

  // Write structure for presolved problem
  std::ofstream coupling_file(structureFilePath);

  for (auto const &[pbName, candidatesNameAndColId] : presolvedCouplings) {
    for (auto const &[candidateName, presolvedColId] : candidatesNameAndColId) {
      coupling_file << std::setw(50) << pbName;
      coupling_file << std::setw(50) << candidateName;
      coupling_file << std::setw(10) << presolvedColId;
      coupling_file << std::endl;
    }
  }
  coupling_file.close();

  return 0;
}
