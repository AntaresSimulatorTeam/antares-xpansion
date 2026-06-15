Feature: Low memory tests

  @fast @short @low_memory
  Scenario: low_memory_study
    Given the study path is "data_test/low_memory_study"
    When I run benders with 1 proc(s)
    Then the simulation succeeds
    And the expected overall cost is 1.8508502774365074
    And the expected investment cost is 1.7938682322243966
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

  @fast @short @low_memory
  Scenario: low_memory_study_3_procs
    Given the study path is "data_test/low_memory_study"
    When I run benders with 3 proc(s)
    Then the simulation succeeds
    And the expected overall cost is 1.8508502774365074
    And the expected investment cost is 1.7938682322243966
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
