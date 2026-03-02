# Mathematical aspects of the investment problem

In this part, we give the mathematical formulation of the investment problem considered in Antares-Xpansion. We also explain the Benders decomposition algorithm that is used.

## Problem formulation

We consider a power system with given production and trasmission capacities. Antares-Xpansion seeks to invest in additional capacties in order to minimize the sum of annualized investment costs and yearly operating costs.

### Variables and costs

#### Invested capacity and investment cost

Let \\((x_i)\_{1 \leq i \leq n}\\) be the variables modeling the invested capacities (in MW) of the candidates.

- If the investment is continuous, \\(x_{i} \in [0, \overline{x}\_{i}] \ \\), where \\(\overline{x}\_{i} \\) is the upper bound on the investment for candidate \\(i \\) i.e. \\(\overline{x}\_{i}\\) is the `max-investment`. 

    We denote by \\(\mathcal{R}\\) (for real) the set of indices of candidates with continuous investment.

- If the investment is integer, \\(x_{i} \in \\{0, u_{i}, \ldots, \overline{x}\_{i} = K_{i}u_{i}\\} \\), where \\(u_{i} \in \mathbb{R}\\) is the `unit-size` and \\(K_{i} \in \mathbb{N}\\) is the maximum number of buildable units `max-units`. Then, \\(\overline{x}\_{i} = K_{i}u_{i}\\) is the upper bound on the investment for candidate \\(i \\) i.e. \\(\overline{x}\_{i}\\) is `max-units` \\(\times\\) `unit-size`. 
    
    We denote by \\(\mathcal{I}\\) (for integer) the set of indices of candidates with integer investment. Thus, we have \\(\mathcal{R} \cup \mathcal{I} = \\{1, \ldots, n \\} \\).

Each candidate is also characterized by an annualized investment cost per MW installed \\(c_{i} \in \mathbb{R}_{+}\ \\).

The investment vector is denoted by \\(x = (x_{1}, \ldots, x_{n})\\) and the cost vector by \\(c = (c_{1}, \ldots, c_{n})\\), so that the investment cost is \\(c^{\top}x\\). 

#### Operating cost

The system is also characterized by an expected yearly operating cost, denoted by \\(\Theta(x) \in \mathbb{R}\\). The cost \\(\Theta(x)\\) is the output of an Antares simulation for the system with the investment level \\(x\\).

##### The operating cost as the solution of an Antares simulation

The expected operating cost in Antares is estimated over \\(N\\) scenarios (a.k.a. MC years), that is:

$$
\Theta(x) = \sum_{l=1}^{N} p_{l} \theta_{l}(x)
$$

where \\(p_{l}\\) is the weight of year \\(l\\). The cost \\(\theta_{l}(x)\\) is the solution of the relaxed yearly Antares problem, that can be written in compact form:

$$
\begin{aligned}
    \theta_{l}(x) = \min_{y \in \mathcal{Y}} \ & g_{l}^{\top} y\\\\
    \text{s.t.} \ & Wy = d_{l} - T_{l}x
\end{aligned}
$$

where \\(y\\) represents all the variables of the Antares problem, \\(\mathcal{Y}\\) is the admissible set and \\(g_{l}\\) is the cost vector. The matrices \\(W\\) and \\(T_{l}\\) as well as the vector \\(d_{l}\\) are used to model the constraints of the system. The differences between MC years come from the availability of thermal plants, the load, the wind and solar power generation, and the hydraulic inflows, that are taken into account in the right-hand side of the constraint through the term \\(d_{l}\\). The term \\(T_{l}\\) changes between MC years due to link profiles on investment candidates.

