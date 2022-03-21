#ifndef __MASTER_GENERATION__
#define __MASTER_GENERATION__

#include "ActiveLinks.h"
#include "AdditionalConstraints.h"
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
      AdditionalConstraints additionalConstraints_p,
      std::map<std::pair<std::string, std::string>, int> &couplings,
      std::string const &master_formulation, std::string const &solver_name);
};
#endif  //__MASTER_GENERATION__