Feature: Multi-year investment problem
	# TODO : add even simpler tests where the solution can be understood and computed by hand
	# This test is not included in the CI because it requires an Xpress license to run.

	@slow @short @full-launch @noci
	Scenario: Solve Simple tree multi-year study
		Given the study path is "data_test/trajectory/simple_tree"
		When I run antares-xpansion in trajectory
		Then the simulation succeeds
		And the expected overall cost is 3255790462.8835249
		And the expected investment cost is 1355866205.1483021
		And the solution is
			| variable                | value  |
			| node_2030__semibase     | 1250.0 |
			| node_2040__peak         | 1000.0 |
			| node_2040__semibase     | 2250.0 |
			| node_2050_A__peak       | 1000.0 |
			| node_2050_A__semibase   | 2908.345483539013 |
			| node_2050_B__peak       | 2500.0 |
