Feature: Validate presolve step feature

    @short @presolve
    Scenario:
        Given the study path is "data_test/examples/additionnal-constraints"
        And solver is "Xpress"
        When I run step problem_generation in memory followed by step presolve
        Then the return status is 0
        And the generated subproblems have between 2400 and 2500 rows
        And the generated subproblems have between 3400 and 3600 cols
        And the generated subproblems have between 11000 and 12000 elements