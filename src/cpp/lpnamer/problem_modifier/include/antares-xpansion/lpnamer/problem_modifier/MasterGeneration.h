#ifndef __MASTER_GENERATION__
#define __MASTER_GENERATION__

#include <filesystem>

#include "antares-xpansion/lpnamer/model/ActiveLinks.h"
#include "antares-xpansion/lpnamer/problem_modifier/AdditionalConstraints.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"

class FileWriter;

class MasterGeneration
{
public:
    /**
     * \fn  MasterGeneration constructor
     *
     *  Functionnal style usage :
     * MasterGeneration(SystemArgs...)(OptimizationArgs...)
     *
     * \param ouput_path Path to simulation output
     * \param solver_name Name of the solver to use
     * \param logger Logger to use for logging messages
     * \param file_writer File writer to use for writing problems
     * \param format Format of the problems to be generated (MPS or SAVED_FILE)
     * between optim variable and antares variable
     */
    explicit MasterGeneration(std::filesystem::path ouput_path,
                              std::string solver_name,
                              std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger,
                              SolverLogManager& solver_log_manager,
                              FileWriter& file_writer,
                              ProblemsFormat format = ProblemsFormat::OPTIMIZED);
    std::vector<Candidate> generate(const std::vector<ActiveLink>& links,
                                    const std::string& master_formulation,
                                    const AdditionalConstraints& additionalConstraints_p) const;

private: /*methods*/
    std::vector<Candidate> build_candidates(const std::vector<ActiveLink>& links) const;
    void write_master_mps(const std::vector<Candidate>& candidates,
                          const std::string& master_formulation,
                          const std::string& solver_name,
                          const AdditionalConstraints& additionalConstraints_p) const;

    /*members*/
    std::filesystem::path output_path_;
    std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger_;
    SolverLogManager& logManager_;
    const std::string solver_name_;
    FileWriter& writer_;
    [[maybe_unused]] ProblemsFormat format_{ProblemsFormat::OPTIMIZED};
};
#endif //__MASTER_GENERATION__
