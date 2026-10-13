Feature: Resume modes of Benders

@fast @short @resume
Scenario: Benders runs for 2 iterations then we resume it from there to convergence
    Given the study path is "data_test/ieee96_base"
    When I set MAX_ITERATIONS to 2
    And I run benders with 1 proc(s)
    Then the simulation succeeds
    And the problem_status is "limit reached"
    And the expected investment cost is 3.2615786040443573
    When I set MAX_ITERATIONS to -1
    And I set RESUME to "resume"
    And I run benders with 1 proc(s)
    Then the simulation succeeds
    And the problem_status is "OPTIMAL"
    And the expected investment cost is 1.6307893020221786

@fast @short @resume 
Scenario: Benders runs until optimality then we relaunch it with hot start 
    Given the study path is "data_test/ieee96_base"
    When I run benders with 1 proc(s) 
    Then the simulation succeeds
    And the problem_status is "OPTIMAL" 
    And the expected investment cost is 1.6307893020221786
    When I set RESUME to "hot_start"
    And I run benders with 1 proc(s)
    Then the simulation succeeds
    And the number of iterations is 1

