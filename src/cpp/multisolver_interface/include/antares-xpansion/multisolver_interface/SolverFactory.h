#pragma once

#include <set>

#include "SolverConfig.h"
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

/**
 * \enum mapper::SOLVER_TYPE
 * \brief algo type
 */
enum class SOLVER_TYPE
{
    INTEGER,
    CONTINUOUS
};

/*!
 * \class class SolverLoader
 * \brief Class to check if supported solvers are available
 */
class SolverLoader
{
public:
    static std::vector<std::string> GetAvailableSolvers(std::shared_ptr<ILoggerXpansion> logger);

    /**
     * @brief Returns a list of supported solvers
     * Supported doesn't mean available, for exemple if licence are not available
     */
    static std::vector<std::string> GetSupportedSolvers();
};

/*!
 * \class class SolverFactory
 * \brief Class to manage the creation of solvers from the different
 * implementations
 */
class SolverFactory
{
private:
    std::vector<std::string> _available_solvers;

public:
    /**
     * @brief Constructor of SolverFactory, fills the list of available solvers
     */
    explicit SolverFactory(
      std::shared_ptr<ILoggerXpansion> logger = std::make_shared<EmptyLogger>());

public:
    /**
     * @brief Creates and returns to an object solver from the wanted
     * implementation
     *
     * @param solver_name : Name of the solver to use
     * @param solver_type : Type of the solver {INTEGER, CONTINUOUS}
     * @param log_manager : A logger
     */
    SolverAbstract::Ptr create_solver(const std::string& solver_name) const;
    SolverAbstract::Ptr create_solver(const std::string& solver_name,
                                      const SolverLogManager& log_manager) const;
    SolverAbstract::Ptr create_solver(const std::string& solver_name,
                                      SOLVER_TYPE solver_type) const;
    SolverAbstract::Ptr create_solver(const std::string& solver_name,
                                      SOLVER_TYPE solver_type,
                                      const SolverLogManager& log_manager) const;

    /**
     * @brief Creates and returns to an object solver from the wanted
     * implementation
     *
     * @param solver_config : A solver configuration
     * @param solver_type : Type of the solver {INTEGER, CONTINUOUS}
     * @param log_manager : A logger
     */
    SolverAbstract::Ptr create_solver(const SolverConfig& solver_config) const;
    SolverAbstract::Ptr create_solver(const SolverConfig& solver_config,
                                      SOLVER_TYPE solver_type) const;
    SolverAbstract::Ptr create_solver(const SolverConfig& solver_config,
                                      const SolverLogManager& log_manager) const;
    SolverAbstract::Ptr create_solver(const SolverConfig& solver_config,
                                      SOLVER_TYPE solver_type,
                                      const SolverLogManager& log_manager) const;

    /**
     * @brief Copy constructor : Creates and returns to an object solver from the
     * wanted implementation by copying datas from same solver implementation
     *
     * @param to_copy : solver to copy
     */
    SolverAbstract::Ptr copy_solver(SolverAbstract* to_copy) const;

    /**
     * @brief Returns a reference to the list of available solvers
     */
    const std::vector<std::string>& get_solvers_list() const;

    bool isXpress_available_ = false;

    std::shared_ptr<ILoggerXpansion> logger_;
};
