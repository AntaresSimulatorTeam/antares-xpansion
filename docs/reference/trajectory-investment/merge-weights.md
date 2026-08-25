# Weights

## Why we need custom weights

The weights given by the user in the ```master_merger_info_file``` are only applied to the merged master problem's
objective. This means that the user must take care to also specify the correct corresponding yearly weights to each
subproblem for each annual Xpansion study using ```SLAVE_WEIGHTS``` parameter when solving the merged master problem.  
Thus, if the Monte Carlo years of node $n \in G$ had respective weights $(\omega_{i,n})_{\forall i \in [|1, N|]}$, we
must set the new weights of each subproblem of the merged master problem
as : $(\omega^{merged}_{i,n} = w(n) \times \omega_{i,n})_{\forall n \in G, \forall i \in [|1, N|]}$.

## Usage

Usage is ```<merge_weights_executable> <master_merger_info.json> <nodal_lp_info.json> <path/to/output/weights/file>```

- ```<master_merger_info.json>``` designates the intermediary file outputted by the initial input parser /
  translator. [See this section](./merge-master.md#master-merger-info-file) for more details. In this part of the
  workflow, only each node's ```node_weight``` entry will be of use to us.
- ```<nodal_lp_info.json>``` designates the intermediary file outputted by the ```MultipleProblemGeneration```
  step. [See this section](./multiple-problem-generation.md#output-nodal-lp-info-file) for more details.

Note that in the ```<nodal_lp_info.json>``` file, if the ```weights_file``` parameter does not appear or points to a non
existant file, the weights of this node are assumed to be uniform.

**Note** : the C++ executable has to be launched at the ```--dataDir```. (The python driver takes care of this, this
should not be a concern to most users).

## Output

The output ```weights_merged.txt``` file will be a file with two columns.  
The first column is the path to the subproblem, and the second is the corresponding weight:

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

In this example, the studies only have two weeks and two MC years, and the user has set a custom weight of $0$ on
```2030```'s first MC year and $1$ on its only other MC year.  
```WEIGHT_SUM``` is set to $1$ because those weights are already correctly normalised against the investment part of the
objective function.