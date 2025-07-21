Feature: Validate presolve step feature

    @short @presolve
    Scenario: "Presolve on additionnal-constraint"
        Given the study path is "data_test/examples/additionnal-constraints"
        And solver is "Xpress"
        When I run step problem_generation in memory followed by step presolve
        Then the return status is 0
        And the generated subproblems have between 2400 and 2500 rows
        And the generated subproblems have between 3400 and 3600 cols
        And the generated subproblems have between 11000 and 12000 elements
    
    @short @presolve
    Scenario: "Presolve on xpansion-test-one-link-two-candidates"
        Given the study path is "data_test/examples/xpansion-test-one-link-two-candidates"
        And solver is "Xpress"
        When I run step problem_generation in memory followed by step presolve
        Then the return status is 0
        And the generated subproblems have between 900 and 1000 rows
        And the generated subproblems have between 1600 and 1800 cols
        And the generated subproblems have between 4800 and 4800 elements