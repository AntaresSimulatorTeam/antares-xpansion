Feature: Benders decomposition with Micro iterations and Memory optimization mode

	@short @full-launch
	Scenario: Benders with micro iterations and memory optimization with 1 proc
		Given the study path is "data_test/benders_micro_iterations_memory_optim"
		When I run benders with 1 proc(s)
		Then the simulation succeeds
		And the expected overall cost is 0.015971938123833442
		And the expected investment cost is 0.0
			| variable          | value |
			| z[LINE-107-108_0] | 0.0 |
			| z[LINE-115-116_0] | 0.0 |
			| z[LINE-116-117_0] | 0.0 |
			| z[LINE-128-203_0] | 0.0 |
			| z[LINE-216-219_0] | 0.0 |


	@short @full-launch
	Scenario: Benders with micro iterations and memory optimization with multiple procs
		Given the study path is "data_test/benders_micro_iterations_memory_optim"
		When I run benders with 3 proc(s)
		Then the simulation succeeds
		And the expected overall cost is 0.015971938123833442
		And the expected investment cost is 0.0
		| variable          | value |
		| z[LINE-107-108_0] | 0.0 |
		| z[LINE-115-116_0] | 0.0 |
		| z[LINE-116-117_0] | 0.0 |
		| z[LINE-128-203_0] | 0.0 |
		| z[LINE-216-219_0] | 0.0 |
