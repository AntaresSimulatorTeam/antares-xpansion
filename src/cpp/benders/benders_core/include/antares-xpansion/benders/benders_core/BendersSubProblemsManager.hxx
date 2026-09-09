#pragma once

#include <execution>
#include <filesystem>
#include <functional>
#include <mutex>

#include "BendersProblemFromFile.h"
#include "BendersStructsDatas.h"
#include "SubproblemBasisCache.h"
#include "SubproblemWorker.h"
#include "SubproblemWorkerFactory.h"
#include "antares-xpansion/benders/plugins/BendersPlugin.h"
#include "antares-xpansion/helpers/Timer.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "common.h"

using FastBeginHook = std::function<std::vector<std::pair<std::string, SubproblemWorkerPtr>>()>;
using CacheBeginHook = std::function<std::vector<std::pair<std::string, VariableMap>>()>;
using PostSolveHook = std::function<
  void(const std::string&, PlainData::SubProblemData&, const SubproblemWorkerPtr&)>;

/**
 * std execution policies don't share a base type so we can't just select
 * them in place in the foreach. This function allows the selection of policy
 * via template deduction.
 */
template<class lambda>
auto selectPolicy(lambda f, bool shouldParallelize)
{
    if (shouldParallelize)
    {
        return f(std::execution::par_unseq);
    }
    else
    {
        return f(std::execution::seq);
    }
}

/**
 * CRTP base class managing subproblem creation, storage, and solving
 * across the 3 cache modes (fast / disk-cache / skeleton).
 *
 * Derived classes customize:
 *   - MakeFastBeginHookImpl(Args...)  — which subproblems to solve (batch-scoped or all)
 *   - MakeCacheBeginHookImpl(Args...) — same for cache/skeleton modes
 *
 * Follow the same pattern as BendersCutsManager.hxx.
 */
template<typename Derived>
class BendersSubProblemsManager
{
protected:
    // Subproblem storage
    SubproblemsMapPtr subproblem_map_;
    CouplingMap coupling_map_;
    SubproblemBasisCache subproblem_basis_cache_;
    std::shared_ptr<SubproblemWorkerFactory> subproblem_worker_factory_;
    StrVector subproblems_;
    VariableMap problem_to_id_;

    // References to BendersBase-owned state
    CurrentIterationData& data_;
    const BendersBaseOptions& options_;
    std::shared_ptr<BendersPlugin> plugin_;
    Logger logger_;
    SolverLogManager& solver_log_manager_;
    std::shared_ptr<Output::OutputWriter> writer_;
    bool should_parallelize_;

    // Injectable hooks
    std::function<void(const std::vector<std::string>&)> on_variables_indices_set_;
    std::once_flag variable_indice_once_flag_;

public:
    BendersSubProblemsManager(CurrentIterationData& data,
                              const BendersBaseOptions& options,
                              std::shared_ptr<BendersPlugin> plugin,
                              Logger logger,
                              SolverLogManager& solver_log_manager,
                              std::shared_ptr<Output::OutputWriter> writer,
                              bool should_parallelize):
        data_(data),
        options_(options),
        plugin_(std::move(plugin)),
        logger_(std::move(logger)),
        solver_log_manager_(solver_log_manager),
        writer_(std::move(writer)),
        should_parallelize_(should_parallelize)
    {
    }

    // ---------------------------------------------------------------
    // CRTP dispatch: hooks (variadic to allow batch-scoped args)
    // ---------------------------------------------------------------

    template<typename... Args>
    FastBeginHook MakeFastBeginHook(Args&&... args)
    {
        return static_cast<Derived*>(this)->MakeFastBeginHookImpl(std::forward<Args>(args)...);
    }

    template<typename... Args>
    CacheBeginHook MakeCacheBeginHook(Args&&... args)
    {
        return static_cast<Derived*>(this)->MakeCacheBeginHookImpl(std::forward<Args>(args)...);
    }

    template<typename... Args>
    PostSolveHook MakePostSolveHook(Args&&... args)
    {
        return static_cast<Derived*>(this)->MakePostSolveHookImpl(std::forward<Args>(args)...);
    }

    // ---------------------------------------------------------------
    // Default hook implementations (iterate all subproblems)
    // ---------------------------------------------------------------

    FastBeginHook MakeFastBeginHookImpl()
    {
        return [this]()
        {
            std::vector<std::pair<std::string, SubproblemWorkerPtr>> nameAndWorkers;
            nameAndWorkers.reserve(subproblem_map_.size());
            for (const auto& [name, worker]: subproblem_map_)
            {
                nameAndWorkers.emplace_back(name, worker);
            }
            return nameAndWorkers;
        };
    }

