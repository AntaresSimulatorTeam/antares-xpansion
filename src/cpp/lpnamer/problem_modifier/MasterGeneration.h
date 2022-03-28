#ifndef __MASTER_GENERATION__
#define __MASTER_GENERATION__

#include "ActiveLinks.h"
#include "AdditionalConstraints.h"
#include "LinkProblemsGenerator.h"
class MasterGeneration {
 public:
  /**
   * \fn  MasterGeneration constructor
   * \brief Generate the master ob the optimization problem
   *
   * \param rootPath String corresponding to the path where are located input
   * data \param links Structure which contains the list of Activelink \param
   * couplings map pairs and integer which give the correspondence between optim
   * variable and antares variable
   */
  explicit MasterGeneration(
      const std::string &rootPath, const std::vector<ActiveLink> &links,
      const AdditionalConstraints &additionalConstraints_p,
      Couplings &couplings, std::string const &master_formulation,
      std::string const &solver_name);

 private: /*methods*/
  void add_candidates(const std::vector<ActiveLink> &links);
  void write_master_mps(
      const std::string &rootPath, std::string const &master_formulation,
      std::string const &solver_name,
      const AdditionalConstraints &additionalConstraints_p) const;
  void write_structure_file(const std::string &rootPath,
                            const Couplings &couplings) const;

 private: /*members*/
  std::vector<Candidate> candidates;
};
#endif  //__MASTER_GENERATION__