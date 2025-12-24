# Stopping criterion computation in Benders by batch

## Status

Accepted

## Context

The master problem at a given iteration of the Benders algorithm is:

$$
\begin{aligned}
    \min\_{x \in \mathcal{X}}\ & c^{\top}x +  \vartheta \\\\
    \text{s.t.} \ & Ax = b\\\\
    & \vartheta = \sum_{s} \vartheta_{s}\\\\
    & \text{Some Benders cuts}
\end{aligned}
$$

### Reminder of the case with no cut aggregation

When there is no cut aggregation, if there are \\(S\\) subproblems, the cuts added at each iteration are of the form:

$$
\begin{aligned}
    & \vartheta_{1} \geq \pi_{1}^{\top} (d_{1} - T_{1}x)\ ,  \\\\
    & \ldots \\\\
    & \vartheta_{S} \geq \pi_{S}^{\top} (d_{S} - T_{S}x) \ ,
\end{aligned}
$$

The Benders by batch algorithm consists in computing after the resolution of each batch, a "contribution of each subproblem" to the filling of the gap. If with the current batch, we overfill the gap compared to the stopping gap criterion, we proceed to the next iteration by solving a master problem. The algorithm stops once we have managed to fit all batches without overtaking the required optimality gap.

Without cut aggregation, the contribution of subproblem \\(s\\) to the gap is given by \\( (\phi_{s}(x) - \underline{\vartheta}_{s})^{+}  \\) where:
- \\(x, (\underline{\vartheta}_{s})_s\\) is the solution of the previous master problem 
- \\( \phi_{s}(x)\\) is the objective value of subproblem \\(s\\) when first stage variables are fixed at \\(x\\)

Fundamentally, as we do not aggregate cuts, we build an under approximation of each subproblem individually.

### Case of cut aggregation

Now suppose we aggregate cuts, so that we end up with \\(K\\) cuts. We get the following cuts:
$$
\begin{aligned}
    & \vartheta_{1} + \ldots + \vartheta_{k_1} \geq \sum_{s=1}^{k_1} \pi_{s}^{\top} (d_{s} - T_{s}x)\ ,  \\\\
    & \ldots \\\\
    & \vartheta_{s_{K-1}} + \ldots + \vartheta_{s_K} \geq \sum_{s=k_{K-1}}^{s_K} \pi_{s}^{\top} (d_{s} - T_{s}x)\ ,
\end{aligned}
$$

The previous equation have been written (for simplicity to avoid complicated notations) in the case we aggregate cuts of successive weeks, but we can naturally aggregate cuts in any way. This does not change the following reasoning.

We see that the master problem is underdetrmined. Indeed, variables \\(\vartheta\\) within each cut could be aggregated (the sum of \\(\vartheta\\) of each cut is uniquely determined, but within the sum, the individual \\(vartheta\\) are free). In practice (due to the algorithmic nature of the simplex algorithm ?), we end up with very low individual \\(\vartheta\\) combined with very large ones (order \\(-10^{10} and 10^{10}\\)), but still satisfying the sum constraints.

Fundamentally, we are building an under approximation of the sum of subproblems from each cut, but we have no detailed information of individual sub-approximation.

Then the criterion should compute a contribution to the gap of each cut, and not of each subproblem. Therefore the contribution to the gap is now given by \\( (\sum_{s \in \text{cut}} \phi_{s}(x) - \underline{\vartheta}_{s})^{+}  \\)

## Decision

Update the criterion computation in Benders by batch so that it is mathematically correct.

## Consequences

- Before the change the Benders by batch could not converge with cut aggregation because the indivdual \\(\vartheta\\) did not reflect correctly the subproblem under approximation (we have very large ones that always overtake the required gap, while negative ones were considered as zero because of the positive part). Therefore it was impossible to solve several batches at a given iteration.
- After the modification, the algorithm behaves as expected.
