# Merging the master problems of annual Xpansion studies

## Usage of the master merger executable

**All examples below are based on the tree example given in the [trajectory problem](./index.md) section**

The underlying C++ code responsible for merging previously generated Xpansion studies master files needs

- An [option file](#options-file) to give the general parameters
- A [master merger info file](#master-merger-info-file) that links together the different annual master problems.
- A [lp info file](#nodal-lp-info-file) that tells the program where to find the ```lp``` folders and files of each
  node.
- [Access](#input-files-from-each-annual-study) to the ```structure.txt``` and ```master.mps``` files previously
  generated using ```antares-xpansion-launcher -i <study> --step problem_generation``` (or rather using
  the [Multiple Problem Generation](./multiple-problem-generation.md) step).

Usage is :
```path/to/exectubale <options_file>.json <master_merger_info_file>.json```

**Note** : the C++ executable has to be launched at the ```--dataDir```. (The python driver takes care of this, this
should not be a concern to most users).

## Options file
Only few of the ```BaseOptions``` from the ```<options_file>.json``` are used. An option file for the master merger can
be :

```json
{
    "OUTPUTROOT": "<path/to/ouput/folder>",
    "INPUTROOT": "<path/to/input/root>",
    "JSON_FILE": "<path/to/json/ouput/file>",
    "SOLVER_NAME": "XPRESS",
    "PROBLEMS_FORMAT": "MPS",
    "MASTER_NAME" : "name",
    "STRUCTURE_FILE" : "<relative/path/to/structure/file>"
}
```

This means that the merged master file will be written to : ```<INPUT_ROOT>/<MASTER_NAME>.<mps|svf> ```, and the merged
structure file to : ```<INPUT_ROOT>/<STRUCTURE_FILE>```.

## Master merger info file

The program expects ```<master_merger_info>.json``` to have the following structure :

**master_merger_info.json** : Example of a master structure file :

```json
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
            "rhs" : 1000,
            "operator" : "<"
        },
        {
            "coeffs" : {
                "2040::semibase::x" : 1,
                "2030::peak::dx_minus" : 1
            },
            "rhs" : 0,
            "operator" : "="
        }
    ],
    "tree" : {
        "2030" : {
            "parent" : "root",
            "node_weight" : 10.0,
            "candidates_costs" : {
                "semibase" : {
                    "investment" : 5000,
                    "operation_maintenance" : 1000,
                    "retirement" : 0
                },
                "peak" : {
                    "investment" : 3500,
                    "operation_maintenance" : 1000,
                    "retirement" : 0
                }
            }
        },
        "2040" : {
            "parent" : "2030",
            "node_weight" : 10.0,
            "candidates_costs" : {
                "semibase" : {
                    "investment" : 5000,
                    "operation_maintenance" : 1000,
                    "retirement" : 0
                },
                "peak" : {
                    "investment" : 3500,
                    "operation_maintenance" : 1000,
                    "retirement" : 0
                }
            }
        },
        "2050_A" : {
            "parent" : "2040",
            "node_weight" : 8.0,
            "candidates_costs" : {
                "semibase" : {
                    "investment" : 4000,
                    "operation_maintenance" : 800,
                    "retirement" : 0
                },
                "peak" : {
                    "investment" : 2800,
                    "operation_maintenance" : 8000,
                    "retirement" : 0
                }
            }

        },
        "2050_B" : {
            "parent" : "2040",
            "node_weight" : 2.0,
            "candidates_costs" : {
                "semibase" : {
                    "investment" : 1000,
                    "operation_maintenance" : 200,
                    "retirement" : 0
                },
                "peak" : {
                    "investment" : 700,
                    "operation_maintenance" : 200,
                    "retirement" : 0
                }
            }

        }
    }
}
```

We give a short description of the data expected in each field :

- ```initial_capacities``` contains, for each candidate, the capacity installed before the first investment time point.
- ```constraints``` contains user-given trajectory constraints on the different investment variables. See
  the [Trajectory constraints section](#trajectory-constraints) for more details.
- ```tree``` contains the trajectory tree itself, and the data pertaining to each of its nodes. Each of the nodes
  contains the following data:
    - ```lp_folder``` points to a folder containing both the ```master_file``` and the ```structure_file``` of this
      annual study.
    - ```parent``` is the name of the node's parent in the tree.
    - ```node_weight``` is the node's weight $w(n)$ in the objective function. Note that in this example, we used a
      discounting rate of $0$ to get an aggregated weight of $10$ on each investment time point.
    - ```candidates_costs``` contains the coefficients of the corresponding variables in the final objective. Each of
      those costs already incorporates the probability of the node appearing, the node's represented duration and the
      discounting rate. The coefficient was pre-computed when parsing user input data.

## Nodal lp info file
To access the ```lp``` data of each node, the executable expects to be passed
the [nodal lp info file](./multiple-problem-generation.md#output-nodal-lp-info-file) that references the location &
filenames of the ouput of the ```problem_generation``` step for each node.

```json
{
   "2030" : 
   {
      "lp_folder" : "./node_2030_study/output/20250526-1505eco/lp/",
      "master_file" : "master",
      "structure_file" : "structure.txt",
      "weights_file" : "weights.txt"
   },
   "2040" : 
   {
      "lp_folder" : "./node_2040_study/output/20250526-1505eco/lp/",
      "master_file" : "master",
      "structure_file" : "structure.txt",
   },
   "2050_A" : 
   {
      "lp_folder" : "./node_2050_A_study/output/20250526-1505eco/lp/",
      "master_file" : "master",
      "structure_file" : "structure.txt",
   },
   "2050_B" : 
   {
      "lp_folder" : "./node_2050_B_study/output/20250526-1505eco/lp/",
      "master_file" : "master",
      "structure_file" : "structure.txt",
   }
}
```

## Input files from each annual study

We give below what the folder given as ```INPUTROOT``` in the options file should look like in the present example :

```
.
├── intermediary_files
│   ├── master_merger_info.json
│   ├── options_merge_master.json
│   └── nodal_lp_info.json
├── node_2030_study
│   ├── Desktop.ini
│   ├── input ...
│   ├── layers ...
│   ├── logs ...
│   ├── output
│   │   └── 20250526-1505eco
│   │       ├── <various_files>
│   │       ├── about-the-study ...
│   │       ├── economy ...
│   │       ├── lp
│   │       │   ├── master.mps
│   │       │   ├── problem-1-1--optim-nb-1.mps
│   │       │   ├── problem-1-2--optim-nb-1.mps
│   │       │   ├── problem-2-1--optim-nb-1.mps
│   │       │   ├── problem-2-2--optim-nb-1.mps
│   │       │   ├── ProblemGenerationLog.txt
│   │       │   ├── structure.txt
│   │       │   └── weights.txt
│   │       ├── optimization ...
│   │       └── ts-numbers ...
│   ├── settings ...
│   ├── study.antares
│   └── user
│       └── expansion
│           ├── candidates.ini
│           ├── capa ...
│           ├── constraints ...
│           ├── outer_loop ...
│           ├── sensitivity ...
│           ├── settings.ini
│           └── weights
│               └── weights.txt
├── node_2040_study
│   ├── Desktop.ini
│   ├── input ...
│   ├── layers ...
│   ├── logs ...
│   ├── output
│   │   └── 20250526-1505eco
│   │       ├── <various_files>
│   │       ├── about-the-study ...
│   │       ├── economy ...
│   │       ├── lp
│   │       │   ├── master.mps
│   │       │   ├── problem-1-1--optim-nb-1.mps
│   │       │   ├── problem-1-2--optim-nb-1.mps
│   │       │   ├── problem-2-1--optim-nb-1.mps
│   │       │   ├── problem-2-2--optim-nb-1.mps
│   │       │   ├── ProblemGenerationLog.txt
│   │       │   ├── structure.txt
│   │       │   └── weights.txt
│   │       ├── optimization ...
│   │       └── ts-numbers ...
│   ├── settings ...
│   ├── study.antares
│   └── user
│       └── expansion
│           ├── candidates.ini
│           ├── capa ...
│           ├── constraints ...
│           ├── outer_loop ...
│           ├── sensitivity ...
│           ├── settings.ini
│           └── weights
│               └── weights.txt
├── node_2050_A_study
│   ├── Desktop.ini
│   ├── input ...
│   ├── layers ...
│   ├── logs ...
│   ├── output
│   │   └── 20250526-1505eco
│   │       ├── <various_files>
│   │       ├── about-the-study ...
│   │       ├── economy ...
│   │       ├── lp
│   │       │   ├── master.mps
│   │       │   ├── problem-1-1--optim-nb-1.mps
│   │       │   ├── problem-1-2--optim-nb-1.mps
│   │       │   ├── problem-2-1--optim-nb-1.mps
│   │       │   ├── problem-2-2--optim-nb-1.mps
│   │       │   ├── ProblemGenerationLog.txt
│   │       │   ├── structure.txt
│   │       │   └── weights.txt
│   │       ├── optimization ...
│   │       └── ts-numbers ...
│   ├── settings ...
│   ├── study.antares
│   └── user
│       └── expansion
│           ├── candidates.ini
│           ├── capa ...
│           ├── constraints ...
│           ├── outer_loop ...
│           ├── sensitivity ...
│           ├── settings.ini
│           └── weights
│               └── weights.txt
└── node_2050_B_study
    ├── Desktop.ini
    ├── input ...
    ├── layers ...
    ├── logs ...
    ├── output
    │   └── 20250526-1505eco
    │       ├── <various_files>
    │       ├── about-the-study ...
    │       ├── economy ...
    │       ├── lp
    │       │   ├── master.mps
    │       │   ├── problem-1-1--optim-nb-1.mps
    │       │   ├── problem-1-2--optim-nb-1.mps
    │       │   ├── problem-2-1--optim-nb-1.mps
    │       │   ├── problem-2-2--optim-nb-1.mps
    │       │   ├── ProblemGenerationLog.txt
    │       │   ├── structure.txt
    │       │   └── weights.txt
    │       ├── optimization ...
    │       └── ts-numbers ...
    ├── settings ...
    ├── study.antares
    └── user
        └── expansion
            ├── candidates.ini
            ├── capa ...
            ├── constraints ...
            ├── outer_loop ...
            ├── sensitivity ...
            ├── settings.ini
            └── weights
                └── weights.txt
```

Each of the subfolder is the output of ```antares -i <study> --step problem_generation```.
For example, the structure file of a given node should be found at :
```<INPUTROOT>/<lp_folder>/<structure_file>```

Note that the subfolders as presented in this example do not have to be inside the the full study structure and could
have been generated and copied from elsewhere in the ```<INPUTROOT>``` folder.

## Trajectory constraints

In the ```constraints``` section of the ```master_merger_info.json``` file, we expect the constraints to be given in the
following manner:

- ```coeffs``` is a dict of investment variable reference to their coefficient in the present constraint. Each reference
  is built as : ```<node_name>::<candidate_name>::<variable_type>```, where variable type is either :
    - ```x``` when referencing the $x_{n,i}$ variable.
    - ```dx_plus``` when referencing the $dx_{n,i}^+$ variable.
    - ```dx_minus``` when referencing the $dx_{n,i}^⁻$ variable.
- ```rhs``` contains the right-hand side of the constraint expression.
- ```operator``` defines the type of constraint we want to set and expects one of three values :
    - ```<``` for constraints of type : $expression \leq rhs$
    - ```=``` for constraints of type : $expression = rhs$
    - ```>``` for constraints of type : $expression \geq rhs$
