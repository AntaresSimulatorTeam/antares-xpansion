Feature: NB_CUTS_PER_ITER in options.json file set the number cuts to add to the master problem in every iteration. We test that we have the right number of cut at every iteration and that we converge to the right overall cost 

    @short @full-launch @xpress
    Scenario: master problem with 14 candidates and 10 subproblems
        Given the study path is "data_test/test_benders_cut_aggregation"
        When I run benders with 1 proc(s)
        Then the simulation succeeds
        AND the expected overall cost is 20.592373390401711
    






