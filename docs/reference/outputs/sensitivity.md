# Sensitivity analysis outputs

The sensitivity analysis module creates a JSON file `sensitvity_out.json` that is stored in the directory `output/simulation-name/sensitivity` of the Antares study folder.

A sample output file is given below:
```json
{
	"antares" : 
	{
		"version" : "8.1.0"
	},
	"antares_xpansion" : 
	{
		"version" : "0.6.0"
	},
	"best benders cost" : 178836183.38241878,
	"epsilon" : 10000,
	"sensitivity solutions" : 
	[
		{
			"candidates" : 
			[
				{
					"invest" : 948.01436354994814,
					"name" : "peak"
				},
				{
					"invest" : 0,
					"name" : "semibase"
				}
			],
			"objective" : 56880861.812996887,
			"optimization direction" : "min",
			"problem type" : "capex",
			"status" : 0,
			"system cost" : 178846183.38241878
		},
		{
			"candidates" : 
			[
				{
					"invest" : 1201.3053164764756,
					"name" : "peak"
				},
				{
					"invest" : 400,
					"name" : "semibase"
				}
			],
			"objective" : 108078318.98858854,
			"optimization direction" : "max",
			"problem type" : "capex",
			"status" : 0,
			"system cost" : 178846183.38241878
		},
		{
			"candidates" : 
			[
				{
					"invest" : 584.99719014093932,
					"name" : "peak"
				},
				{
					"invest" : 400,
					"name" : "semibase"
				}
			],
			"objective" : 584.99719014093932,
			"optimization direction" : "min",
			"problem type" : "projection peak",
			"status" : 0,
			"system cost" : 178846183.38241875
		},
		{
			"candidates" : 
			[
				{
					"invest" : 1759.8724009014643,
					"name" : "peak"
				},
				{
					"invest" : 0,
					"name" : "semibase"
				}
			],
			"objective" : 1759.8724009014643,
			"optimization direction" : "max",
			"problem type" : "projection peak",
			"status" : 0,
			"system cost" : 178846183.38241875
		},
	]
}
```

In this example, we have performed the sensitivity analysis with the following input file:
```json
{
    "epsilon" : 1e4,
    "capex": false,
    "projection": ["peak"]
}
```

The output file gathers the following data:

- The version of Antares and Antares Xpansion that is used,
- `best benders cost`: Best upper bound, that is the optimal overall cost, found in the Antares Xpansion optimization that was executed beforehand,
- `epsilon`: Maximum gap with the optimal solution that is allowed,
- `sensitivity solutions`: An array containing data for each sensitivity problem that is solved:

    - `objective`: Value of the objective of the sensitivity problem:
    
        - For a CAPEX minimization (resp. maximization) problem, this the value of the mimimum (resp. maximum) CAPEX that is found over the \\(\varepsilon\\)-optimal solutions. 
        - For the minimization (resp. maximization) projection problem of candidate \\(i\\), this is the minimum (resp. maximum) invested capacity of candidate \\(i\\) over the \\(\varepsilon\\)-optimal solutions.
    
    - `optimization direction`: The direction of the sensitivity problem i.e. minimization or maximization,
    - `problem type`: The type of sensitivity problem that is solved,
    - `status`: Optimization status

        - 0: optimal,
        - 1: infeasible,
        - 2: unbounded.

    - `candidates` : An array describing an \\(\varepsilon\\)-optimal investment combination that satisfies the bound found in the sensitivity problem:
        - For a CAPEX minimization (resp. maximization) problem, this is an \\(\varepsilon\\)-optimal investment combination that minimizes (resp. maximizes) the CAPEX.
        - For the minimization (resp. maximization) projection problem of candidate \\(i\\), this is an \\(\varepsilon\\)-optimal investment combination that minimizes (resp. maximizes) the capacity of candidate \\(i\\).

    - `system cost`: Value of the overall system cost obtained with the investment combination given in `candidates`.

!!! Remarks
    - For the projection problem on candidate \\(i\\), we logically retrieve that the `objective` is equal to the invested capacity in candiate \\(i\\) from the `candidates` section.
    - For all sensitivity problems, we must have `system cost <= best benders cost + epsilon` as this is the constraint that is enforced. As the solutions of a linear program are on the boundary of the domain, it is often the case (but not always) that this constraint is saturated for the sensitivity solutions, so that we have `system cost = best benders cost + epsilon`.
 