# Trajectory investment problem
## General description

Recall the annual investment problem in Xpansion :

$$
\min_{x \in \mathcal{X}} \quad C^T x + \text{ANTARES}(x)
$$
over a set of investment variables specified by the user, where :  

- $x$ is the vector of the capacities installed for each candidate  
- $C$ contains the fixed cost annuities of those candidates  
- $\text{ANTARES}(x)$ is the operating cost of the system for a given investment level. 


We want to switch to a pluriannual vision and optimise the investments over several possible trajectories described on a diverging tree of scenarios

![](../../assets/media/trajectory/trajectory.png)

**Figure 1** - Trajectory tree made up of annual Xpansion studies

The optimisation problem we now want to solve is, denoting by $n \in \mathcal{T}$ a node in the tree:

Alternative 1

$$
\begin{aligned}
    \min_{x, dx^+, dx^-} \quad & \sum_{n \in \mathcal{T}} IC_n^T dx_n^+ + DC_n^T dx_n^- + w(n) \times (OC_{n}^Tx_{n} + ANTARES_n(x_n))\\\\
    \text{s.t.} \quad & \forall i,n \quad  x_{i,n} = x_{i, \text{parent}(n)} + dx^+_{i,n}
    - dx^-_{i,n} \\\\
    & \forall i,n \quad dx^+_{i,n} \geq 0, \quad dx^-_{i,n} \geq 0 \\\\
    & \forall i,n \quad 0 \leq x_{i,n} \leq X_{i,n}^{max} \\
\end{aligned}
$$

- $IC_n$ contains the one-time payment investment costs per MW.
- $DC_n$ contains the one-time payment retirement costs per MW.
- $OC_n$ contains the annual operation and maintenance fixed costs.
- $w(n) = P(n) \times \times \sum_{y = y_n}^{y = y_n + d_n - 1} \frac{1}{(1+r)^{y - y_0}}$.
- $P(n)$ is the probability of realisation of node $n$ : $P(n) = P_{\text{parent}(n)}(n) \times P(\text{parent}(n))$.
- $P(root) = 1$.

**Note** : In our model, $x_{i,n}$ is the capacity available during period $n$, and this means the decisions $dx_{i,n}^{+/-}$ represent the variation of capacity during the period between $\text{parent}(n)$ and $n$ (i.e. the capacity being built or decommisionned during this period, with effective entry into service at the beginning of $n$).

We can impose the decisions to be the same in all children of a given node (see [trajectory constraints](./merge-master.md#trajectory-constraints)) if we want the investment decision that take effect for a certain period to be the same regardless of all scenarios for this investment period.  

- In the example from **Figure 1**, this would mean that the capacity we install in the period [2040, 2050] is independent on wether ```2050_A``` or ```2050_B``` will be realised.