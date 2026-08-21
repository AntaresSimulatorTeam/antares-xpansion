Feature: Benders converges identically across cache levels, batching, and micro-iterations

@fast @short @low_memory
Scenario Outline: Benders solves the ieee96 study to the same solution
    Given the study path is "<study_path>"
    And the cache problems level is <cache_level>
    And the batch size is <batch_size>
    When I run benders with <procs> proc(s)
    Then the simulation succeeds
    And the expected overall cost is 1.630789302019473
    And the expected investment cost is 1.6307893020221784
    And the solution is
      | variable          | value |
      | z[LINE-107-108_0] | 0.0   |
      | z[LINE-115-116_0] | 0.0   |
      | z[LINE-116-117_0] | 0.0   |
      | z[LINE-128-203_0] | 0.0   |
      | z[LINE-216-219_0] | 0.0   |
      | z[LINE-330-213]   | 0.0   |
      | z[LINE-330-319]   | 1.0   |

    Examples:
      | study_path                                | cache_level | batch_size | procs |
      | data_test/ieee96_base                      | 0           | 0          | 1     |
      | data_test/ieee96_base                      | 0           | 1          | 3     |
      | data_test/ieee96_base                      | 1           | 0          | 1     |
      | data_test/ieee96_base                      | 1           | 1          | 3     |
      | data_test/ieee96_skeleton                   | 2           | 0          | 1     |
      | data_test/ieee96_skeleton                   | 2           | 1          | 3     |

@fast @short @low_memory
Scenario Outline: Benders solves the ieee96 study to the same solution with micro-iterations, regardless of warm start
    Given the study path is "<study_path>"
    And the cache problems level is <cache_level>
    And the batch size is <batch_size>
    And the warm start is <warm_start>
    When I run benders with <procs> proc(s)
    Then the simulation succeeds
    And the expected overall cost is 1.630789302019473
    And the expected investment cost is 1.6307893020221784
    And the solution is
      | variable          | value |
      | z[LINE-107-108_0] | 0.0   |
      | z[LINE-115-116_0] | 0.0   |
      | z[LINE-116-117_0] | 0.0   |
      | z[LINE-128-203_0] | 0.0   |
      | z[LINE-216-219_0] | 0.0   |
      | z[LINE-330-213]   | 0.0   |
      | z[LINE-330-319]   | 1.0   |

    Examples:
      | study_path                                | cache_level | batch_size | procs | warm_start |
      | data_test/ieee96_micro_it                  | 0           | 0          | 1     | 0          |
      | data_test/ieee96_micro_it                  | 0           | 0          | 1     | 1          |
      | data_test/ieee96_micro_it                  | 0           | 1          | 3     | 0          |
      | data_test/ieee96_micro_it                  | 0           | 1          | 3     | 1          |
      | data_test/ieee96_micro_it                  | 1           | 0          | 1     | 0          |
      | data_test/ieee96_micro_it                  | 1           | 0          | 1     | 1          |
      | data_test/ieee96_micro_it                  | 1           | 1          | 3     | 0          |
      | data_test/ieee96_micro_it                  | 1           | 1          | 3     | 1          |
      | data_test/ieee96_micro_it_skeleton_study   | 2           | 0          | 1     | 0          |
      | data_test/ieee96_micro_it_skeleton_study   | 2           | 0          | 1     | 1          |
      | data_test/ieee96_micro_it_skeleton_study   | 2           | 1          | 3     | 0          |
      | data_test/ieee96_micro_it_skeleton_study   | 2           | 1          | 3     | 1          |
