# The Benders decomposition for the optimal investment problem

In this part, we give the mathematical formulation of the investment problem considered in Antares-Xpansion. We also explain the Benders decomposition algorithm that is used.

## Formulation of the problem

Starting from an initial system, with defined capacities and transmission lines, Antares-Xpansion seeks to invest in additional capacties in order to minimize the sum of annualized investment costs and yearly operating costs.

### Variables and costs

#### Invested capacity and investment cost

Suppose that we have \\(n\\) investment candidates. They are characterized by:

- An invested capacity \\(x_{i}\\) in MW:
    - If the investment is continuous, \\(x_{i} \in [0, \overline{x}\_{i}] \ \\), where \\(\overline{x}\_{i} \\) is the upper bound on the investment for candidate \\(i \\) i.e. \\(\overline{x}\_{i}\\) is the `max-investment`. 
    
        We deonte by \\(\mathcal{R}\\) (for real) the set of indices of candidates with continuous investment.
    
    - If the investment is integer, \\(x_{i} \in \\{0, u_{i}, \ldots, \overline{x}\_{i} = K_{i}u_{i}\\} \\), where \\(u_{i} \in \mathbb{R}\\) is the `unit-size` and \\(K_{i} \in \mathbb{N}\\) is the maximum number of buildable units `max-units`. Then, \\(\overline{x}\_{i} = K_{i}u_{i}\\) is the upper bound on the investment for candidate \\(i \\) i.e. \\(\overline{x}\_{i}\\) is `max-units` \\(\times\\) `unit-size`. 
    
        We deonte by \\(\mathcal{I}\\) (for integer) the set of indices of candidates with integer investment. Thus, we have \\(\mathcal{R} \cup \mathcal{I} = \\{1, \ldots, n \\} \\).

- An annualized investment cost per MW installed \\(c_{i} \in \mathbb{R}_{+}\ \\).

The investment vector is denoted by \\(x = (x_{1}, \ldots, x_{n})\\) and the cost vector by \\(c = (c_{1}, \ldots, c_{n})\\), so that the investment cost is \\(c^{\top}x\\). 

#### Operating cost

The system is also characterized by an expected yearly operating cost, denoted by \\(\Theta(x) \in \mathbb{R}\\). The cost \\(\Theta(x)\\) is the output of an Antares simulation for the system with the investment level \\(x\\).

##### The operating cost as the solution of an Antares simulation

The expected operating cost in Antares is evaluated through a Monte-Carlo approximation over \\(N\\) scenarios (a.k.a. MC years) \\(\\{\omega_{1}, \ldots, \omega_{N}\\}\\), that is:

$$
\Theta(x) = \frac{1}{N} \sum_{l=1}^{N} p_{\omega_{l}} \theta_{\omega_{l}}(x)
$$

where \\(p_{\omega_{l}}\\) is the weight of year \\(l\\). The cost \\(\theta_{\omega_{l}}(x)\\) is the solution of the relaxed yearly Antares problem, that can be written in compact form:

$$
\begin{aligned}
    \theta_{\omega_{l}}(x) = \min_{y \in \mathcal{Y}} \ & b^{\top} y\\\\
    \text{s.t.} \ & Wy = d_{\omega_{l}} - Tx
\end{aligned}
$$

where \\(y\\) represents all the variables of the Antares problem and \\(\mathcal{Y}\\) is the admissible set. The matrices \\(W\\) and \\(T\\) as well as the vector \\(d_{\omega_{l}}\\) are used to model the constraints of the system. The difference between MC years comes from the availability of thermal plants, the load, the wind and solar power generation, and the hydraulic inflows, that are taken into account in the right-hand side of the constraint through the term \\(d_{\omega_{l}}\\).

