
# this file has been copied by the R-package antaresXpansion
#
# this file should have been copied in the temporary folder of 
# current expansion optimisation, i.e.:
# study_path/user/expansion/temp/

reset;

model master_mod.ampl;    # load model
data  master_dat.ampl;     # load data

# set options
option solver (option_v["solver"]);    # set solver
option relax_integrality  (option_v["relaxed"]);  # relaxed master problem ?
option presolve  (option_v["presolve"]);  # relaxed master problem ?


# set solver options
if (option_v["solver"]) == "amplxpress" then 
	option xpress_options "presolve=0 mipabsstop=0 miprelstop= 0";
 
if (option_v["solver"]) == "cbc" then 
	option cbc_options "PassP=0";
 
   		


# compute restricted bounds on Invested capacities
if (num0(option_v["solve_bounds"]) == 1 or (card(ITERATION) mod num0(option_v["solve_bounds_frequency"]) == 0)) then
{
   let epsilon := num0(option_v["epsilon"]);
   for{z in INV_CANDIDATE}
   {
   	  solve bound_capacity_min[z] >> out_log.txt;
   		if solve_result = "infeasible" then break;
   		let restrained_lb[z] := bound_capacity_min[z];
   		
   		solve bound_capacity_max[z] >> out_log.txt;
   		if solve_result = "infeasible" then break;
   		let restrained_ub[z] := bound_capacity_max[z];	
   } 
}


# solver master problem
if (num0(option_v["solve_master"]) == 1) then
{
	solve master >> out_log.txt;

	# relaxed ub constraint if problem is infeasible
	if solve_result = "infeasible" then 
	{
		drop ub;
		solve master >> out_log.txt;
		if solve_result = "infeasible" then printf "error infeasible problem";
	}
	# correct Invested_capacity, slight negative values are possible due to constraint tolerances
	let {z in INV_CANDIDATE} Invested_capacity[z] := max(0, Invested_capacity[z]);
	
}



# write results (in the same folder)
if (num0(option_v["solve_master"]) == 1) then
{
	printf "" > out_solutionmaster.txt;
  printf{z in INV_CANDIDATE} "%s;%f\n", z, Invested_capacity[z] >> out_solutionmaster.txt;
	printf "%f\n", master >> out_underestimator.txt;
}

if (num0(option_v["solve_bounds"]) == 1 or (card(ITERATION) mod num0(option_v["solve_bounds_frequency"]) == 0)) then
{
	printf "" > in_out_capacitybounds.txt;
  printf{z in INV_CANDIDATE} "%s %f %f\n", z, restrained_lb[z], restrained_ub[z] >> in_out_capacitybounds.txt;
} 

if (num0(option_v["write_time"]) == 1) then
{
	printf "%s;%f;%f;%f\n", card(ITERATION), _ampl_time + _total_solve_time  , _ampl_time  , _total_solve_time >> out_ampltime.txt;  
};

# write theta

#for {y in YEAR, w in WEEK}
#{
#	printf "%s;%s;%s;%f\n", card(ITERATION), y,w, Theta[y,w] >> out_theta.txt;
#}