    CacheBeginHook MakeCacheBeginHookImpl()
    {
        return [this]()
        {
            std::vector<std::pair<std::string, VariableMap>> nameAndVariableMap;
            nameAndVariableMap.reserve(coupling_map_.size());
            for (auto& [name, variables]: coupling_map_)
            {
                nameAndVariableMap.emplace_back(name, variables);
            }
            return nameAndVariableMap;
        };
    }

    // As we want to unify the api, for now the post solve hook is only needed to compute crieterion
    // stuff on BendersMPI, to avoid setting nullptr on the api from the benders sequential and
    // bybatch cases we decided to just impelement a hook that renders a void method
    auto MakePostSolveHookImpl()
    {
        return [](const std::string&, PlainData::SubProblemData&, const SubproblemWorkerPtr&) {};
    }

    // ---------------------------------------------------------------
    // Dispatcher
    // ---------------------------------------------------------------

    void GetSubproblemCut(SubProblemDataMap& subproblem_data_map,
                          const FastBeginHook& fast_begin_hook,
                          const CacheBeginHook& cache_begin_hook,
                          const PostSolveHook& post_solve_hook)
    {
        switch (options_.CACHE_PROBLEMS)
        {
        case 0:
            GetSubproblemCutFast(subproblem_data_map, fast_begin_hook, post_solve_hook);
            break;
        case 1:
            GetSubproblemCutCache(subproblem_data_map, cache_begin_hook, post_solve_hook);
            break;
        case 2:
            GetCompactInMemCuts(subproblem_data_map, cache_begin_hook, post_solve_hook);
            break;
        default:
            break;
        }
    }

    // ---------------------------------------------------------------
    // Cache=0: fast path — persistent workers
    // ---------------------------------------------------------------

    void GetSubproblemCutFast(SubProblemDataMap& subproblem_data_map,
                              const FastBeginHook& begin_hook,
                              const PostSolveHook& post_solve_hook)
    {
        auto nameAndWorkers = begin_hook();

        std::mutex m;
        std::exception_ptr first_exception;
        selectPolicy(
          [this, &nameAndWorkers, &m, &subproblem_data_map, &first_exception, &post_solve_hook](
            auto& policy)
          {
              std::for_each(policy,
                            nameAndWorkers.begin(),
                            nameAndWorkers.end(),
                            [this, &m, &subproblem_data_map, &first_exception, &post_solve_hook](
                              const std::pair<std::string, SubproblemWorkerPtr>& kvp)
                            {
                                try
                                {
                                    PlainData::SubProblemData subproblem_data;
                                    const auto& [name, worker] = kvp;
                                    SolveSubproblem(subproblem_data, name, worker, nullptr);
                                    post_solve_hook(name, subproblem_data, worker);

                                    std::lock_guard guard(m);
                                    subproblem_data_map[name] = subproblem_data;
                                }
                                catch (...)
                                {
                                    std::lock_guard guard(m);
                                    if (!first_exception)
                                    {
                                        first_exception = std::current_exception();
                                    }
                                }
                            });
          },
          should_parallelize_);
        if (first_exception)
        {
            std::rethrow_exception(first_exception);
        }
    }

    // ---------------------------------------------------------------
    // Cache=1: disk cache — recreate workers each iteration
    // ---------------------------------------------------------------

    void GetSubproblemCutCache(SubProblemDataMap& subproblem_data_map,
                               const CacheBeginHook& begin_hook,
                               const PostSolveHook& post_solve_hook)
    {
        auto nameAndVariableMap = begin_hook();

        std::mutex m;
        std::exception_ptr first_exception;

        selectPolicy(
          [this, &nameAndVariableMap, &m, &subproblem_data_map, &first_exception, &post_solve_hook](
            auto& policy)
          {
              std::for_each(policy,
                            nameAndVariableMap.begin(),
                            nameAndVariableMap.end(),
                            [this, &m, &subproblem_data_map, &first_exception, &post_solve_hook](
                              const std::pair<std::string, VariableMap>& kvp)
                            {
                                try
                                {
                                    const auto& [name, variables] = kvp;
                                    auto worker = makeSubproblemWorker(kvp);
                                    PlainData::SubProblemData subproblem_data;
                                    SolveSubproblem(subproblem_data,
                                                    name,
                                                    worker,
                                                    [this, &name, &worker]
                                                    { TryRestoreSubproblemBasis(name, worker); });
                                    post_solve_hook(name, subproblem_data, worker);
                                    std::lock_guard guard(m);
                                    subproblem_data_map[name] = subproblem_data;
                                    StoreSubproblemBasis(name, worker);

                                    std::call_once(
                                      variable_indice_once_flag_,
                                      [this](const auto& worker_)
                                      { SetSubproblemVariablesIndices(worker_); },
                                      *worker);
                                }
                                catch (...)
                                {
                                    std::lock_guard guard(m);
                                    if (!first_exception)
                                    {
                                        first_exception = std::current_exception();
                                    }
                                }
                            });
          },
          should_parallelize_);
        if (first_exception)
        {
            std::rethrow_exception(first_exception);
        }
    }

