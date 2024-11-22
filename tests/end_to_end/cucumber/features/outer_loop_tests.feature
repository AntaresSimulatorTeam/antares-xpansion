Feature: outer loop tests

  @fast @short @outerloop
  Scenario: a system with 4 nodes, on 1 timestep, 2 scenarios
    Given the study path is "data_test/external_loop_test"
    When I run outer loop with 1 proc(s)
    Then the simulation succeeds
    And the simulation takes less than 5 seconds
    And the expected overall cost is 92.70005
    And the solution is
      | variable    | value    |
      | G_p_max_0_0 | 2.900004 |

  @fast @short @outerloop
  Scenario: a non outer loop study e.g with non-consistent adequacy_criterion file and with un-formatted mps (unnamed mps)
    Given the study path is "data_test/mini_instance_MIP"
    When I run outer loop with 1 proc(s) and "options_default.json" as option file
    Then the simulation takes less than 5 seconds
    And the simulation succeeds
    And LOLD.txt and PositiveUnsuppliedEnergy.txt files are full of zeros
