Feature: add weights on MC years

  @slow @short @full-launch
  Scenario: handling different weights on MC years in API mode
    # For now, only check that the simulation succeeds
    # TODO : add more non-regression tests when we have more steps
    Given the study path is "examples/SmallTestFiveCandidatesWithWeights"
    When I run antares-xpansion in memory with the benders method and 1 proc(s)
    Then the simulation succeeds
    And the expected overall cost is 24232177891.450203
    And the expected investment cost is 230600000.0
    And the solution is
      | variable          | value     |
      | battery           | 1000.0    |
      | peak              | 1500.0000 |
      | pv                | 1000.0000 |
      | semibase          | 200.0     |
      | transmission_line | 0.0       |