    // ---------------------------------------------------------------
    // Cache=2: skeleton — shared solver, morphed per subproblem
    // ---------------------------------------------------------------

    void GetCompactInMemCuts(SubProblemDataMap& subproblem_data_map,
                             const CacheBeginHook& begin_hook,
                             const PostSolveHook& post_solve_hook)
    {
        auto nameAndVariableMap = begin_hook();

        for (auto& [sub, variables]: nameAndVariableMap)
        {
            double slave_weights = SubproblemWeight(data_.nsubproblem, sub);

            auto subproblem_worker = subproblem_worker_factory_
                                       ->CreateSubSolverAbstract(sub, variables, slave_weights);

            PlainData::SubProblemData subproblem_data;
            SolveSubproblem(subproblem_data,
                            sub,
                            subproblem_worker,
                            [this, &sub] { subproblem_worker_factory_->ApplyBasis(sub); });
            post_solve_hook(sub, subproblem_data, subproblem_worker);

            subproblem_worker_factory_->GetBasis(sub);

            subproblem_data_map[sub] = subproblem_data;
        }
    }

    // ---------------------------------------------------------------
    // Core solve (uniform for all variants — no virtual override)
    // ---------------------------------------------------------------

    void SolveSubproblem(PlainData::SubProblemData& subproblem_data,
                         const std::string& name,
                         const std::shared_ptr<SubproblemWorker>& worker,
                         const std::function<void()>& post_reset_hook)
    {
        Timer subproblem_timer;
        worker->fix_to(data_.x_cut);
        plugin_->OnBendersSubResolutionStart(worker, name);
        // with this hook we try to avoid duplicating the whole bloc since the diffence is just the
        // overhead of the method finally so for every special case we just set a lambda function
        if (post_reset_hook && plugin_->ShouldRestoreSubproblemBasis())
        {
            post_reset_hook();
        }

        int num_micro_iter(0);
        if (options_.MICRO_ITERATIONS)
        {
            bool added_rows = true;
            plugin_->OnBendersMicroIterationStart();
            while (added_rows)
            {
                auto t1 = std::chrono::steady_clock::now();
                worker->solve(subproblem_data.lpstatus,
                              options_.OUTPUTROOT,
                              options_.LAST_MASTER_MPS + MPS_SUFFIX,
                              writer_);

                auto t2 = std::chrono::steady_clock::now();
                auto elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
                                              t2 - t1)
                                              .count();

                num_micro_iter++;
                plugin_->OnBendersMicroIterationEnd(name,
                                                    added_rows,
                                                    std::to_string(elapsed_microseconds),
                                                    data_.it,
                                                    num_micro_iter);
            }
        }
        else
        {
            worker->solve(subproblem_data.lpstatus,
                          options_.OUTPUTROOT,
                          options_.LAST_MASTER_MPS + MPS_SUFFIX,
                          writer_);
        }

        worker->get_value(subproblem_data.subproblem_cost);
        worker->get_subgradient(subproblem_data.var_name_and_subgradient);
        worker->get_splex_num_of_ite_last(subproblem_data.simplex_iter);
        subproblem_data.subproblem_timer = subproblem_timer.elapsed();

