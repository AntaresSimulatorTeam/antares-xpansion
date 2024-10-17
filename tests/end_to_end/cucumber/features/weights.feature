Feature: add weights on MC years

  @slow @short @full-launch
  Scenario: handling different weights on MC years in API mode
    # For now, only check that the simulation succeeds
    # TODO : add more non-regression tests when we have more steps
    Given the study path is "examples/SmallTestFiveCandidatesWithWeights"
    When I run antares-xpansion in memory with the benders method and 1 proc(s)
    Then the simulation succeeds
