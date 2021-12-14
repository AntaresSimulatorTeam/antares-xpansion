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

The cost \\(\theta_{\omega_{l}}(x)\\) is the solution of the relaxed yearly Antares problem, that can be written in compact form:
$$
\begin{align}
    \theta_{\omega_{l}}(x) = \min_{y \in \mathcal{Y}} \ & b^{\top} y\\\\
    \text{s.t.} \ & Wy = d_{\omega_{l}} - Tx
\end{align}
$$
where \\(y\\) represents all the variables of the Antares problem and \\(\mathcal{Y}\\) is the admissible set. The matrices \\(W\\) and \\(T\\) as well as the vector \\(d_{\omega_{l}}\\) are used to model the constraints of the system. The difference between MC years comes from the availability of thermal plants, the load, the wind and solar power generation, and the hydraulic inflows, that are taken into account in the right-hand side of the constraint through the term \\(d_{\omega_{l}}\\).

More details on the Antares problem can be found in the [Antares documentation](https://antares-simulator.readthedocs.io/en/latest/reference-guide/2-modeling/#formulation-of-the-optimisation-problems). We simply mention that the linear problem presented here and used in Antares-Xpansion is a relaxation of the Antares problem as start-up costs are not taken into account.

##### Splitting the weeks

In fact, the variables in the Antares problem are defined with an hourly time step so that \\(y = (y_{1}, \ldots, y_{8760})\\), where \\(y_{h}\\) gathers all the variables of the Antares problem at hour \\(h\\) of the year. Antares solves weekly problems, that involves only the variables of a given week i.e. the problem on week \\(s \in [1,52]\\) involves only the subvector \\(y_{s} = (y_{168 s}, \ldots, y_{168 (s+1) - 1})\\). In the sequel, the index \\(_s\\) always denotes subvectors correponding to week \\(s\\). 

In Antares-Xpansion, the **weekly problems are assumed to be independent**, this is why, **no coupling constraints between the weeks** are allowed. By doing so, the matrix \\(W\\) is **block diagonal** i.e. \\(W = \mathrm{diag}(W_{1}, \ldots, W_{52})\\). Writing:

- \\(b = (b_{1}, \ldots, b_{52})\\),
- \\(d_{\omega_{l}} = (d_{\omega_{l}, 1}, \ldots, d_{\omega_{l}, 1})\\) ,
- \\(T = \begin{pmatrix} T_{1} \\\\ \vdots \\\\ T_{52} \end{pmatrix} \\),

the yearly Antares problem (here for MC year \\(l\\)) can be split in 52 independent weekly problems, given for week \\(s\\) by:
$$
\begin{align}
    \theta_{\omega_{l}, s}(x) = \min_{y_{s} \in \mathcal{Y}\_{s}} \ & b_{s}^{\top} y_{s}\\\\
    \text{s.t.} \ & W_{s}y_{s} = d_{\omega_{l}, s} - T_{s}x
\end{align}
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

!!! important "Investment problem formulation"
    The optimal investment problem is therefore:

    $$
    \begin{align}
        \min\_{x \in \mathcal{X}}\ & c^{\top}x + \Theta(x) \\\\
        \text{s.t.} \ & Ax = e\\
    \end{align}
    $$

    where \\(\mathcal{X} = \prod_{i \in \mathcal{R}} [0, \overline{x}\_{i}] \times \prod\_{i \in \mathcal{I}} \\{0, u\_{i}, \ldots, K\_{i}u\_{i}\\}\\) is the set of admissible investment levels.

## Benders reformulation and decomposition algorithm

The problem structure, with investments as first stage decision and optimal dispatch as second stage decision, is naturally suited for a Benders decomposition. We briefly expose the ideas of the Benders reformulation and of the Benders decomposition algorithm.

### Benders reformulation of the investment problem

Taking the dual of the weekly Antares problem, we get:
$$
\begin{align}
    \theta_{\omega_{l}, s}(x) = \max_{\pi_{s} \in \Pi\_{s}} \ & \pi_{s}^{\top} (d_{\omega_{l}, s} - T_{s}x)\\\\
    \text{s.t.} \ & W_{s}^{\top}\pi_{s} \geq b_{s}
\end{align}
$$
where \\(\Pi_{s}\\) is the appropriate admissible set. An important feature of the dual problem is that the feasible set \\(F=\\{\pi_{s}, \ W_{s}^{\top}\pi_{s} \geq b_{s}\\}\\) does not depend on the investment variable \\(x\\). Assuming that \\(F\\) is not empty and that the dual problem has a solution (we do not consider the unbounded case in this short presentation), we know that it is one of the extreme points of \\(F \cap \Pi_{s}\\). Therefore:
$$
\begin{align}
    \theta_{\omega_{l}, s}(x) = \max_{\pi_{s} \in \mathrm{extr}(F \cap \Pi\_{s})} \ & \pi_{s}^{\top} (d_{\omega_{l}, s} - T_{s}x)\\\\
\end{align}
$$
where \\(\mathrm{extr}(F \cap \Pi\_{s})\\) is the set of extreme points of \\(F \cap \Pi_{s}\\). Thus, the investment problem can be reformulated as:
$$
\begin{align}
    \min\_{x \in \mathcal{X}}\ & c^{\top}x + \frac{1}{52 N} \sum_{l=1}^{N} \sum_{s=1}^{52} p_{\omega_{l}, s} \max_{\pi_{s} \in \mathrm{extr}(F \cap \Pi\_{s})} \pi_{s}^{\top} (d_{\omega_{l}, s} - T_{s}x) \\\\
    \text{s.t.} \ & Ax = e\\
\end{align}
$$
This problem can be linearized by introducing continuous variables \\(\vartheta_{\omega_{l}, s} \in \mathbb{R}\\) for \\(l \in [1, N]\\) and \\(s \in [1,52]\\), which gives rise to the equivalent reformulation, known as the Benders reformulation:

!!! important "Benders reformulation"
    $$
    \begin{align}
        \min\_{x \in \mathcal{X}}\ & c^{\top}x + \frac{1}{52 N} \sum_{l=1}^{N} \sum_{s=1}^{52} p_{\omega_{l}, s}\vartheta_{\omega_{l}, s} \\\\
        \text{s.t.} \ & Ax = e\\\\
        & \vartheta_{\omega_{l}, s} \geq \pi_{s}^{\top} (d_{\omega_{l}, s} - T_{s}x)\ , \quad \forall l \ , \forall s \ , \forall \pi_{s} \in \mathrm{extr}(F \cap \Pi\_{s})
    \end{align}
    $$

### The Benders decomposition algorithm