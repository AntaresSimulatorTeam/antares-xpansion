# Trajectory investment problem
## General description

Recall the annual investment problem in Xpansion :

$$
\min_{x \in \mathcal{X}} \quad \sum_i C_i \, x_i + \text{ANTARES}(x)
$$

over a set of investment variables specified by the user, where, denoting by $i$ a candidate :

- $x_i$ is the capacity installed for candidate $i$, and $x = (x_i)_i$ is the vector of those capacities
- $C_i$ is the fixed cost annuity of candidate $i$, given by its ```annual-cost-per-mw``` entry in the study's [```candidates.ini```](../inputs/candidates.md) file
- $\text{ANTARES}(x)$ is the operating cost of the system for a given investment level.

## Switching to a pluriannual vision

We want to switch to a pluriannual vision and optimise the investments over several possible trajectories described on a diverging tree of scenarios

![Trajectory tree](../../assets/media/trajectory/trajectory.png){: .center}

**Figure 1** - Trajectory tree made up of annual Xpansion studies

The optimisation problem we now want to solve is, denoting by $n \in \mathcal{T}$ a node in the tree, by $i$ a candidate and by $x_n = (x_{i,n})_i$ the vector of the capacities installed at node $n$:

$$
\begin{aligned}
    \min_{x, dx^+, dx^-} \quad & \sum_{n \in \mathcal{T}} \left[ \sum_i \left( wIC_{i,n} \, dx^+_{i,n} + wDC_{i,n} \, dx^-_{i,n} + wOC_{i,n} \, x_{i,n} \right) + w_n \times ANTARES_n(x_n) \right]\\\\
    \text{s.t.} \quad & \forall i,n \quad  x_{i,n} = x_{i, \text{parent}(n)} + dx^+_{i,n}
    - dx^-_{i,n} \\\\
    & \forall i,n \quad dx^+_{i,n} \geq 0, \quad dx^-_{i,n} \geq 0 \\\\
    & \forall i,n \quad 0 \leq x_{i,n} \leq X_{i,n}^{max} \\
\end{aligned}
$$

with $r$ the discount rate, $y_0$ the first investment year, $y_n$ the investment date of node $n$ and $d_n$ its represented duration ($d_n$ = next investment date $- y_n$, or end of horizon $- y_n$ for a leaf) :

- $p_n$ is the transition probability from $\text{parent}(n)$ to $n$.
- $P(n)$ is the probability of realisation of node $n$ : $P(n) = p_n \times P(\text{parent}(n))$, with $P(\text{root}) = 1$.
- $w_n = P(n) \times \sum_{y = y_n + 1}^{y = y_n + d_n} \frac{1}{(1+r)^{y - y_0}}$.
- $wIC_{i,n} = P(n) \times \frac{1}{(1+r)^{y_n - y_0}} \times IC_i$, the weighted and discounted one-time payment investment cost per MW.
- $wDC_{i,n} = P(n) \times \frac{1}{(1+r)^{y_n - y_0}} \times DC_i$, same for the retirement cost per MW.
- $wOC_{i,n} = w_n \times OC_i$, the weighted and discounted operation and maintenance fixed cost per MW, paid every year of the period.

!!! note "On the $dx^{+/-}$ variables"

    In our model, $x_{i,n}$ is the capacity available during the period represented by $n$, and this means the decisions $dx_{i,n}^{+/-}$ represent the variation of capacity during the period between $\text{parent}(n)$ and $n$ (i.e. the capacity being built or decommisionned during the period represented by $\text{parent}(n)$, with effective entry into service at the beginning of $n$).

    We can impose the decisions to be the same in all children of a given node (see [trajectory constraints](./merge-master.md#trajectory-constraints)) if we want the investment decision of a given period to be independent of what scenario will materialize in the next period when the new capacities enter into service.

    In the example from **Figure 1**, this would mean that the capacity we install in the period [2040, 2050] is independent of wether ```2050_A``` or ```2050_B``` will be realised.


## Link with the user input file

Every parameter of the problem comes from the [user input file](./user-input.md), except the bounds $X_{i,n}^{max}$ :

| Parameter | Comes from |
|---|---|
| $\mathcal{T}$, $\text{parent}(n)$ | the ```tree``` section |
| $p_n$ | the ```probability``` entry of the ```tree``` section |
| $r$ | ```global: discount_rate``` |
| $y_0$ | ```global: first_investment_year``` |
| $y_n$ | the ```investment_date``` of the node |
| $d_n$ | the difference between the ```investment_date``` of the children of $n$ and $y_n$, or ```global: end_of_horizon``` $- \, y_n$ for a leaf |
| $IC_i$, $DC_i$, $OC_i$ | the ```investment```, ```retirement``` and ```operation_maintenance``` [costs](./user-input.md#candidates-costs) of the type associated to the candidate by the node's ```candidate_to_type``` entry |
| $ANTARES_n$ | the annual study of the node, given in ```global: studies``` |
| $x_{i, \text{parent}(\text{root})}$ | ```initial_capacities```, the capacity installed before the first investment date, or before the node where the candidate [first appears](./user-input.md#a-note-on-candidates) |
| $X_{i,n}^{max}$ | the ```candidates.ini``` of the node's annual study, not the trajectory input file |

The ```constraints``` section adds constraints on the $dx^{+/-}_{i,n}$ variables, ```global: formulation``` sets whether the investment variables are integer or relaxed, and ```global: scaling``` divides every coefficient of the objective without changing the optimal solution.

## Link between the annual and the pluriannual problems

The annual problem is the pluriannual problem with a single node $n$ of duration $d_n$ : being the root, $P(n) = 1$ and $y_n = y_0$, and with no initial capacity nor retirement, $dx^+_{i,n} = x_{i,n}$ and $dx^-_{i,n} = 0$. The objective is then :

$$
\sum_i \left( IC_i + w_n \, OC_i \right) x_{i,n} + w_n \times ANTARES_n(x_n),
$$

with : 

$$
w_n = \sum_{k = 1}^{d_n} \frac{1}{(1+r)^{k}} = \frac{1 - \frac{1}{(1+r)^{d_n}}}{r}
$$

Dividing by $w_n$ gives back $\sum_i C_i x_i + \text{ANTARES}(x)$ with $C_i = \frac{IC_i}{w_n} + OC_i = \frac{r}{1 - \frac{1}{(1+r)^{d_n}}}IC_i + OC_i$ : the annuity amortises the investment cost over the $d_n$ years of the node.
