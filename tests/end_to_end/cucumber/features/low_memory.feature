Feature: Low memory tests

  @fast @short @low_memory
  Scenario: low_memory_study
    Given the study path is "data_test/low_memory_study"
    When I run benders with 1 proc(s)
    Then the simulation succeeds

  @fast @short @low_memory
  Scenario: low_memory_study_3_procs
    Given the study path is "data_test/low_memory_study"
    When I run benders with 3 proc(s)
    Then the simulation succeeds
