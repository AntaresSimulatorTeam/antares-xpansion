Feature: outer loop tests

  @fast @short @Benders
  Scenario: xpansion-test-01
    Given the study path is "data_test/examples/xpansion-test-01"
    When I run antares-xpansion with the benders method and 1 proc(s)
    Then the simulation takes less than 300 seconds
    And the simulation succeeds
#    And the expected overall cost is 92.70005
#    And the solution is
#      | variable    | value    |
#      | G_p_max_0_0 | 2.900004 |

