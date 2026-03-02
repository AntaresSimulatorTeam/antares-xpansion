Feature: Multi-year investment problem
	# TODO : add even simpler tests where the solution can be understood and computed by hand
	# This test is not included in the CI because it requires an Xpress license to run.

	@slow @short @full-launch @noci
	Scenario: Solve Simple tree multi-year study
		Given the study path is "data_test/trajectory/simple_tree"
		When I run antares-xpansion in trajectory
		Then the simulation succeeds
		And the expected overall cost is 3319916676.4628983
		And the expected investment cost is 1391071819.5959396
		And the solution is
			| variable                | value  |
			| node_2030__semibase     | 1250.0 |
			| node_2040__peak         | 1000.0 |
			| node_2040__semibase     | 2250.0 |
			| node_2050_A__peak       | 1000.0 |
			| node_2050_A__semibase   | 3031.9947402993416 |
			| node_2050_B__peak       | 2500.0 |
