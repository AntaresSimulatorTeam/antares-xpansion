# Merging the master problems of annual Xpansion studies

## MergeMasterMPS structure file
### Note : Perhaps move to developer guide, as this should not be relevant to the end user.

**All examples below are based on the tree example given in the [trajectory problem](./trajectory-problem.md) section**

The underlying C++ code responsible for merging previously generated Xpansion studies master files needs an input  structure file that links together the different annual master problems. The programm expects a ```json``` file with the following structure :

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
        "2050_A" : {
            "lp_folder" : "node_2030_A__lp",
            "master_mps_file" : "node_2030_A__master.mps",
            "structure_file" : "node_2030_A__structure.txt",
            "parent" : "2040",
            "weight_factor" : 7.0,
            "candidates" : {
                "semibase" : "semibase_type",
                "peak" : "peak_type"
            }
            
        },
        "2050_B" : {
            "lp_folder" : "node_2030_B__lp",
            "master_mps_file" : "node_2030_B__master.mps",
            "structure_file" : "node_2030_B__structure.txt",
            "parent" : "2040",
            "weight_factor" : 3.0,
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

## MergeMasterMPS Input files from each annual study
We give below what the folder given as ```INPUTROOT``` in the options file should look like :
```
├── master_structure.json
├── node_2030__lp
│   ├── node_2030__master.mps
│   ├── node_2030__problem-1-1--optim-nb-1.mps
│   ├── node_2030__problem-1-2--optim-nb-1.mps
│   ├── node_2030__problem-2-1--optim-nb-1.mps
│   ├── node_2030__problem-2-2--optim-nb-1.mps
│   └── node_2030__structure.txt
├── node_2040__lp
│   ├── node_2040__master.mps
│   ├── node_2040__problem-1-1--optim-nb-1.mps
│   ├── node_2040__problem-1-2--optim-nb-1.mps
│   ├── node_2040__problem-2-1--optim-nb-1.mps
│   ├── node_2040__problem-2-2--optim-nb-1.mps
│   └── node_2040__structure.txt
├── node_2050_A__lp
│   ├── node_2050_A__master.mps
│   ├── node_2050_A__problem-1-1--optim-nb-1.mps
│   ├── node_2050_A__problem-1-2--optim-nb-1.mps
│   ├── node_2050_A__problem-2-1--optim-nb-1.mps
│   ├── node_2050_A__problem-2-2--optim-nb-1.mps
│   ├── node_2050_A__ProblemGenerationLog.txt
│   └── node_2050_A__structure.txt
├── node_2050_B__lp
│   ├── node_2050_B__master.mps
│   ├── node_2050_B__problem-1-1--optim-nb-1.mps
│   ├── node_2050_B__problem-1-2--optim-nb-1.mps
│   ├── node_2050_B__problem-2-1--optim-nb-1.mps
│   ├── node_2050_B__problem-2-2--optim-nb-1.mps
│   └── node_2050_B__structure.txt
```

For example, the structure file of a given node should be found at : 
```<INPUTROOT>/<lp_folder>/<structure_file>```

## Trajectory constraints

In the ```constraints``` section of the ```master_structure.json``` file, we expect the constraints to be given in the following manner:

- ```coeffs``` is a dict of investment variable reference to their coefficient in the present constraint. Each reference is built as : ```<node_name>::<candidate_name>::<variable_type>```, where variable type is either : 
    - ```x``` when referencing the $x_{n,i}$ variable.
    - ```dx_plus``` when referencing the $dx_{n,i}^+$ variable.
    - ```dx_minus``` when referencing the $dx_{n,i}^⁻$ variable.
- ```rhs``` contains the right-hand side of the constraint expression. (Note that all constraints are formulated as a $\leq$ constraint for now)

