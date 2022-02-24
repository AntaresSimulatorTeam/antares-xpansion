# Sensitivity analysis

Antares-Xpansion solves an investment problem and returns the optimal combination of investment candidates. Then, it may be worth to assess the robustness of the optimal solution by looking at near optimal solutions, see **Figure 16**. 

We call \\(\varepsilon\\)-optimal solution a combination of investments that is within \\(\varepsilon\\) euros of the optimal cost. The interest of knowing the set of \\(\varepsilon\\)-optimal solutions is the following:

- Suppose that there exists an \\(\varepsilon\\)-optimal solution for which the invested capactities are very different from the optimal solution. This means that the optimal solution is not stable with respect to these capacities. The cost difference between technologies does not allow to clearly distinguish these solutions.

- On the other hand, if all \\(\varepsilon\\)-optimal solutions have almost the same invested capacities, then the solution is robust and the cost-effectiveness of the investment is ensured.

![](../../assets/media/sensitivity_illustration.png)

** Figure 16 ** - Example of \\(\varepsilon\\)-optimal solutions. There exists solutions with an investment in the range \\([x_{lb}, x_{ub}]\\) that are closer than \\( \varepsilon \\) euros to the optimal cost \\( UB^{*} \\).

The sensitivity analysis module of Anatres-Xpansion is able to perform the following computation:

- It determines the min and max capacity of a given candidate for which there exists an \\(\varepsilon\\)-optimal solution. The associated \\(\varepsilon\\)-optimal solution is also returned. This allows to define, for each candidate, the interval within which all \\(\varepsilon\\)-optimal solution can be found.
- It computes the \\(\varepsilon\\)-optimal solution that minimizes or maximizes the investment cost (CAPEX).

## Description of the method

The sensitivity analysis computation is based on the last master problem used during the Benders alogrithm. The Benders cuts of the last master (dotted grey lines in **Figure 16**) define a piecewise approximation of the cost function (in blue, that is unknown).

The user sets a threshold \\(\varepsilon\\) up to which investment solutions are considered to be _near optimal_ or _equivalent_. The sensitivity analysis is an exploration, in a given _direction_, of the set of solutions that are within \\(\varepsilon\\) euros of the optimal cost.

**Figure 16** illustrates the case where we look at the range of invested capacity for candidate \\(i\\) for \\(\varepsilon\\)-optimal solutions. We can then deduce the min and max invested capacity for this candidate for which there exists an \\(\varepsilon\\)-optimal solution. The capacity interval we get is the projection of the set of \\(\epsilon\\)-optimal solutions on the dimension of the capacity of cnadidate \\(i\\).

!!! Remark
    As we only known a lower approximation of the real cost function (see **Figure 16**), the width of the capacity interval given by the sensitivity analysis may be overestimated.

The capacity intervals of all candidates define a _hyperrectangle_ that is (often strictly) larger than the set of \\(\varepsilon\\) optimal solutions.

!!! Warning
    There may be some solutions within the hyperrectangle that are **not \\(\varepsilon\\)-optimal**. For example, the solution where all capacities are taken to be the lower bound of the candidate interval is not necessarily \\(\varepsilon\\)-optimal. However, for each bound of each candidate interval, the sensitivity analysis module returns an \\(\varepsilon\\)-optimal solution where this bound is reached.

It is also possible to find the \\(\varepsilon\\)-solution with min or max CAPEX. This is just another _direction_ of exploration of the set of \\(\varepsilon\\)-optimal solutions.



## Mathematical formulation of the sensitivity analysis problem

Let us recall the formulation of the master problem:
$$
\begin{aligned}
    \min\_{x \in \mathcal{X}}\ & c^{\top}x + \frac{1}{N} \sum_{l=1}^{N} \sum_{s=1}^{52} p_{l, s}\vartheta_{l, s} \\\\
    \text{s.t.} \ & Ax = b\\\\
    & \vartheta_{l, s} \geq {{}\bar{\pi}\_{l, s}^{i}}^{\top} (d_{l, s} - T_{s}x)\ , \quad \forall l \ , \forall s \ , \forall i
\end{aligned}
$$
where the constraints \\( \vartheta_{l, s} \geq {{}\bar{\pi}\_{l, s}^{i}}^{\top} (d_{l, s} - T_{s}x) \\) are the Benders cuts. More details can be found in [Mathematical formulation of the investment problem](../optimization-principles/problem-formalization.md).

The sensitivity analysis problem is then of the form:
$$
\begin{aligned}
    \min\_{x \in \mathcal{X}}\ & \tilde{c}^{\top}x \\\\
    \text{s.t.} \ & Ax = b\\\\
    & \vartheta_{l, s} \geq {{}\bar{\pi}\_{l, s}^{i}}^{\top} (d_{l, s} - T_{s}x)\ , \quad \forall l \ , \forall s \ , \forall i \\\\
    & c^{\top}x + \frac{1}{N} \sum_{l=1}^{N} \sum_{s=1}^{52} p_{l, s}\vartheta_{l, s} \leq UB^{*} + \varepsilon
\end{aligned}
$$
where the last constraint means that we are looking only for solutions within \\(\varepsilon\\) euros of the optimal solution. The cost vector \\(\tilde{c}\\) defines the _direction_ in which we explore the set of \\(\varepsilon\\)-optimal solutions:

- With \\(\tilde{c} = (0,\ldots,0,1,0,\ldots,0)\\) where the \\(1\\) is in the \\(i\\)-th position, the cost vector becomes \\((0,\ldots,0,x_{i},0, \ldots, 0)\\). The sensitivity problem consists in finding the \\(\varepsilon\\)-optimal solution with the least installed capacity for candidate \\(i\\).
- With \\(\tilde{c} =c\\), the cost vector is \\(c^{\top}x\\), which is exactly the CAPEX. The sensitivity problem consists in finding the \\(\varepsilon\\)-optimal solution with the minimum investment cost.

The sensitivity problem is also solved as a maximization problem to find \\(\varepsilon\\)-optimal solutions with the maximum installed capacity for some candidate or the maximum CAPEX.

## Results interpretation

Suppose that for candidate \\(i\\), the one-dimensional projection of \\(\varepsilon\\)-optimal solutions is the interval \\([\underline{x}_{i}, \overline{x}^{i}]\\).

- The lower bound \\(\underline{x}_{i}\\) is the minimum installed capacity of candidate \\(i\\) found in **every** \\(varespilon\\)-optimal solution. This means that for an investment up to \\(\underline{x}\_{i}\\), profitability is ensured.

- The upper bound \\(\overline{x}_{i}\\) is the maximum installed capacity of candidate \\(i\\) found in the set of \\(varespilon\\)-optimal solutions.

The width of the interval gives information on the robustness of the solution:

- If the interval \\([\underline{x}\_{i}, \overline{x}^{i}]\\) is tight, this means that all _equivalent_ solutions have almost the same installed capacity of candidate \\(i\\). The optimal solution is _stable_ for this candidate, therefore the investment is profitable and robust to small variations of the cost function.

- If the interval is large, the cost function near the optimum is flat in the direction of candidate \\(i\\). The economic criterion alone is not sufficient to choose a capacity value over another. 

