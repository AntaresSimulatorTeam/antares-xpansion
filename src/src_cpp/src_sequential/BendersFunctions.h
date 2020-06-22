#pragma once

#include "SlaveCut.h"
#include "Worker.h"
#include "WorkerSlave.h"
#include "WorkerMaster.h"
#include "WorkerTrace.h"
#include "BendersOptions.h"


void init(BendersData & data);
void init_log(std::ostream&stream, int const log_level);


void print_log(std::ostream&stream, BendersData const & data, int const log_level);
void print_csv(BendersTrace & trace, Str2Int & problem_to_id, BendersData const & data, BendersOptions const & options);
void print_master_csv(std::ostream&stream, WorkerMasterDataPtr & trace, Point const & xopt, std::string const & name, int const nslaves);
void print_cut_csv(std::ostream&stream, SlaveCutDataHandler const & handler, std::string const & name, int const islaves);
void print_solution(std::ostream&stream, Point const & point, bool const filter_non_zero);
void print_active_cut(ActiveCutStorage const & active_cuts, BendersOptions const & options);


void update_best_ub(double & best_ub, double const & ub, Point & bestx, Point const & x0);
void bound_simplex_iter(int simplexiter, BendersData & data);
bool stopping_criterion(BendersData & data, BendersOptions const & options);
void update_trace(BendersTrace & trace, BendersData const & data);
void check_status(AllCutPackage const & all_package, BendersData const & data);


void get_master_value(WorkerMasterPtr & master, BendersData & data, BendersOptions const & options);
void get_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, BendersOptions const & options, BendersData const & data);
void get_random_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, StrVector const & random_slaves, BendersOptions const & options, BendersData const & data);


void sort_cut_slave(AllCutPackage const & all_package, WorkerMasterPtr & master, Str2Int & problem_to_id, BendersTrace & trace, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options, SlaveCutId & slave_cut_id);
void sort_cut_slave_aggregate(AllCutPackage const & all_package, WorkerMasterPtr & master, Str2Int & problem_to_id, BendersTrace & trace, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options);
void add_random_cuts(WorkerMasterPtr & master, AllCutPackage const & all_package, Str2Int & problem_to_id, BendersTrace & trace, BendersOptions & options, BendersData & data);
void build_cut_full(WorkerMasterPtr & master, AllCutPackage const & all_package, Str2Int & problem_to_id, BendersTrace & trace, SlaveCutId & slave_cut_id, AllCutStorage & all_cuts_storage, DynamicAggregateCuts & dynamic_aggregate_cuts, BendersData & data, BendersOptions & options);

void dynamic_aggregation(WorkerMasterPtr & master, DynamicAggregateCuts & dynamic_aggregate_cuts, AllCutPackage const & all_package, Str2Int & problem_to_id, BendersData const & data, BendersOptions const & options);
void dynamic_iteration(WorkerMasterPtr & master, DynamicAggregateCuts & dynamic_aggregate_cuts, AllCutPackage const & all_package, Str2Int & problem_to_id, BendersData const & data, BendersOptions const & options);
void store_current_aggregate_cut(DynamicAggregateCuts & dynamic_cuts, AllCutPackage const & all_package, Str2Int & problem_to_id, BendersData const & data, BendersOptions const & options);
void store_iter_aggregate_cut(DynamicAggregateCuts & dynamic_cuts, AllCutPackage const & all_package, Str2Int & problem_to_id, Point const & x0, BendersData const & data, BendersOptions const & options);
void gather_cut(DynamicAggregateCuts & dynamic_cuts, WorkerMasterPtr & master, BendersOptions const & options, int const nconstraints, Str2Int const & problem_to_id);


void get_slave_basis(SimplexBasisPackage & simplex_basis_package, SlavesMapPtr & map_slaves);
void sort_basis(AllBasisPackage const & all_basis_package, Str2Int & problem_to_id, SimplexBasisStorage & basis_storage, BendersData & data);
void update_active_cuts(WorkerMasterPtr & master, ActiveCutStorage & active_cuts, SlaveCutId & cut_id, int const it);
