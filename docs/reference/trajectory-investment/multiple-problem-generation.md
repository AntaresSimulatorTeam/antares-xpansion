# Multiple problem generation

The ```MultipleProblemGeneration``` executable will run the problem generation step for every study in the tree.

## Usage

The most simple usage is :  
```<multiple_problem_gen_executable> --study <study_paths_file> --weights <weights_reference_file> --nodal-file <path/to/nodal-lp-info/output/file>```

Note that this executable uses options that extend the simple ```ProblemGenerationExeOptions``` parser used in the
individual problem generation executable. Thus, every other option of the individual problem generation can be given to
the ```MultipleProblemGeneration``` executable and will be duplicated for each individual node's problem generation.

**Note** : the C++ executable has to be launched at the ```--dataDir```. (The python driver takes care of this, this
should not be a concern to most users).

## Study paths / archives file

The study paths file (or archive paths file when called with ```--archive <archive_paths_file>```) tells the executable
where to find the relevant input for each node :

- Study paths when using memory mode (and option ```--study```)
- Previously generated antares output archives otherwise (with option ```--archive```)

The reference file is a simple text file with two columns :
The first column contains the node's name, and the second the path to the relevant input (in this case the path to the
study).
**Note** : the paths have to be given as relatives paths, relative to the ```--input-root```

```
2030 ./node_2030_study
2040 ./node_2040_study
2050_A ./node_2050_A_study
2050_B ./node_2050_B_study
```

The full input files & folders layout for this example is diplayed
in [this section](./trajectory-workflow.md#file-structure) for more clarity.

## Weights / additional constraints reference file

The weights (resp. additional constraints) reference file gives the ```MultipleProblemGeneration``` executable
information about the potential custom weights file (resp. additional constraints file) given by the user.

The reference file is a simple text file with two columns :
The first column contains the node's name, and the second the path to the custom weights file (resp. additional
constraints file).
**Note** : the paths have to be given as relatives paths, relative to the ```--dataDir```

```
2030 ./node_2030_study/user/expansion/weights/weights.txt
```

If a node does not appear in this file, it will be assumed to have no custom weights file (resp. no additional
constraints file).

- [See this section](https://antares-doc.readthedocs.io/en/latest/reference/xpansion-settings/#yearly-weight) for information about each individual node's
  custom weight file
- [See this section](https://antares-doc.readthedocs.io/en/latest/reference/xpansion-settings/#additional-constraints) for information about each individual
  node's additional constraints.

In this example, for node ```2030```, the result will be the same as calling :  
```<single_problem_gen_executable> --study ./node_2030_study --weights ./node_2030_study/user/expansion/weights/weights.txt```.  
And for node ```2040```, the result will be the same as calling :   
```<single_problem_gen_executable> --study ./node_2040_study```

## Output : Nodal Lp Info file

We give an example of a corresponding ```nodal_lp_info.json```.
This file is written to the path given in the ```--nodal-file``` argument when calling the executable.

```json
{
  "2030": {
    "lp_folder": "./node_2030_study/output/20250526-1505eco/lp/",
    "master_file": "master",
    "structure_file": "structure.txt",
    "weights_file": "weights.txt"
  },
  "2040": {
    "lp_folder": "./node_2040_study/output/20250526-1505eco/lp/",
    "master_file": "master",
    "structure_file": "structure.txt"
  },
  "2050_A": {
    "lp_folder": "./node_2050_A_study/output/20250526-1505eco/lp/",
    "master_file": "master",
    "structure_file": "structure.txt"
  },
  "2050_B": {
    "lp_folder": "./node_2050_B_study/output/20250526-1505eco/lp/",
    "master_file": "master",
    "structure_file": "structure.txt"
  }
}
```

This gives the following information :

- ```lp_folder``` is the path to the folder containing the output of the problem generation.
- ```master_file```is the name of the file containing this node's master problem (no file extension, file extension is
  implicitely deduced from ```PROBLEMS_FORMAT``` input option later.)
- ```structure_file``` is the corresponding structure file (mostly useful if the name is not the default
  ```structure.txt```).
- ```weights_file``` points to the weights file outputted by the problem generation if it exists (optional).
