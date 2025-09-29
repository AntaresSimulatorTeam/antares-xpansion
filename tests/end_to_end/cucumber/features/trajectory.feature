Feature: Multi-year investment problem
	# TODO : add even simpler tests where the solution can be understood and computed by hand

	@slow @short @full-launch
	Scenario: Solve Simple tree multi-year study
		Given the study path is "data_test/trajectory/simple_tree"
		When I run antares-xpansion in trajectory #TODO: Rename this step to be convenient with regards to other "When" steps"
		Then the simulation succeeds
		And the expected overall cost is 2355947546.4238625
		And the expected investment cost is 337038731.96844411
		And the solution is #Warning : The trajectory output structure may be different from a classical Xpansion study, for now written in `output/out_benders.json`
			| variable                | value  |
			| node_2030__semibase     | 1250.0 |
			| node_2040__peak         | 1500.0 |
			| node_2040__semibase     | 1750.0 |
			| node_2050_A__peak       | 3000.0 |
			| node_2050_A__semibase   | 2250.0 |
			| node_2050_B__peak       | 3000.0 |
