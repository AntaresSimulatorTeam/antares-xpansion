#ifndef __MASTER_GENERATION__
#define __MASTER_GENERATION__

#include <filesystem>

#include "antares-xpansion/lpnamer/model/ActiveLinks.h"
#include "antares-xpansion/lpnamer/problem_modifier/AdditionalConstraints.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"

class MasterGeneration
{
public:
    /**
     * \fn  MasterGeneration constructor
     * \brief Generate the master ob the optimization problem
     *
     * \param rootPath String corresponding to the path where are located input
     * data
     * \param links Structure which contains the list of Activelink
     * \param couplings map pairs and integer which give the correspondence
     * between optim variable and antares variable
     */
    explicit MasterGeneration(const std::filesystem::path& rootPath,
                              const std::vector<ActiveLink>& links,
                              const AdditionalConstraints& additionalConstraints_p,
                              Couplings& couplings,
                              const std::string& master_formulation,
                              const std::string& solver_name,
                              std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger,
                              SolverLogManager& solver_log_manager,
                              bool asmps = false);

private: /*methods*/
    void add_candidates(const std::vector<ActiveLink>& links);
    void write_master_mps(const std::filesystem::path& rootPath,
                          const std::string& master_formulation,
                          const std::string& solver_name,
                          const AdditionalConstraints& additionalConstraints_p,
                          SolverLogManager& solver_log_manager) const;
    void write_structure_file(const std::filesystem::path& rootPath,
                              const Couplings& couplings) const;

private: /*members*/
    std::vector<Candidate> candidates;
    std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger_;
    const std::string solver_name_;
    bool asmps{false};
};
#endif //__MASTER_GENERATION__
