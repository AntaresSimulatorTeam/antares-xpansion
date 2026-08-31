Feature: NB_CUTS_PER_ITER in options.json file sets the number cuts to add to the master problem at every iteration. We test that we converge to the right overall cost and with the correct solution

@short @full-launch
Scenario Outline: Benders converges to the same solution across batch and cache configurations
    Given the study path is "data_test/test_cut_aggregation_ieee96_10inc"
    And the cache problems level is <cache_level>
    And the batch size is <batch_size>
    When I run benders with <procs> proc(s)
    Then the simulation succeeds
    And the expected overall cost is 5731.9596696788394
    And the expected investment cost is 5707.7625570776254
    And the solution is
      | variable          | value |
      | z[LINE-107-108_0] | 0.0   |
      | z[LINE-108-203]   | 0.0   |
      | z[LINE-115-116_0] | 0.0   |
      | z[LINE-116-117_0] | 0.0   |
      | z[LINE-116-118]   | 0.0   |
      | z[LINE-116-121]   | 0.0   |
      | z[LINE-128-203_0] | 0.0   |
      | z[LINE-216-218]   | 0.0   |
      | z[LINE-216-219_0] | 0.0   |
      | z[LINE-217-219]   | 0.0   |
      | z[LINE-316-318]   | 0.0   |
      | z[LINE-330-213]   | 0.0   |
      | z[LINE-330-315]   | 0.0   |
      | z[LINE-330-319]   | 1.0   |

    Examples:
      | cache_level | batch_size | procs |
      | 0           | 0          | 1     |
      | 0           | 0          | 5     |
      | 0           | 5          | 1     |
      | 0           | 5          | 3     |
      | 1           | 0          | 1     |
      | 1           | 0          | 5     |
      | 1           | 5          | 1     |
      | 1           | 5          | 3     |
