# Merging the master problems of annual Xpansion studies


## Note : Perhaps move to developer guide, as this should not be relevant to the end user.

## Usage of the master merger executable

**All examples below are based on the tree example given in the [trajectory problem](./trajectory-problem.md) section**

The underlying C++ code responsible for merging previously generated Xpansion studies master files needs 

- An [option file](#options-file) to give the general parameters
- A [master structure file](#master-structure-file) that links together the different annual master problems.
- [Access](#input-files-from-each-annual-study) to the ```structure.txt``̀  and ```master.mps``` files previously generated using ```antares -i <study> --step problem_generation```.

Usage is :
```path/to/exectubale <options_file>.json <master_structure_file>.json```


## Options file
Only few of the ```BendersOptions``` from the ```<options_file>.json``` are truly necessary. An option file for the master merger can be : 

```json
{
    "OUTPUTROOT": "<path/to/ouput/folder>",
    "INPUTROOT": "<path/to/input/root>",
    "JSON_FILE": "<path/to/metadata/json/ouput/file>",
    "SOLVER_NAME": "Xpress",
    "PROBLEMS_FORMAT": "MPS"
}
```

## Master structure file
The programm expects a ```<master_structure_file>.json``` to have the following structure :

**master_structure.json** : Example of a master structure file :

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
            "weight_factor" : 8.0,
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
            "weight_factor" : 2.0,
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
- ```constraints``` contains user-given trajectory constraints on the different investment variables. See the [Trajectory constraints section](#trajectory-constraints) for more details.
- ```candidates_types``` contains investment, operation and maintenance and retirement costs for a given candidate type. This allows sharing cost properties among similar candidates in different zones.
- ```tree``` contains the trajectory tree itself, and the data pertaining to each of its nodes. Each of the nodes contains the following data:
    - ```lp_folder``` points to a folder containing both the ```master_mps_file``` and the ```structure_file``` of this annual study.
    - ```parent``` is the name of the node's parent in the tree.
    - ```weight_factor``` is the node's weight $w(n)$ in the objective function.
    - ```candidates``` points each of the node's candidate to its type and thus its costs.

## Input files from each annual study
We give below what the folder given as ```INPUTROOT``` in the options file should look like in the present example :
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
Each of the subfolder is the ouput of of ```antares -i <study> --step problem_generation```.  
For example, the structure file of a given node should be found at : 
```<INPUTROOT>/<lp_folder>/<structure_file>```

Note that the subfolders as presented in this example do not have to be immediately contained in the ```<INPUTROOT>``` folder, and could be kept in the ```output``` folders of each annual study (and thus ````<INPUTROOT>``` would then contain each of the full Antares studies, and each ```lp_folder``` would point to : ```<study_folder>/ouput/<date>-Xpansion/lp```)


## Trajectory constraints

In the ```constraints``` section of the ```master_structure.json``` file, we expect the constraints to be given in the following manner:

- ```coeffs``` is a dict of investment variable reference to their coefficient in the present constraint. Each reference is built as : ```<node_name>::<candidate_name>::<variable_type>```, where variable type is either : 
    - ```x``` when referencing the $x_{n,i}$ variable.
    - ```dx_plus``` when referencing the $dx_{n,i}^+$ variable.
    - ```dx_minus``` when referencing the $dx_{n,i}^⁻$ variable.
- ```rhs``` contains the right-hand side of the constraint expression. (Note that all constraints are formulated as a $\leq$ constraint for now)
- ```operator``` defines the type of constraint we want to set and expects one of three values :
    - ```<``` for constraints of type : $expression \leq rhs$
    - ```=``` for constraints of type : $expression = rhs$
    - ```>``` for constraints of type : $expression \geq rhs$

