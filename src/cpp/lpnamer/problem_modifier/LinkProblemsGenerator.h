#pragma once

#include <multisolver_interface/SolverAbstract.h>

#include <filesystem>
#include <memory>

#include "ActiveLinks.h"
#include "ArchiveReader.h"
#include "ArchiveWriter.h"
#include "Candidate.h"
#include "FileInBuffer.h"
#include "ProblemModifier.h"
#include "common_lpnamer.h"
const std::string CANDIDATES_INI{"candidates.ini"};
const std::string STRUCTURE_FILE{"structure.txt"};
const std::string MPS_TXT{"mps.txt"};
const std::string MPS_ZIP_FILE{"MPS_ZIP_FILE"};
const std::string ZIP_EXT{".zip"};
const std::string STUDY_FILE{"study.antares"};
typedef std::pair<std::string, std::filesystem::path>
    CandidateNameAndMpsFilePath;
typedef unsigned int ColId;
typedef std::map<CandidateNameAndMpsFilePath, ColId> Couplings;

struct ProblemData {
  ProblemData(const std::string& problem_mps, const std::string& variables_txt);
  std::string _problem_mps;
  std::string _variables_txt;
};

class LinkProblemsGenerator {
 public:
  LinkProblemsGenerator(const std::vector<ActiveLink>& links,
                        const std::string& solver_name)
      : _links(links), _solver_name(solver_name) {}

  void treatloop(const std::filesystem::path& root,
                 const std::filesystem::path& archivePath,
                 Couplings& couplings);

 private:
  std::vector<ProblemData> readMPSList(
      const std::filesystem::path& mps_filePath_p) const;

  void treat(const std::filesystem::path& root, ProblemData const&,
             Couplings& couplings, ArchiveReader& reader,
             ArchiveWriter& writer);

  const std::vector<ActiveLink>& _links;
  std::string _solver_name;
  std::filesystem::path lpDir_ = "";
  FileBufferVector mpsBufferVector = {};
};
