Feature: NB_CUTS_PER_ITER in options.json file sets the number cuts to add to the master problem at every iteration. We test that we converge to the right overall cost and with the correct solution
	
	@short @full-launch
	Scenario: Classical Benders with 1 proc
		Given the study path is "data_test/test_cut_aggregation_ieee96_10inc"
		When I run benders with 1 proc(s)
		Then the simulation succeeds
		And the expected overall cost is 5731.9596696788394
		And the expected investment cost is 5707.7625570776254
		And the solution is
			| variable          | value |
			| z[LINE-107-108_0] | 0.0 |
			| z[LINE-108-203]   | 0.0 |
			| z[LINE-115-116_0] | 0.0 |
			| z[LINE-116-117_0] | 0.0 |
			| z[LINE-116-118]   | 0.0 |
			| z[LINE-116-121]   | 0.0 |
			| z[LINE-128-203_0] | 0.0 |
			| z[LINE-216-218]   | 0.0 |
			| z[LINE-216-219_0] | 0.0 |
			| z[LINE-217-219]   | 0.0 |
			| z[LINE-316-318]   | 0.0 |
			| z[LINE-330-213]   | 0.0 |
			| z[LINE-330-315]   | 0.0 |
			| z[LINE-330-319]   | 1.0 |
    
    @short @full-launch
	Scenario: Benders MPI with multiple procs
		Given the study path is "data_test/test_cut_aggregation_ieee96_10inc"
		When I run benders with 5 proc(s)
		Then the simulation succeeds
		And the expected overall cost is 5731.9596696788394
		And the expected investment cost is 5707.7625570776254
		And the solution is
			| variable          | value |
			| z[LINE-107-108_0] | 0.0 |
			| z[LINE-108-203]   | 0.0 |
			| z[LINE-115-116_0] | 0.0 |
			| z[LINE-116-117_0] | 0.0 |
			| z[LINE-116-118]   | 0.0 |
			| z[LINE-116-121]   | 0.0 |
			| z[LINE-128-203_0] | 0.0 |
			| z[LINE-216-218]   | 0.0 |
			| z[LINE-216-219_0] | 0.0 |
			| z[LINE-217-219]   | 0.0 |
			| z[LINE-316-318]   | 0.0 |
			| z[LINE-330-213]   | 0.0 |
			| z[LINE-330-315]   | 0.0 |
			| z[LINE-330-319]   | 1.0 |

    @short @full-launch
	Scenario: Benders by batch single proc
		Given the study path is "data_test/test_cut_aggregation_ieee96_10inc"
        And the batch size is 5
		When I run benders with 1 proc(s)
		Then the simulation succeeds
		And the expected overall cost is 5731.9596696788394
		And the expected investment cost is 5707.7625570776254
		And the solution is
			| variable          | value |
			| z[LINE-107-108_0] | 0.0 |
			| z[LINE-108-203]   | 0.0 |
			| z[LINE-115-116_0] | 0.0 |
			| z[LINE-116-117_0] | 0.0 |
			| z[LINE-116-118]   | 0.0 |
			| z[LINE-116-121]   | 0.0 |
			| z[LINE-128-203_0] | 0.0 |
			| z[LINE-216-218]   | 0.0 |
			| z[LINE-216-219_0] | 0.0 |
			| z[LINE-217-219]   | 0.0 |
			| z[LINE-316-318]   | 0.0 |
			| z[LINE-330-213]   | 0.0 |
			| z[LINE-330-315]   | 0.0 |
			| z[LINE-330-319]   | 1.0 |

    @short @full-launch
	Scenario: Benders by batch multiple proc
		Given the study path is "data_test/test_cut_aggregation_ieee96_10inc"
        And the batch size is 5
		When I run benders with 3 proc(s)
		Then the simulation succeeds
		And the expected overall cost is 5731.9596696788394
		And the expected investment cost is 5707.7625570776254
		And the solution is
			| variable          | value |
			| z[LINE-107-108_0] | 0.0 |
			| z[LINE-108-203]   | 0.0 |
			| z[LINE-115-116_0] | 0.0 |
			| z[LINE-116-117_0] | 0.0 |
			| z[LINE-116-118]   | 0.0 |
			| z[LINE-116-121]   | 0.0 |
			| z[LINE-128-203_0] | 0.0 |
			| z[LINE-216-218]   | 0.0 |
			| z[LINE-216-219_0] | 0.0 |
			| z[LINE-217-219]   | 0.0 |
			| z[LINE-316-318]   | 0.0 |
			| z[LINE-330-213]   | 0.0 |
			| z[LINE-330-315]   | 0.0 |
			| z[LINE-330-319]   | 1.0 |

	@short @full-launch
	Scenario: Classical Benders with 1 proc with cache problems
		Given the study path is "data_test/test_cut_aggregation_ieee96_10inc"
		And we have batch problems
		When I run benders with 1 proc(s)
		Then the simulation succeeds
		And the expected overall cost is 5731.9596696788394
		And the expected investment cost is 5707.7625570776254
		And the solution is
			| variable          | value |
			| z[LINE-107-108_0] | 0.0 |
			| z[LINE-108-203]   | 0.0 |
			| z[LINE-115-116_0] | 0.0 |
			| z[LINE-116-117_0] | 0.0 |
			| z[LINE-116-118]   | 0.0 |
			| z[LINE-116-121]   | 0.0 |
			| z[LINE-128-203_0] | 0.0 |
			| z[LINE-216-218]   | 0.0 |
			| z[LINE-216-219_0] | 0.0 |
			| z[LINE-217-219]   | 0.0 |
			| z[LINE-316-318]   | 0.0 |
			| z[LINE-330-213]   | 0.0 |
			| z[LINE-330-315]   | 0.0 |
			| z[LINE-330-319]   | 1.0 |


    @short @full-launch
	Scenario: Benders MPI with multiple procs
		Given the study path is "data_test/test_cut_aggregation_ieee96_10inc"
		And we have batch problems
		When I run benders with 5 proc(s)
		Then the simulation succeeds
		And the expected overall cost is 5731.9596696788394
		And the expected investment cost is 5707.7625570776254
		And the solution is
			| variable          | value |
			| z[LINE-107-108_0] | 0.0 |
			| z[LINE-108-203]   | 0.0 |
			| z[LINE-115-116_0] | 0.0 |
			| z[LINE-116-117_0] | 0.0 |
			| z[LINE-116-118]   | 0.0 |
			| z[LINE-116-121]   | 0.0 |
			| z[LINE-128-203_0] | 0.0 |
			| z[LINE-216-218]   | 0.0 |
			| z[LINE-216-219_0] | 0.0 |
			| z[LINE-217-219]   | 0.0 |
			| z[LINE-316-318]   | 0.0 |
			| z[LINE-330-213]   | 0.0 |
			| z[LINE-330-315]   | 0.0 |
			| z[LINE-330-319]   | 1.0 |


    @short @full-launch
	Scenario: Benders by batch single proc
		Given the study path is "data_test/test_cut_aggregation_ieee96_10inc"
		And we have batch problems
        And the batch size is 5
		When I run benders with 1 proc(s)
		Then the simulation succeeds
		And the expected overall cost is 5731.9596696788394
		And the expected investment cost is 5707.7625570776254
		And the solution is
			| variable          | value |
			| z[LINE-107-108_0] | 0.0 |
			| z[LINE-108-203]   | 0.0 |
			| z[LINE-115-116_0] | 0.0 |
			| z[LINE-116-117_0] | 0.0 |
			| z[LINE-116-118]   | 0.0 |
			| z[LINE-116-121]   | 0.0 |
			| z[LINE-128-203_0] | 0.0 |
			| z[LINE-216-218]   | 0.0 |
			| z[LINE-216-219_0] | 0.0 |
			| z[LINE-217-219]   | 0.0 |
			| z[LINE-316-318]   | 0.0 |
			| z[LINE-330-213]   | 0.0 |
			| z[LINE-330-315]   | 0.0 |
			| z[LINE-330-319]   | 1.0 |


    @short @full-launch
	Scenario: Benders by batch multiple proc
		Given the study path is "data_test/test_cut_aggregation_ieee96_10inc"
		And we have batch problems
        And the batch size is 5
		When I run benders with 3 proc(s)
		Then the simulation succeeds
		And the expected overall cost is 5731.9596696788394
		And the expected investment cost is 5707.7625570776254
		And the solution is
			| variable          | value |
			| z[LINE-107-108_0] | 0.0 |
			| z[LINE-108-203]   | 0.0 |
			| z[LINE-115-116_0] | 0.0 |
			| z[LINE-116-117_0] | 0.0 |
			| z[LINE-116-118]   | 0.0 |
			| z[LINE-116-121]   | 0.0 |
			| z[LINE-128-203_0] | 0.0 |
			| z[LINE-216-218]   | 0.0 |
			| z[LINE-216-219_0] | 0.0 |
			| z[LINE-217-219]   | 0.0 |
			| z[LINE-316-318]   | 0.0 |
			| z[LINE-330-213]   | 0.0 |
			| z[LINE-330-315]   | 0.0 |
			| z[LINE-330-319]   | 1.0 |