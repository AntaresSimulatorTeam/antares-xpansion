
#include <utility>

#include "SolverXpress.h"
#include "antares-xpansion/multisolver_interface/environment.h"

#ifdef COIN_OR
#include "SolverCbc.h"
#include "SolverClp.h"
#endif
#include "antares-xpansion/multisolver_interface/SolverConfig.h"
#include "antares-xpansion/multisolver_interface/SolverFactory.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

namespace
{
// not std::string, static initialization fiasco with tests static
// initialization
const char* COIN_STR("COIN");
const char* CBC_STR("CBC");
const char* CLP_STR("CLP");
const char* XPRESS_STR("XPRESS");

#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
std::vector<std::string> available_solvers;
std::once_flag solver_flag;

void GetAvailableSolversInternal(std::shared_ptr<ILoggerXpansion> logger)
{
    if (available_solvers.empty())
    {
        static XpressManager xpress_manager;
        LoadXpress::XpressLoader xpress_loader(std::move(logger));
        if (xpress_loader.XpressIsCorrectlyInstalled(true))
        {
            available_solvers.emplace_back(XPRESS_STR);
        }
#ifdef COIN_OR
        available_solvers.emplace_back(CLP_STR);
        available_solvers.emplace_back(CBC_STR);
#endif
    }
}
} // namespace

std::vector<std::string> SolverLoader::GetAvailableSolvers(std::shared_ptr<ILoggerXpansion> logger)
{
    std::call_once(solver_flag, GetAvailableSolversInternal, logger);
    return available_solvers;
}

/**
 * @brief Returns a list of supported solvers
 * Supported doesn't mean available, for exemple if licence are not available
 * @return
 */
std::vector<std::string> SolverLoader::GetSupportedSolvers()
{
    static std::vector<std::string> supported_solvers;
    if (supported_solvers.empty())
    {
        supported_solvers.emplace_back(XPRESS_STR);
#ifdef COIN_OR
        supported_solvers.emplace_back(CLP_STR);
        supported_solvers.emplace_back(CBC_STR);
#endif
    }
    return supported_solvers;
}

SolverFactory::SolverFactory(std::shared_ptr<ILoggerXpansion> logger):
    _available_solvers(SolverLoader::GetAvailableSolvers(logger)),
    logger_(std::move(logger))
{
    isXpress_available_ = std::find(available_solvers.cbegin(),
                                    available_solvers.cend(),
                                    XPRESS_STR)
                          != available_solvers.cend();
}

std::shared_ptr<SolverAbstract> SolverFactory::create_solver(const std::string& solver_name,
                                                             const SOLVER_TYPE solver_type) const
{
    try
    {
        return create_solver(SolverConfig(solver_name), solver_type);
    }
    catch (LogUtils::XpansionError<std::invalid_argument>& ex)
    {
        throw InvalidSolverNameException(solver_name, LOGLOCATION);
    }
}

std::shared_ptr<SolverAbstract> SolverFactory::create_solver(
  const std::string& solver_name,
  const SOLVER_TYPE solver_type,
  const SolverLogManager& log_manager) const
{
    try
    {
        return create_solver(SolverConfig(solver_name), solver_type, log_manager);
    }
    catch (LogUtils::XpansionError<std::invalid_argument>& ex)
    {
        throw InvalidSolverNameException(solver_name, LOGLOCATION);
    }
}

std::shared_ptr<SolverAbstract> SolverFactory::create_solver(const std::string& solver_name) const
{
    try
    {
        return create_solver(SolverConfig(solver_name));
    }
    catch (LogUtils::XpansionError<std::invalid_argument>& ex)
    {
        throw InvalidSolverNameException(solver_name, LOGLOCATION);
    }
}

std::shared_ptr<SolverAbstract> SolverFactory::create_solver(
  const std::string& solver_name,
  const SolverLogManager& log_manager) const
{
    try
    {
        return create_solver(SolverConfig(solver_name), log_manager);
    }
    catch (LogUtils::XpansionError<std::invalid_argument>& ex)
    {
        throw InvalidSolverNameException(solver_name, LOGLOCATION);
    }
}

