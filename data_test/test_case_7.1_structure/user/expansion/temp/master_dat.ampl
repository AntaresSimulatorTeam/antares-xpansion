
# this file has been copied by the R-package antaresXpansion
#
# this file should have been copied in the temporary folder of 
# current expansion optimisation, i.e.:
# study_path/user/expansion/temp/
#
# all data files are supposed to be located in the same folder


data;

#number of mc years
set YEAR := include in_mc.txt;

#number of weeks
set WEEK := include in_week.txt;

#iteration
set ITERATION := include in_iterations.txt;

#invested capacity in the simulated iterations
param z0 := include in_z0.txt ;


#investment candidates description
param : INV_CANDIDATE : c_inv  unit_size max_unit relaxed := include in_candidates.txt ;
param : restrained_lb restrained_ub := include in_out_capacitybounds.txt;


#bender cuts 
param : AVG_CUT : c0_avg := include in_avgcuts.txt ;
param : YEARLY_CUT_ALL : c0_yearly := include in_yearlycuts.txt ;
param : WEEKLY_CUT_ALL : c0_weekly := include in_weeklycuts.txt ;



#marg. cost of each investment (as returned by ANTARES)
param lambda_avg := include in_avgrentability.txt ;
param lambda_yearly := include in_yearlyrentability.txt ;
param lambda_weekly := include in_weeklyrentability.txt ;


# uper bound on total costs
param ub_cost := include in_ubcosts.txt;


# options 
param prob := include in_yweights.txt;

param : OPTION : option_default_value := include master_default_options.txt;
param : OPTION_REDIFINED : option_new_value = include in_options.txt;

