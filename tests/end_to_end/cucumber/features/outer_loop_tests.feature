Feature: outer loop tests

  @fast @short @outerloop
  Scenario: a system with 4 nodes, on 1 timestep, 2 scenarios
    Given the study path is "data_test/external_loop_test"
    When I run outer loop with 1 proc(s)
    Then the simulation takes less than 5 seconds
    And the simulation succeeds
    And the expected overall cost is 92.700
    And the solution is
      | variable    | value |
      | G_p_max_0_0 | 2.900 |