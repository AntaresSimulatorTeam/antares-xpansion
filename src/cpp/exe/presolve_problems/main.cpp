// projet_benders.cpp : définit le point d'entrée pour l'application console.
//

#include <common.h>

#include <filesystem>
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
  std::filesystem::path fullMpsPath(argv[1]);
  LoadXpress::XPRSreadprob(xprsProb, fullMpsPath.c_str(), "");

  // Parse structure and get candidates id
  std::filesystem::path structureFilePath = "structure.txt";
  CouplingMap pbAndVarNameToVarId = build_input(structureFilePath);
  std::vector<int> candidatesId;
  for (const auto &[var_name, id] :
       pbAndVarNameToVarId[fullMpsPath.replace_extension()]) {
    candidatesId.emplace_back(id);
  }

  // Configure presolve and solve with 0 iteration
  LoadXpress::XPRSloadsecurevecs(xprsProb, 0, candidatesId.size(), nullptr,
                                 candidatesId.data());
  LoadXpress::XPRSsetintcontrol(xprsProb, XPRS_LPITERLIMIT, 0);
  LoadXpress::XPRSlpoptimize(xprsProb, "");

  // Get candidates id in presolved problem
  int nbCols(0);
  int nbRows(0);
  LoadXpress::XPRSgetintattrib(xprsProb, XPRS_COLS, &nbCols);
  LoadXpress::XPRSgetintattrib(xprsProb, XPRS_ROWS, &nbRows);
  std::vector<int> colMap(nbCols);
  std::vector<int> rowMap(nbRows);
  LoadXpress::XPRSgetpresolvemap(xprsProb, rowMap.data(), colMap.data());

  std::unordered_set<int> initCandidatesIdSet(candidatesId.begin(),
                                              candidatesId.end());

  // Use an unordered_map to store the indices of found values
  std::unordered_map<int, int> initIdToPresolvedId;

  // Iterate through the vector
  for (int i = 0; i < colMap.size(); ++i) {
    if (initCandidatesIdSet.find(colMap[i]) != initCandidatesIdSet.end()) {
      
      initIdToPresolvedId[colMap[i]] = i;
      initCandidatesIdSet.erase(colMap[i]);
      if (initCandidatesIdSet.empty()) {
        break;
      }
    }
  }

  // Write presolved problem MPS
  std::string presolvedPrefix = "presolved-";
  std::filesystem::path presolvedFilename =
      presolvedPrefix + fullMpsPath.filename().string();
  std::filesystem::path presolvedPath =
      fullMpsPath.parent_path() / presolvedFilename;
  LoadXpress::XPRSwriteprob(xprsProb, presolvedPath.c_str(), "");

  return 0;
}
