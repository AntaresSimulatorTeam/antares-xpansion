# Data test mini_netwrok

## Brief

This test case is a toy network design problem to test the resolution of a stochastic problem by AntaresXpansion optimization modules :

* benderssequential
* bendersmpi
* merge_mps

This data is used in the test bendersEndToEnd. This data is used in two different tests cases :

* Resolution with default options, with the file *options_default.txt*
* Resolution with a file defining the weights of each subproblems in a file *slave_weights.txt*, with options file *options_weights.txt*

## Problem formulation

We denote by *w1* (resp. *w2*) the weight of subproblem SP1 (resp. SP2). 

Variables :
* t : transmission capacity investment
* p : production capacity investment
* z1 : actual flow in scenario 1
* z2 : actual flow in scenario 2
* perte1 : breakdown variable in scenario 1
* perte2 : breakdown variable in scenario 2

min 3 t + 2 p + 1.5 w1 z1 + 1.5 w2 z2 + 100 (perte1 + perte 2)
sc:
    t + p <= 10 *budget constraint*
    z1 <= t
    z1 <= p
    z1 + perte1 = 2.0 *demand constraint in scenario 1*
    z2 <= t
    z2 <= p
    z2 + perte2 = 6.0 *demand constraint in scenario 2*
    t, p, z1, z2, perte1, perte1 >= 0

## Weights file format

The weights file format is the following :
* One line defining the total weight apploed to subproblems, with the key WEIGHT_SUM
    WEIGHT_SUM     K
* One line by subproblem with its particular weight, with its name as key :
    SP1         k1
    SP2         k2

The weight applied to subproblem SPi = ki / K  .
The file *slave_weights.txt* gives an example.