std::shared_ptr<SolverAbstract> SolverFactory::create_solver(
  const SolverConfig& solver_config) const
{
    std::shared_ptr<SolverAbstract> ret;
    if (solver_config.Name().empty())
    {
        throw InvalidSolverNameException(solver_config.Name(), LOGLOCATION);
    }
    else if (isXpress_available_ && solver_config == XPRESS_STR)
    {
        ret = std::make_shared<SolverXpress>();
    }
#ifdef COIN_OR
    else if (solver_config == CLP_STR)
    {
        ret = std::make_shared<SolverClp>();
    }
    else if (solver_config == CBC_STR)
    {
        ret = std::make_shared<SolverCbc>();
    }
#endif
    else
    {
        throw InvalidSolverNameException(solver_config.Name(), LOGLOCATION);
    }
    ret->init();
    return ret;
}

std::shared_ptr<SolverAbstract> SolverFactory::create_solver(const SolverConfig& solver_config,
                                                             const SOLVER_TYPE solver_type) const
{
    std::shared_ptr<SolverAbstract> ret;
    if (solver_config.Name().empty())
    {
        throw InvalidSolverNameException(solver_config.Name(), LOGLOCATION);
    }
    else if (isXpress_available_ && solver_config == XPRESS_STR)
    {
        ret = std::make_shared<SolverXpress>();
    }
#ifdef COIN_OR
    if (solver_config == COIN_STR && solver_type == SOLVER_TYPE::CONTINUOUS)
    {
        ret = std::make_shared<SolverClp>();
    }
    else if (solver_config == COIN_STR && solver_type == SOLVER_TYPE::INTEGER)
    {
        ret = std::make_shared<SolverCbc>();
    }
#endif
    else
    {
        throw InvalidSolverNameException(solver_config.Name(), LOGLOCATION);
    }
    ret->init();
    return ret;
}

std::shared_ptr<SolverAbstract> SolverFactory::create_solver(
  const SolverConfig& solver_config,
  const SolverLogManager& log_manager) const
{
    if (solver_config.Name().empty())
    {
        throw InvalidSolverNameException(solver_config.Name(), LOGLOCATION);
    }
    std::shared_ptr<SolverAbstract> ret;
    if (isXpress_available_ && solver_config == XPRESS_STR)
    {
        ret = std::make_shared<SolverXpress>(log_manager);
    }
#ifdef COIN_OR
    else if (solver_config == CLP_STR)
    {
        ret = std::make_shared<SolverClp>(log_manager);
    }
    else if (solver_config == CBC_STR)
    {
        ret = std::make_shared<SolverCbc>(log_manager);
    }
#endif
    else
    {
        throw InvalidSolverNameException(solver_config.Name(), LOGLOCATION);
    }
    ret->init();
    return ret;
}

std::shared_ptr<SolverAbstract> SolverFactory::create_solver(
  const SolverConfig& solver_config,
  const SOLVER_TYPE solver_type,
  const SolverLogManager& log_manager) const
{
#ifdef COIN_OR
    if (solver_config == COIN_STR && solver_type == SOLVER_TYPE::CONTINUOUS)
    {
        return std::make_shared<SolverClp>(log_manager);
    }
    else if (solver_config == COIN_STR && solver_type == SOLVER_TYPE::INTEGER)
    {
        return std::make_shared<SolverCbc>(log_manager);
    }
#endif
    return create_solver(solver_config, log_manager);
}

std::shared_ptr<SolverAbstract> SolverFactory::copy_solver(const SolverAbstract& to_copy) const
{
    return std::shared_ptr<SolverAbstract>(to_copy.clone());
}

const std::vector<std::string>& SolverFactory::get_solvers_list() const
{
    return _available_solvers;
}