        plugin_->OnBendersSubResolutionEnd();
    }

    // ---------------------------------------------------------------
    // Creation / storage
    // ---------------------------------------------------------------

    void AddSubproblem(const std::pair<std::string, VariableMap>& kvp)
    {
        std::shared_ptr<IBendersProblemProvider>
          benders_problem_provider = std::make_shared<BendersProblemFromFile>(
            GetSubproblemPath(kvp.first));
        subproblem_map_[kvp.first] = std::make_shared<SubproblemWorker>(
          kvp.second,
          SubproblemWeight(data_.nsubproblem, kvp.first),
          options_.SOLVER_NAME,
          options_.LOG_LEVEL,
          solver_log_manager_,
          logger_,
          options_.PROBLEMS_FORMAT,
          benders_problem_provider.get());
    }

    void AddSubproblemName(const std::string& name)
    {
        subproblems_.push_back(name);
    }

    std::shared_ptr<SubproblemWorker> makeSubproblemWorker(
      const std::pair<std::string, VariableMap>& kvp) const
    {
        std::shared_ptr<IBendersProblemProvider>
          benders_problem_provider = std::make_shared<BendersProblemFromFile>(
            GetSubproblemPath(kvp.first));
        return std::make_shared<SubproblemWorker>(kvp.second,
                                                  SubproblemWeight(data_.nsubproblem, kvp.first),
                                                  options_.SOLVER_NAME,
                                                  options_.LOG_LEVEL,
                                                  solver_log_manager_,
                                                  logger_,
                                                  options_.PROBLEMS_FORMAT,
                                                  benders_problem_provider.get());
    }

    void free_subproblems()
    {
        for (auto& ptr: subproblem_map_)
        {
            ptr.second->free();
        }
    }

    void MatchProblemToId()
    {
        int count = 0;
        for (const auto& problem: coupling_map_)
        {
            problem_to_id_[problem.first] = count;
            count++;
        }
    }

    // ---------------------------------------------------------------
    // Basis management
    // ---------------------------------------------------------------

    void StoreSubproblemBasis(const std::string& name,
                              const std::shared_ptr<SubproblemWorker>& worker)
    {
        subproblem_basis_cache_.Store(name, *worker->_solver);
    }

    void TryRestoreSubproblemBasis(const std::string& name,
                                   const std::shared_ptr<SubproblemWorker>& worker)
    {
        subproblem_basis_cache_.TryRestore(name, *worker->_solver, logger_);
    }

    // ---------------------------------------------------------------
    // Variable indices (criterion support)
    // ---------------------------------------------------------------

    void SetSubproblemVariablesIndices(const SubproblemWorker& subproblem)
    {
        auto&& col_names = subproblem._solver->get_col_names();
        if (on_variables_indices_set_)
        {
            on_variables_indices_set_(col_names);
        }
    }

    void SetSubproblemsVariablesIndices()
    {
        if (!subproblem_map_.empty())
        {
            auto subproblem = subproblem_map_.begin();
            SetSubproblemVariablesIndices(*subproblem->second);
        }
    }

    // ---------------------------------------------------------------
    // Accessors
    // ---------------------------------------------------------------

    [[nodiscard]] std::filesystem::path GetSubproblemPath(const std::string& subproblem_name) const
    {
        return std::filesystem::path(options_.INPUTROOT) / subproblem_name;
    }

    [[nodiscard]] double SubproblemWeight(int subproblem_count, const std::string& name) const
    {
        if (options_.SLAVE_WEIGHT == SUBPROBLEM_WEIGHT_UNIFORM_CST_STR)
        {
            return 1 / static_cast<double>(subproblem_count);
        }
        else if (options_.SLAVE_WEIGHT == SUBPROBLEM_WEIGHT_CST_STR)
        {
            const double weight(options_.SLAVE_WEIGHT_VALUE);
            return 1 / weight;
        }
        else
        {
            return options_.weights.find(name)->second;
        }
    }

    SubproblemsMapPtr GetSubProblemMap() const
    {
        return subproblem_map_;
    }

    StrVector GetSubProblemNames() const
    {
        return subproblems_;
    }

    const VariableMap& GetProblemToId() const
    {
        return problem_to_id_;
    }

    int ProblemToId(const std::string& problem_name) const
    {
        return problem_to_id_.at(problem_name);
    }

    // ---------------------------------------------------------------
    // Hook setters
    // ---------------------------------------------------------------

    void SetOnVariablesIndicesSet(std::function<void(const std::vector<std::string>&)> callback)
    {
        on_variables_indices_set_ = std::move(callback);
    }

    // we need it in the cache problem == 2 to create the skeleton
    void BuildSubproblemWorkerFactory(int cache_problems, boost::mpi::communicator* world = nullptr)
    {
        if (cache_problems == 2)
        {
            subproblem_worker_factory_ = std::make_shared<SubproblemWorkerFactory>(
              options_.INPUTROOT,
              logger_,
              options_.SOLVER_NAME,
              options_.LOG_LEVEL,
              options_.PROBLEMS_FORMAT,
              subproblems_,
              solver_log_manager_,
              world);
        }
    }

    std::shared_ptr<SolverAbstract> GetFactorySolver() const
    {
        return subproblem_worker_factory_ ? subproblem_worker_factory_->GetSolver() : nullptr;
    }

    void SetCouplingMap(const CouplingMap& coupling_map)
    {
        coupling_map_ = coupling_map;
    }

    CouplingMap& GetCouplingMap()
    {
        return coupling_map_;
    }

    const CouplingMap& GetCouplingMap() const
    {
        return coupling_map_;
    }
};
