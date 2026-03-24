# These tests may fail after any change in the presolve settings or a solver upgrade (if it implies changes in the way presolve is done). In this case, checks must be done manually to define the new expected results.
Feature: Validate presolve step feature

  @short @presolve @noci
  Scenario: "Presolve on additionnal-constraint"
    Given the study path is "data_test/examples/additionnal-constraints"
    And solver is "Xpress"
    When I run step problem_generation in-memory followed by step presolve
    Then the return status is 0
    And the generated subproblems have between 2400 and 2600 rows
    And the generated subproblems have between 3375 and 3700 cols
    And the generated subproblems have between 11000 and 12000 elements

    # The no-presolve tests are used to clearly show the difference between the sizes of the problem that are generated with and without presolve. Moreover, problems generated without presolve have deterministic size.
  @short @no-presolve @noci
  Scenario: "No presolve on additionnal-constraint"
    Given the study path is "data_test/examples/additionnal-constraints"
    And solver is "Xpress"
    When I run step problem_generation in-memory mps
    Then the return status is 0
    And the generated subproblems have 6223 rows
    And the generated subproblems have 5549 cols
    And the generated subproblems have between 15050 and 15250 elements

  @short @presolve
  Scenario: "Presolve on xpansion-test-one-link-two-candidates"
    Given the study path is "data_test/examples/xpansion-test-one-link-two-candidates"
    And solver is "Xpress"
    When I run step problem_generation in-memory followed by step presolve
    Then the return status is 0
    And the generated subproblems have between 900 and 1000 rows
    And the generated subproblems have between 1600 and 1800 cols
    And the generated subproblems have between 4800 and 4800 elements

  @short @no-presolve
  Scenario: "No presolve on xpansion-test-one-link-two-candidates"
    Given the study path is "data_test/examples/xpansion-test-one-link-two-candidates"
    And solver is "Xpress"
    When I run step problem_generation in-memory mps
    Then the return status is 0
    And the generated subproblems have 1344 rows
    And the generated subproblems have 1850 cols
    And the generated subproblems have 4704 elements