
# this file has been copied by the R-package antaresXpansion
#
# this file should have been copied in the temporary folder of 
# current expansion optimisation, i.e.:
# study_path/user/expansion/temp/
#
# this file describes the master problem of the benders decomposition


#-------------
#--- SETS ----
#-------------

# set of options
set OPTION;  # set of all options
set OPTION_REDIFINED; # set of all changed options

# set of investment candidates
set INV_CANDIDATE ;

# set of MC years
set YEAR ;

# set of weeks
set WEEK ;

#set of benders iterations
set ITERATION ordered;

# set of average bender cuts
set AVG_CUT within {ITERATION} ;

# set of yearly bender cuts
set YEARLY_CUT_ALL within {ITERATION, YEAR} ;

# set of weekly bender cuts
set WEEKLY_CUT_ALL within {ITERATION, YEAR, WEEK} ;


#----------------
#--- OPTIONS ----
#----------------

param option_default_value {OPTION} symbolic; 
param option_new_value {OPTION_REDIFINED} symbolic; 
param option_v {o in OPTION} := if (o in OPTION_REDIFINED) then option_new_value[o] else option_default_value[o] symbolic;



#-------------------
#--- PARAMETERS ----
#-------------------

# investment candidates
param c_inv{INV_CANDIDATE};      	# investment costs 
param unit_size{INV_CANDIDATE};  	# unit of each investment step
param max_unit{INV_CANDIDATE};	 	# max number of units which can be invested
param relaxed{INV_CANDIDATE} symbolic ;	  # (true or false) is the investment made continuously, or with steps ?
param restrained_ub{INV_CANDIDATE} default Infinity; # uper bound on invested capacity
param restrained_lb{INV_CANDIDATE} default 0 ; # lower bound on invested capacity
param tol_bounds := 1e-3 ;   # to avoid infeasibility
param z0{ITERATION, INV_CANDIDATE}  ;# invested capacity of each candidates for the given iteratoin



# average cut
param c0_avg{AVG_CUT} ;                 	# total costs (operation + investment) for the given iteration
param lambda_avg{AVG_CUT, INV_CANDIDATE} ;	#  rentability (average value over all MC years)

# yearly cut
param c0_yearly{YEARLY_CUT_ALL} ;    					# yearly total costs
param lambda_yearly{YEARLY_CUT_ALL, INV_CANDIDATE} ;    #  rentability (yearly values)

#weekly cut
param c0_weekly{WEEKLY_CUT_ALL} ;   					# weekly total costs
param lambda_weekly{WEEKLY_CUT_ALL, INV_CANDIDATE} ;    # rentability (weekly values)

# other
param prob{YEAR}; 					# probability of occurence of each MC year
param ub_cost default Infinity;     # ub on objective function (total of all costs)
param epsilon default 10000;     # the sensibility of the optimal solution

#-------------------------------------
#--- SETS of CUTS WITHOUT DOUBLON ----
#-------------------------------------

param tol := 1e-3;  # tolerance to identify doublons


# yearly 

param cut_in_zero_y{(c,y) in YEARLY_CUT_ALL} := c0_yearly[c,y] - sum{z in INV_CANDIDATE} (lambda_yearly[c,y,z] * (0 - z0[c,z]));
set CUT_COMBN_Y := setof{(c1, y) in YEARLY_CUT_ALL, (c2, y) in YEARLY_CUT_ALL} (c1, c2, y);

param difference_y{(c1, c2, y) in CUT_COMBN_Y} := sum{z in INV_CANDIDATE} (lambda_yearly[c1,y,z] - lambda_yearly[c2,y,z])^2;
param isdoublon_y{(c1,y) in YEARLY_CUT_ALL} := if ((min{(c2, y) in YEARLY_CUT_ALL} difference_y[c1, c2, y]) < tol) then 1 else 0;
param deleteit1_y{(c1,y) in YEARLY_CUT_ALL} := if ((isdoublon_y[c1,y]) == 1 and ((max{(c2, y) in YEARLY_CUT_ALL : difference_y[c1, c2, y] < tol} cut_in_zero_y[c2,y]) > cut_in_zero_y[c1, y])) then 1 else 0;
param deleteit2_y{(c1,y) in YEARLY_CUT_ALL} := if ((isdoublon_y[c1,y]) == 1 and (deleteit1_y[c1,y] == 0) and (card{(c2,y) in YEARLY_CUT_ALL : (ord(c2, ITERATION) > ord(c1, ITERATION)) and (deleteit1_y[c2,y] == 0) and (difference_y[c1, c2, y] < tol)} >= 1)) then 1 else 0;

