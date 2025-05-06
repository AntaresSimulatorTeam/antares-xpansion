# Merging the master problems of annual Xpansion studies

## MergeMasterMPS Input file
### Note : Perhaps move to developer guide, as this should not be relevant to the end user.
The underlying C++ code responsible for merging previously generated Xpansion studies master files needs an input file that with the following structure :

**master_structure.json** : Example of a simple ```MergeMasterTrajectoryMPS``` input file

```js
{
    "initial_capacities" : {
        "default" : 0,
        "semibase" : 250
    },
    "constraints" : [
        {   
            "coeffs" : {
                "2030::semibase::dx_plus" : 1,
                "2030::peak::dx_plus" : 1
            },
            "rhs" : 1000
        },
        {
            "coeffs" : {
                "2040::semibase::x" : 1,
                "2030::peak::dx_minus" : 1
            },
            "rhs" : 0
        }
    ],
    "candidates_types" : {
        "semibase_type" : {
            "investment" : 5000,
            "operation_maintenance" : 100,
            "retirement" : 0
        },
        "peak_type" : {
            "investment" : 3500,
            "operation_maintenance" : 100,
            "retirement" : 0
        }
    },
    "tree" : {
        "2030" : {
            "investment_date" : 2030,
            "lp_folder" : "node_2030__lp",
            "master_mps_file" : "node_2030__master.mps",
            "structure_file" : "node_2030__structure.txt",
            "parent" : "root",
            "weight_factor" : 10.0,
            "candidates" : {
                "semibase" : "semibase_type",
                "peak" : "peak_type"
            }
        },
        "2040" : {
            "investment_date" : 2040,
            "lp_folder" : "node_2040__lp",
            "master_mps_file" : "node_2040__master.mps",
            "structure_file" : "node_2040__structure.txt",
            "parent" : "2030",
            "weight_factor" : 10.0,
            "candidates" : {
                "semibase" : "semibase_type",
                "peak" : "peak_type"
            }
        },
        "2050" : {
            "investment_date" : 2030,
            "lp_folder" : "node_2030__lp",
            "master_mps_file" : "node_2030__master.mps",
            "structure_file" : "node_2030__structure.txt",
            "parent" : "2040",
            "weight_factor" : 10.0,
            "candidates" : {
                "semibase" : "semibase_type",
                "peak" : "peak_type"
            }
            
        }
    }
}
```

We give a short description of the data expected in each field :

- ```initial_capacities``` contains, for each candidate, the capacity installed at before the first investment time point.
- ```constraints``` contains user-given trajectory constraints on the different investment variables. See the [Trajectory constraints section](./trajectory-constraints.md) for more details.
- ```candidates_types``` contains investment, operation and maintenance and retirement costs for a given candidate type. This allows sharing cost properties among similar candidates in different zones.
- ```tree``` contains the trajectory tree itself, and the data pertaining to each of its nodes.
