Feature: Testing low memory + micro iterations case 

@fast @short @low_memory 
Scenario: micro_iteration_mem_optim_study 
    Given the study path is "data_test/micro_it_mem_optim_study" 
    When I run benders with 1 proc(s)
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
    