set YEARLY_CUT := setof{(c, y) in YEARLY_CUT_ALL : deleteit1_y[c,y] == 0 and deleteit2_y[c,y] ==0} (c, y);


# weekly 

param cut_in_zero_w{(c,y,w) in WEEKLY_CUT_ALL} := c0_weekly[c,y,w] - sum{z in INV_CANDIDATE} (lambda_weekly[c,y,w,z] * (0 - z0[c,z]));
set CUT_COMBN_W := setof{(c1, y, w) in WEEKLY_CUT_ALL, (c2, y, w) in WEEKLY_CUT_ALL} (c1, c2, y, w);

param difference_w{(c1, c2, y, w) in CUT_COMBN_W} := sum{z in INV_CANDIDATE} (lambda_weekly[c1,y,w,z] - lambda_weekly[c2,y,w,z])^2;
param isdoublon_w{(c1,y,w) in WEEKLY_CUT_ALL} := if ((min{(c2, y, w) in WEEKLY_CUT_ALL} difference_w[c1, c2, y, w]) < tol) then 1 else 0;
param deleteit1_w{(c1,y,w) in WEEKLY_CUT_ALL} := if ((isdoublon_w[c1,y,w]) == 1 and ((max{(c2, y, w) in WEEKLY_CUT_ALL : difference_w[c1, c2, y, w] < tol} cut_in_zero_w[c2,y,w]) > cut_in_zero_w[c1, y, w])) then 1 else 0;
param deleteit2_w{(c1,y,w) in WEEKLY_CUT_ALL} := if ((isdoublon_w[c1,y,w]) == 1 and (deleteit1_w[c1,y,w] == 0) and (card{(c2,y,w) in WEEKLY_CUT_ALL : (ord(c2, ITERATION) > ord(c1, ITERATION)) and (deleteit1_w[c2,y,w] == 0) and (difference_w[c1, c2, y, w] < tol)} >= 1)) then 1 else 0;

set WEEKLY_CUT := setof{(c, y, w) in WEEKLY_CUT_ALL : deleteit1_w[c,y,w] == 0 and deleteit2_w[c,y,w] ==0} (c, y, w);


#------------------
#--- VARIABLES ----
#------------------

var Invested_capacity{INV_CANDIDATE} >= 0;       # capacity invested
var N_invested{INV_CANDIDATE} integer >=0;  # number of units invested

var Theta{YEAR, WEEK};


#-----------
#--- LP ----
#-----------

# objectives :
minimize master : sum{y in YEAR} ( prob[y] * sum{w in WEEK} Theta[y,w]) ;

minimize bound_capacity_min {z in INV_CANDIDATE} : Invested_capacity[z];
maximize bound_capacity_max {z in INV_CANDIDATE} : Invested_capacity[z];

# ub on objective function
subject to ub {if (option_v["ub_constraint"] or option_v["solve_bounds"] or (card(ITERATION) mod option_v["solve_bounds_frequency"] == 0))}: 
sum{y in YEAR} ( prob[y] * sum{w in WEEK} Theta[y,w]) <= ub_cost + epsilon + 1;


# description of invested capacity :
subject to bounds_on_invested_capacity_relaxed{z in INV_CANDIDATE : relaxed[z] == "true"} : Invested_capacity[z] <= max_unit[z] * unit_size[z]; 
subject to bounds_on_invested_capacity_integer{z in INV_CANDIDATE : relaxed[z] != "true"} : N_invested[z] <= max_unit[z];
subject to integer_constraint{z in INV_CANDIDATE : relaxed[z] != "true"} : Invested_capacity[z] = unit_size[z] * N_invested[z];		 

subject to restrained_bounds_on_capacity {z in INV_CANDIDATE} : restrained_lb[z] - tol_bounds<= Invested_capacity[z] <= restrained_ub[z] + tol_bounds; 

# bender's cut :
subject to cut_avg{c in AVG_CUT} : sum{y in YEAR} ( prob[y] * sum{w in WEEK} Theta[y,w]) >=   c0_avg[c] - sum{z in INV_CANDIDATE}(lambda_avg[c,z] * (Invested_capacity[z] - z0[c,z])) ;

subject to cut_yearly{(c,y) in YEARLY_CUT} : sum{w in WEEK} Theta[y,w] >=  c0_yearly[c,y] - sum{z in INV_CANDIDATE} (lambda_yearly[c,y,z] * (Invested_capacity[z] - z0[c,z]));

subject to cut_weekly{(c,y,w) in WEEKLY_CUT} : Theta[y,w] >=  c0_weekly[c,y,w] - sum{z in INV_CANDIDATE} (lambda_weekly[c,y,w,z] * (Invested_capacity[z] - z0[c,z]));

# additional constraints
include in_additional_constraints.txt;
