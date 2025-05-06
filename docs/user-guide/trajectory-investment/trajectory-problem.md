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

The optimisation problem we now want to solve is, denoting by \\( \mathbb{n} \in \mathcal{T} \\) a node in the tree:

$$
\begin{aligned}
    \min_{x, dx^+, dx^-} \quad & \sum_{\mathbb{n} \in \mathcal{T}} IC_n^T dx_n^+ + DC_n^T dx_n^- + w(\mathbb{n}) \times (OC_{\mathbb{n}}^Tx_{\mathbb{n}} + ANTARES_n(x_n))\\\\
    \text{s.t.} \quad & \forall i,a \quad  x_{i,\mathbb{n}} = x_{i, \text{parent}(\mathbb{n})} + dx^+_{i,\mathbb{n}}
    - dx^-_{i,\mathbb{n}} \\\\
    & \forall i,\mathbb{n} \quad dx^+_{i,\mathbb{n}} \geq 0, \quad dx^-_{i,\mathbb{n}} \geq 0 \\\\
    & \forall i,\mathbb{n} \quad 0 \leq x_{i,\mathbb{n}} \leq X_{i,\mathbb{n}}^{max} \\
\end{aligned}
$$

TBA
