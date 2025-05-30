# Weights

## Why we need custom weights
The weights given by the user in the ```master_structure_file``` are only applied to the merged master problem's objective. This means that the user must take care to also specify the correct corresponding yearly weights to each subproblem for each annual Xpansion study using ```SLAVE_WEIGHTS``` parameter when solving the merged master problem.
Thus, if the Monte Carlo years of node $n \in G$ had respective weights $(\omega_{i,n})_{\forall i \in [|1, N|]}$, we must set the new weights of each subproblem of the merged master problem as : $(\omega^{merged}_{i,n} = w(n) \times \omega_{i,n})_{\forall n \in G, \forall i \in [|1, N|]}$.
Note that this means that the weights given to the subproblems do not add up to $1$, as they are now reflective of the duration represented by the assiociated node, its probability of occurence and the probability of occurence of each specific Monte-Carlo year inside a specific node - whereas in the annual version, the weight of the node was 1 (the node represented one year and was the only possible scenario) and thus the weights of subproblems was simply the probability of occurence of each Monte-Carlo year.

## Usage

Usage is ```<merge_weights_executable> <master_structure.json> <nodal_lp_info.json> <path/to/output/weights/file>

- ```<master_structure.json>``` designates the intermediary file outputed by the initial input parser / translator. [See this section](./merge-master.md#master-structure-file) for more details. In this part of the workflow, only each node's ```node_weight``` entry will be of use to us.
- ```<nodal_lp_info.json>``` designates the intermediary file outputed by the ```MultipleProblemGeneration``` step. [See this section](./multiple-problem-generation.md#output--nodal-lp-info-file) for more details. 

Note that in the ```<nodal_lp_info.json>``` file, if the ```weights_file``` parameter does not appear or points to a non existant file, the weights of this node are assumed to be uniform.

## Output

The ouput ```weights_merged.txt``` file will be a file with two columns.  
The first column is the path to the subproblem, and the second the corresponding weight:
```
./node_2030_study/output/20250526-1505eco/lp/problem-1-1--optim-nb-1.mps 0
./node_2030_study/output/20250526-1505eco/lp/problem-1-2--optim-nb-1.mps 0
./node_2030_study/output/20250526-1505eco/lp/problem-2-1--optim-nb-1.mps 10.0
./node_2030_study/output/20250526-1505eco/lp/problem-2-2--optim-nb-1.mps 10.0
./node_2040_study/output/20250526-1505eco/lp/problem-1-1--optim-nb-1.mps 5.0
./node_2040_study/output/20250526-1505eco/lp/problem-1-2--optim-nb-1.mps 5.0
./node_2040_study/output/20250526-1505eco/lp/problem-2-1--optim-nb-1.mps 5.0
./node_2040_study/output/20250526-1505eco/lp/problem-2-2--optim-nb-1.mps 5.0
./node_2050_A_study/output/20250526-1505eco/lp/problem-1-1--optim-nb-1.mps 4.0
./node_2050_A_study/output/20250526-1505eco/lp/problem-1-2--optim-nb-1.mps 4.0
./node_2050_A_study/output/20250526-1505eco/lp/problem-2-1--optim-nb-1.mps 4.0
./node_2050_A_study/output/20250526-1505eco/lp/problem-2-2--optim-nb-1.mps 4.0
./node_2050_B_study/output/20250526-1505eco/lp/problem-1-1--optim-nb-1.mps 1.0
./node_2050_B_study/output/20250526-1505eco/lp/problem-1-2--optim-nb-1.mps 1.0
./node_2050_B_study/output/20250526-1505eco/lp/problem-2-1--optim-nb-1.mps 1.0
./node_2050_B_study/output/20250526-1505eco/lp/problem-2-2--optim-nb-1.mps 1.0
WEIGHT_SUM 1
```
In this example, the user has set a custom weight of 0 on ```2030```'s first MC year and 1 on its only other MC year.  
See [the first section](#why-we-need-custom-weights) as to why the ```WEIGHT_SUM``` entry is set to 1.