More details on the Antares problem can be found in the [Antares documentation](https://antares-simulator.readthedocs.io/en/latest/reference-guide/11-modeling/). We simply mention that the linear problem presented here and used in Antares-Xpansion is a relaxation of the Antares problem as unit-commitment constraints (minimum on and off time) are not taken into account.

##### Splitting the weeks

In fact, the variables in the Antares problem are defined with an hourly time step so that \\(y = (y_{1}, \ldots, y_{8760})\\), where \\(y_{h}\\) gathers all the variables of the Antares problem at hour \\(h\\) of the year. Antares solves weekly problems, that involves only the variables of a given week i.e. the problem on week \\(s \in [1,52]\\) involves only the subvector \\(y_{s} = (y_{168 (s-1) + 1}, \ldots, y_{168s})\\). In the sequel, the index \\(_s\\) always denotes subvectors correponding to week \\(s\\). 

In Antares-Xpansion, the **weekly problems are assumed to be independent**, this is why, **no coupling constraints between the weeks** are allowed. By doing so, the matrix \\(W\\) is **block diagonal** i.e. \\(W = \mathrm{diag}(W_{1}, \ldots, W_{52})\\). Writing:

- \\(g_{l} = (g_{l,1}, \ldots, g_{l,52})\\),
- \\(d_{l} = (d_{l, 1}, \ldots, d_{l, 52})\\) ,
- \\(T_{l} = \begin{pmatrix} T_{l, 1} \\\\ \vdots \\\\ T_{l, 52} \end{pmatrix} \\),

the yearly Antares problem (here for MC year \\(l\\)) can be split in 52 independent weekly problems, given for week \\(s\\) by:

$$
\begin{aligned}
    \theta_{l, s}(x) = \min_{y_{s} \in \mathcal{Y}\_{s}} \ & g_{l,s}^{\top} y_{s}\\\\
    \text{s.t.} \ & W_{s}y_{s} = d_{l, s} - T_{l, s}x
\end{aligned}
$$

where \\(\theta_{l, s}(x)\\) is the operational cost of week \\(s\\) of MC year \\(l\\) and \\(\mathcal{Y}\_{s}\\) is the admissible set for the variables of week \\(s\\).

Overall, the expected yearly operating cost becomes:

$$
\Theta(x) = \sum_{l=1}^{N} p_{l} \sum_{s=1}^{52} \theta_{l, s}(x)
$$

#### Summary of the costs

For an investment \\(x\\), the overall cost of the system is given by \\(c^{\top}x + \Theta(x)\\), where \\(\Theta(x)\\) is computed from the solution of \\(52N\\) linear weekly Antares problems, with \\(N\\) the number of MC years.

### Constraints (for the investment problem)

The invested capacities of the different candidates can be bound by linear constraints, specified by the user with the [`additional-constraints`](../get-started/settings-definition.md#additional-constraints) parameter. These constraints are written \\(Ax = b\\), with \\(A \in \mathbb{R}^{m \times n}\\) and \\(b \in \mathbb{R}^{m}\\), where \\(m\\) is the number of constraints.

### Investment problem

The optimal investment problem is therefore:

$$
\begin{aligned}
    \min\_{x \in \mathcal{X}}\ & c^{\top}x + \Theta(x) \\\\
    \text{s.t.} \ & Ax = b\\
\end{aligned}
$$

where \\(\mathcal{X} = \prod_{i \in \mathcal{R}} [0, \overline{x}\_{i}] \times \prod\_{i \in \mathcal{I}} \\{0, u\_{i}, \ldots, K\_{i}u\_{i}\\}\\) is the set of admissible investment levels.

## Benders reformulation and decomposition algorithm

The problem structure, with investments as first stage decision and optimal dispatch as second stage decision, is naturally suited for a Benders decomposition. We briefly expose the ideas of the Benders reformulation and of the Benders decomposition algorithm.

### Benders reformulation of the investment problem

Taking the dual of the weekly Antares problem, we get:

$$
\begin{aligned}
    \theta_{l, s}(x) = \max_{\pi_{l, s} \in \Pi\_{l, s}} \ & \pi_{l, s}^{\top} (d_{l, s} - T_{l, s}x)\\\\
    \text{s.t.} \ & W_{s}^{\top}\pi_{l, s} \geq g_{l, s}
\end{aligned}
$$

where \\(\Pi_{l, s}\\) is the appropriate admissible set. An important feature of the dual problem is that the feasible set \\(F_{l, s}=\\{\pi_{l, s}, \ W_{s}^{\top}\pi_{l, s} \geq g_{l, s}\\} \cap \Pi_{l, s}\\), which is a polyhedron, does not depend on the investment variable \\(x\\). 

Weekly Antares problems are always feasible, by penalizing feasibility with a large enough coefficient in their objective. Then, \\(F_{l, s}\\) is always non empty and bounded. Therefore, the dual problem has a solution and we know that it is one of the extreme points of the polyhedron \\(F_{l, s}\\). We deduce:

$$
\begin{aligned}
    \theta_{l, s}(x) = \max_{\pi_{l, s} \in \mathrm{extr}(F_{l, s})} \ & \pi_{l, s}^{\top} (d_{l, s} - T_{l, s}x)\\\\
\end{aligned}
$$

where \\(\mathrm{extr}(F_{l, s})\\) is the set of extreme points of \\(F_{l, s} \\). Thus, the investment problem can be reformulated as:

$$
\begin{aligned}
    \min\_{x \in \mathcal{X}}\ & c^{\top}x + \sum_{l=1}^{N} p_{l} \sum_{s=1}^{52} \max_{\pi_{l, s} \in \mathrm{extr}(F_{l, s})} \pi_{l, s}^{\top} (d_{l, s} - T_{l, s}x) \\\\
    \text{s.t.} \ & Ax = b\\
\end{aligned}
$$

This problem can be linearized by introducing continuous variables \\(\vartheta_{l, s} \in \mathbb{R}\\) for \\(l \in [1, N]\\) and \\(s \in [1,52]\\), which gives rise to an equivalent reformulation of the investment problem, known as the Benders reformulation or _Benders master problem_:

$$
\begin{aligned}
    \min\_{x \in \mathcal{X}}\ & c^{\top}x + \sum_{l=1}^{N} p_{l} \sum_{s=1}^{52} \vartheta_{l, s} \\\\
    \text{s.t.} \ & Ax = b\\\\
    & \vartheta_{l, s} \geq \pi_{l, s}^{\top} (d_{l, s} - T_{l, s}x)\ , \quad \forall l \ , \forall s \ , \forall \pi_{l, s} \in \mathrm{extr}(F_{l, s})
\end{aligned}
$$

The constraints of the form \\(\vartheta_{l, s} \geq \pi_{l, s}^{\top} (d_{l, s} - T_{l, s}x)\\) in the master problem are referred as _Benders cuts_ in the sequel.
### The Benders decomposition algorithm

As the number of extreme points of \\(F_{l, s}\\) is often very large, the full Benders master problem has a large number of constraints, so it is difficult to work directly with.

This is why, the Benders decomposition algorithm proceeds iteratively:

1. Solve the master problem where all Benders cuts have been removed. The optimal solution \\(\bar{x}\\) is taken as a trial investment value.
2. For each MC year and each week, solve the (dual) weekly Antares problem, referred as _subproblem_, with the investment value set to \\(\bar{x}\\):

    $$
    \begin{aligned}
        \max_{\pi_{l, s} \in \Pi\_{l, s}} \ & \pi_{l, s}^{\top} (d_{l, s} - T_{l, s}\bar{x})\\\\
        \text{s.t.} \ & W_{s}^{\top}\pi_{l, s} \geq g_{l, s}
    \end{aligned}
    $$
    
    There are \\(52 N\\) subproblems to solve. Let \\(\bar{\pi}\_{l, s}\\) the solution of the subproblem for MC year \\(l\\) and week \\(s\\), so that its optimal objective value is \\(\bar{\pi}\_{l, s}^{\top} (d_{l, s} - T_{l, s}\bar{x})\\).

3. For all \\(l\\), for all \\(s\\), add the cut \\(\vartheta_{l, s} \geq \bar{\pi}\_{l, s}^{\top} (d_{l, s} - T_{l, s}x)\\) to the master problem. There are \\(52 N\\) new cuts i.e. additional constraints to the master problem.

4. At iteration \\(k\\), the master problem is of the form:

$$
\begin{aligned}
    \min\_{x \in \mathcal{X}}\ & c^{\top}x + \sum_{l=1}^{N} p_{l} \sum_{s=1}^{52} \vartheta_{l, s} \\\\
    \text{s.t.} \ & Ax = b\\\\
    & \vartheta_{l, s} \geq {{}\bar{\pi}\_{l, s}^{i}}^{\top} (d_{l, s} - T_{l, s}x)\ , \quad \forall l \ , \forall s \ , \forall i < k
\end{aligned}
$$

where \\({\bar{\pi}\_{l, s}^{i}}\\) is the solution of the subproblem for MC year \\(l\\) at week \\(s\\) at iteration \\(i < k\\). Solve this master problem. The optimal solution is denoted \\(\bar{x}\\). Go to step 2.

In order to check convergence of the algorithm, an optimality gap is computed at each iteration:

- The solution of the master problem is a lower bound of the optimal cost as it is a relaxation of the investment problem.
- With a given investment level \\(\bar{x}\\), we get a feasible solution with a cost equal to the sum of the investment cost and of the optimal cost of the subproblems: \\(c^{\top}\bar{x} + \sum_{l=1}^{N} p_{l} \sum_{s=1}^{52} \bar{\pi}\_{l, s}^{\top} (d_{l, s} - T_{l, s}\bar{x})\\). This gives a valid upper bound for the investment problem.

The optimality gap is the difference (either absolute or relative) between the lower and the upper bound. The Benders decomposition algorithm stops when the optimality gap falls below a value specified by the user (or set by default), see [`optimality_gap`](../get-started/settings-definition.md#optimality_gap) and [`relative_gap`](../get-started/settings-definition.md#relative_gap).

### The Benders by batch algorithm

In the classical Benders algorithm, all subproblems are solved at each iteration, resulting in \\(52 N\\) resolutions and as many cuts are added in the master problem. This can be quite time-consuming. The idea of the Benders by batch algorithm is to solve subproblems by batch and stop solving them at a given iteration once we know that the master solution is not optimal. Each iteration consists in fewer subproblems resolution (and fewer cuts added to the master) but we need more iterations to converge. Overall the tradeoff makes the Benders by batch algorithm usually faster than the classical Benders method. A comprehensive description of the Benders by batch algorithm can be found in the thesis of Xavier Blanchot[@blanchot_solving_2022].

## Reliability-constrained investment problem

Starting from version 1.3.0, Antares-Xpansion can take into account a reliability constraint on the maximum expected number of hours of loss of load. This means that the user is able to specify, for each area, an expected number of hours of loss a load that should not be exceeded, see [Adequacy criterion](../get-started/adequacy-criterion.md). Antares-Xpansion will output a solution that satisfy this reliability criterion using a Benders-based matheuristic designed in Chapter 5 of the thesis of Xavier Blanchot[@blanchot_solving_2022].

The heuristic is based on the insight that increasing the investment cost is strongly correlated to a decrease in loss of load. It works by iteratively solving the classical investment problem (without the reliability constraint) to which we add a minimum investment cost constraint \\(c^{\top}x \geq \Lambda\\). 

After each resolution, the expected loss of load is checked and the minimum investment cost \\(\Lambda\\) is adjusted using a dichotomy of an interval \\([\underline{\Lambda}, \overline{\Lambda}]\\). If we denote by \\(\Lambda^{(k)}, \underline{\Lambda}^{(k)}, \overline{\Lambda}^{(k)}\\) the values from the \\(k\\)-th resolution of the investment problem:
    
- If there is too much loss of load, we increase \\(\underline{\Lambda}\\) : \\(\underline{\Lambda}^{(k+1)} = \Lambda^{(k)}\\)`, 
- Otherwise, we decrease \\(\overline{\Lambda}\\): \\(\overline{\Lambda}^{(k+1)} = \Lambda^{(k)}\\)`.
- Then, we take \\(\Lambda^{(k+1)} = \frac{\overline{\Lambda}^{(k+1)}-\underline{\Lambda}^{(k+1)}}{2}\\)
    
The algorithm stops when \\(\overline{\Lambda}^{(k+1)}-\underline{\Lambda}^{(k+1)} < \varepsilon\\) where \\(\varepsilon\\) is user-defined. Antares-Xpansion outputs a feasible solution for the reliability-constrained that should be of good quality thanks to the initial insight linking investment cost and loss of load. This procedure is a heuristic so there is no guarantee to get the theoretical optimal solution.