More details on the Antares problem can be found in the [Antares documentation](https://antares-simulator.readthedocs.io/en/latest/reference-guide/2-modeling/#formulation-of-the-optimisation-problems). We simply mention that the linear problem presented here and used in Antares-Xpansion is a relaxation of the Antares problem as unit-commitment constraints (minimum on and off time) are not taken into account.

##### Splitting the weeks

In fact, the variables in the Antares problem are defined with an hourly time step so that \\(y = (y_{1}, \ldots, y_{8760})\\), where \\(y_{h}\\) gathers all the variables of the Antares problem at hour \\(h\\) of the year. Antares solves weekly problems, that involves only the variables of a given week i.e. the problem on week \\(s \in [1,52]\\) involves only the subvector \\(y_{s} = (y_{168 s}, \ldots, y_{168 (s+1) - 1})\\). In the sequel, the index \\(_s\\) always denotes subvectors correponding to week \\(s\\). 

In Antares-Xpansion, the **weekly problems are assumed to be independent**, this is why, **no coupling constraints between the weeks** are allowed. By doing so, the matrix \\(W\\) is **block diagonal** i.e. \\(W = \mathrm{diag}(W_{1}, \ldots, W_{52})\\). Writing:

- \\(b = (b_{1}, \ldots, b_{52})\\),
- \\(d_{\omega_{l}} = (d_{\omega_{l}, 1}, \ldots, d_{\omega_{l}, 52})\\) ,
- \\(T = \begin{pmatrix} T_{1} \\\\ \vdots \\\\ T_{52} \end{pmatrix} \\),

the yearly Antares problem (here for MC year \\(l\\)) can be split in 52 independent weekly problems, given for week \\(s\\) by:

$$
\begin{aligned}
    \theta_{\omega_{l}, s}(x) = \min_{y_{s} \in \mathcal{Y}\_{s}} \ & b_{s}^{\top} y_{s}\\\\
    \text{s.t.} \ & W_{s}y_{s} = d_{\omega_{l}, s} - T_{s}x
\end{aligned}
$$

where \\(\theta_{\omega_{l}, s}(x)\\) is the operational cost of week \\(s\\) of MC year \\(l\\) and \\(\mathcal{Y}\_{s}\\) is the admissible set for the variables of week \\(s\\).

Overall, if week \\(s\\) of MC year \\(l\\) has weight \\(p_{\omega_{l}, s}\\), the expected yearly operating cost becomes:

$$
\Theta(x) = \frac{1}{52 N} \sum_{l=1}^{N} \sum_{s=1}^{52} p_{\omega_{l}, s} \theta_{\omega_{l}, s}(x)
$$

#### Summary of the costs

For an investment \\(x\\), the overall cost of the system is given by \\(c^{\top}x + \Theta(x)\\), where \\(\Theta(x)\\) is computed from the solution of \\(52N\\) linear weekly Antares problems, with \\(N\\) the number of MC years.

### Constraints (for the investment problem)

The invested capacities of the different candidates can be bound by linear constraints, specified by the user with the [`additional-constraints`](../get-started/settings-definition.md#additional-constraints) parameter. These constraints are written \\(Ax = e\\), with \\(A \in \mathbb{R}^{m \times n}\\) and \\(e \in \mathbb{R}^{m}\\), where \\(m\\) is the number of constraints.

### Investment problem

The optimal investment problem is therefore:

$$
\begin{aligned}
    \min\_{x \in \mathcal{X}}\ & c^{\top}x + \Theta(x) \\\\
    \text{s.t.} \ & Ax = e\\
\end{aligned}
$$

where \\(\mathcal{X} = \prod_{i \in \mathcal{R}} [0, \overline{x}\_{i}] \times \prod\_{i \in \mathcal{I}} \\{0, u\_{i}, \ldots, K\_{i}u\_{i}\\}\\) is the set of admissible investment levels.

## Benders reformulation and decomposition algorithm

The problem structure, with investments as first stage decision and optimal dispatch as second stage decision, is naturally suited for a Benders decomposition. We briefly expose the ideas of the Benders reformulation and of the Benders decomposition algorithm.

### Benders reformulation of the investment problem

Taking the dual of the weekly Antares problem, we get:

$$
\begin{aligned}
    \theta_{\omega_{l}, s}(x) = \max_{\pi_{s} \in \Pi\_{s}} \ & \pi_{s}^{\top} (d_{\omega_{l}, s} - T_{s}x)\\\\
    \text{s.t.} \ & W_{s}^{\top}\pi_{s} \geq b_{s}
\end{aligned}
$$

where \\(\Pi_{s}\\) is the appropriate admissible set. An important feature of the dual problem is that the feasible set \\(F=\\{\pi_{s}, \ W_{s}^{\top}\pi_{s} \geq b_{s}\\} \cap \Pi_{s}\\) does not depend on the investment variable \\(x\\). Assuming that \\(F\\) is not empty and that the dual problem has a solution (we do not consider the unbounded case in this short presentation), we know that it is one of the extreme points of \\(F\\). Therefore:

$$
\begin{aligned}
    \theta_{\omega_{l}, s}(x) = \max_{\pi_{s} \in \mathrm{extr}(F)} \ & \pi_{s}^{\top} (d_{\omega_{l}, s} - T_{s}x)\\\\
\end{aligned}
$$

where \\(\mathrm{extr}(F)\\) is the set of extreme points of \\(F \\). Thus, the investment problem can be reformulated as:

$$
\begin{aligned}
    \min\_{x \in \mathcal{X}}\ & c^{\top}x + \frac{1}{52 N} \sum_{l=1}^{N} \sum_{s=1}^{52} p_{\omega_{l}, s} \max_{\pi_{s} \in \mathrm{extr}(F)} \pi_{s}^{\top} (d_{\omega_{l}, s} - T_{s}x) \\\\
    \text{s.t.} \ & Ax = e\\
\end{aligned}
$$

This problem can be linearized by introducing continuous variables \\(\vartheta_{\omega_{l}, s} \in \mathbb{R}\\) for \\(l \in [1, N]\\) and \\(s \in [1,52]\\), which gives rise to an equivalent reformulation of the investment problem, known as the Benders reformulation or _Benders master problem_:
    
$$
\begin{aligned}
    \min\_{x \in \mathcal{X}}\ & c^{\top}x + \frac{1}{52 N} \sum_{l=1}^{N} \sum_{s=1}^{52} p_{\omega_{l}, s}\vartheta_{\omega_{l}, s} \\\\
    \text{s.t.} \ & Ax = e\\\\
    & \vartheta_{\omega_{l}, s} \geq \pi_{s}^{\top} (d_{\omega_{l}, s} - T_{s}x)\ , \quad \forall l \ , \forall s \ , \forall \pi_{s} \in \mathrm{extr}(F)
\end{aligned}
$$

The constraints of the form \\(\vartheta_{\omega_{l}, s} \geq \pi_{s}^{\top} (d_{\omega_{l}, s} - T_{s}x)\\) in the master problem are referred as _Benders cuts_ in the sequel.
### The Benders decomposition algorithm

As the number of extreme points of \\(F\\) is often very large, the full Benders master problem has a large number of constraints, so it is difficult to work directly with.

This is why, the Benders decomposition algorithm proceeds iteratively:

1. Solve the master problem where all Benders cuts have been removed. The optimal solution \\(\bar{x}\\) is taken as a trial investment value.
2. For each MC year and each week, solve the (dual) weekly Antares problem, referred as _satellite problem_, with the investment value set to \\(\bar{x}\\):

$$
\begin{aligned}
    \max_{\pi_{s} \in \Pi\_{s}} \ & \pi_{s}^{\top} (d_{\omega_{l}, s} - T_{s}\bar{x})\\\\
    \text{s.t.} \ & W_{s}^{\top}\pi_{s} \geq b_{s}
\end{aligned}
$$

There are \\(52 N\\) satellite problems to solve. Let \\(\bar{\pi}\_{\omega_{l}, s}\\) the solution of the satellite problem for MC year \\(l\\) and week \\(s\\), so that its optimal objective value is \\(\bar{\pi}\_{\omega_{l}, s}^{\top} (d_{\omega_{l}, s} - T_{s}\bar{x})\\).

3. For all \\(\omega_{l}\\), for all \\(s\\), add the cut \\(\vartheta_{\omega_{l}, s} \geq \bar{\pi}\_{\omega_{l}, s}^{\top} (d_{\omega_{l}, s} - T_{s}x)\\) to the master problem. There are \\(52 N\\) new cuts i.e. additional constraints to the master problem.

4. At iteration \\(k\\), the master problem is of the form:

$$
\begin{aligned}
    \min\_{x \in \mathcal{X}}\ & c^{\top}x + \frac{1}{52 N} \sum_{l=1}^{N} \sum_{s=1}^{52} p_{\omega_{l}, s}\vartheta_{\omega_{l}, s} \\\\
    \text{s.t.} \ & Ax = e\\\\
    & \vartheta_{\omega_{l}, s} \geq {{}\bar{\pi}\_{\omega_{l}, s}^{i}}^{\top} (d_{\omega_{l}, s} - T_{s}x)\ , \quad \forall l \ , \forall s \ , \forall i < k
\end{aligned}
$$

where \\({\bar{\pi}\_{\omega_{l}, s}^{i}}\\) is the solution of the satellite problem for MC year \\(l\\) at week \\(s\\) at iteration \\(i < k\\). Solve this master problem. The optimal solution is denoted \\(\bar{x}\\). Go to step 2.

In order to check convergence of the algorithm, an optimality gap can computed at each iteration:

- The solution of the master problem is a lower bound of the optimal cost as it is a relaxation of the investment problem.
- With a given investment level \\(\bar{x}\\), we get a feasible solution with a cost equal to the sum of the investment cost and of the optimal cost of the satellite problems: \\(c^{\top}\bar{x} + \frac{1}{52 N} \sum_{l=1}^{N} \sum_{s=1}^{52} p_{\omega_{l}, s}\bar{\pi}\_{\omega_{l}, s}^{\top} (d_{\omega_{l}, s} - T_{s}\bar{x})\\). This gives a valid upper bound for the investment problem.

The optimality gap is the difference (either absolute or relative) between the lower and the upper bound. The Benders decomposition algorithm stops when the optimality gap falls below a value specified by the user (or set by default), see [`optimality_gap`](../get-started/settings-definition.md#optimality_gap) and [`relative_gap`](../get-started/settings-definition.md#relative_gap).