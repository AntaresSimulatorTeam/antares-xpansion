Feature: AGGREGATION in options.json file set the number cuts to add to the master problem in every iteration. We test that we have the right number of cut at every iteration and that we converge to the right overall cost 

    @short @full-launch @xpress
    Scenario: master problem with 14 candidates and 10 subproblems
        Given the study path is "data_test/investment_study"
        When I run benders with 6 proc(s)
        Then I check the value of the overall cost
    






