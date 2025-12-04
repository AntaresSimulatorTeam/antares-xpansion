Feature: Benders Criterion files

  # this study has been generated with Antares Simulator Modeler, it has 3 vars, 2 candidates, the first var is not in the subproblem
  @fast @short @Benders
  Scenario: Benders_handle_mixed_order_of_var
    Given the study path is "data_test/Benders_handle_mixed_order_of_var"
    When I run benders with 1 proc(s)
    Then the simulation succeeds
    And the simulation takes less than 5 seconds
    And the solution is
      | variable                             | value |
      | continuous_generator_candidate.p_max | 100.0 |
      | discrete_generator_candidate.p_max   | 100.0 |