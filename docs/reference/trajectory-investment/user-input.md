# User input file parsing

## User input file

We give here an example of a user input file corresponding with the illustration given in the problem presentation.  
**input-trajectory.yaml** :

```yaml
# Global trajectory data
global:
  formulation: relaxed # If set to 'integer', capacity is built in steps of unit-size
  discount_rate: 0.064
  first_investment_year: 2030
  end_of_horizon: 2060
  scaling: 1e3 # e.g. if 1e6, objective function will be expressed in M€
  #forbid_retirement: true
  studies:
    "2030": ./node_2030_study
    "2040": ./node_2040_study
    "2050_A": ./node_2050_A_study
    "2050_B": ./node_2050_B_study

# Represents the tree's structure
tree:
  node: "2030"
  children:
    - node: "2040"
      probability: 1.0
      children:
        - node: "2050_A"
          probability: 0.8
        - node: "2050_B"
          probability: 0.2

constraints:
  - name: add_max_1_GW_semibase
    nodes: [ "2030", "2040", "2050_A", "2050_B" ]
    candidates: [ semibase ]
    type: max_investment_per_node_per_candidate
    value: 1000
  - name: add_max_1.5GW_peak
    nodes: [ "2030", "2040", "2050_A", "2050_B" ]
    candidates: [ peak ]
    type: max_investment_per_node_per_candidate
    value: 1500
  - name: add_cum_2GW
    nodes: [ "2030", "2040" ]
    candidates: [ peak, semibase ]
    type: max_cumulative_investment_per_node
    value: 2000
  - name: forbid_retirement
    nodes: all
    candidates: all
    type: max_retirement_per_node_per_candidate
    value: 0


# Nodes' individual data
nodes:
  "2030":
    investment_date: 2030
    candidate_to_type:
      semibase: semibase_type
      peak: peak_type
  "2040":
    investment_date: 2040
    candidate_to_type:
      semibase: semibase_type
      peak: peak_type
  "2050_A":
    investment_date: 2050
    candidate_to_type:
      semibase: semibase_type
      peak: peak_type
  "2050_B":
    investment_date: 2050
    candidate_to_type:
      semibase: semibase_type
      peak: peak_type


# Candidates costs structures
candidates_types:
  semibase_type:
    investment: 5000
    operation_maintenance: 100
    retirement: 0
  peak_type:
    investment: 3500
    operation_maintenance: 100
    retirement: 0

# Initial conditions
initial_capacities:
  default: 0
  semibase: 250
```

## Candidates costs

Each candidate is associated to a type of the ```candidates_types``` section, which holds its three unitary costs. They are given **undiscounted** and **in euros per MW** : the probability of the node and the discounting are applied by the translator (see [the objective's coefficients](./index.md#switching-to-a-pluriannual-vision)).

- ```investment``` : cost of building 1 MW, paid once when the capacity enters into service.
- ```operation_maintenance``` : fixed operation and maintenance cost of 1 MW, **per year**, paid every year of the period during which the capacity is available.
- ```retirement``` : cost of decommissioning 1 MW, paid once when the capacity is removed.

## Input file parser & translator

- Parses the user input file and verifies that the data given in the file matches with the studies (To be implemented :
  check that every candidate is present in both the study and the node's info).
- Computes the relevant data :
    - Node duration
    - Node complete probability
    - Node weight
    - Discounted investment, retirement and operational costs
- Translates the constraints to their mathematical formulation (see
  the [trajectory constraints section](./merge-master.md#trajectory-constraints) of the master merger.)
- Formats and outputs the ```master_merger_info_file.json``` used as input in both
  the [merged master problem generator](./merge-master.md) and [merged weights file generator](./merge-weights.md).

## A note on candidates

We recommand that each candidate is defined in all of the studies and adding the necessary constraints to restrict the
investment / divestment behaviour for this candidate to the desired feasible space.  
However, it is still possible to have candidates not present in all of the nodes of the tree.
A candidate can either :

- Appear only in later studies, to represent a cluster that might only enter into service later. In this case, when the
  candidate $i$ first appear in node $n$, the constraint will
  be : $\text{initial_capacities}_i + dx_{i,n}^+ - dx_{i,n}^- = x_{i,n}$, regardless of wether $n$ is the root node. (
  Though as noted above, the recommanded modelisation would be to add the candidate in all the nodes and impose its
  installed capacity to be $0$ during the construction period)

- Disappear in the final studies, to represent a cluster that is assuredly decommissioned at this point.
  In this case, when the candidate $i$ disappears at node $n$, the constraint will
  be : $x_{i, \text{parent}(n)} + dx_{i,n}^+ - dx_{i,n}^- = 0$.

## Trajectory constraints translation

The trajectory constraints translator implements the following types of constraints.  
We implement the ```all``` keyword for ease of readability. ```all``` will be expanded to a list containing all of the
nodes or all of the candidates depending on the context.

### ```type: max_investment_per_node_per_candidate```

Example :

```yaml
  - name: add_max_1_GW_semibase
    nodes: [ "2030", "2040", "2050_A", "2050_B" ]
    candidates: [ semibase ]
    type: max_investment_per_node_per_candidate
    value: 1000
```

This entry will result in 4 (4 = |```nodes```| $\times$ |```candidates```|) constraints in the merged problem :

- $dx_{semibase, 2030}^+ \leq 1000$,
- $dx_{semibase, 2040}^+ \leq 1000$,
- $dx_{semibase, 2050\_A}^+ \leq 1000$,
- $dx_{semibase, 2050\_B}^+ \leq 1000$

More generally, this type of input constraint entry translates to :
$$
\forall n \in \text{nodes}, \quad \forall i \in \text{candidates}, \quad dx_{i, n}^+ \leq \text{value}
$$

### ```type: max_cumulative_investment_per_node```
Example :

```yaml
  - name: add_cum_2GW
    nodes: [ "2030", "2040" ]
    candidates: [ peak, semibase ]
    type: max_cumulative_investment_per_node
    value: 2000
```

This entry will result in 2 (2 = |```nodes```|) constraints in the merged problem :

- $dx_{semibase, 2030}^+ + dx_{peak, 2030}^+ \leq 2000$,
- $dx_{semibase, 2040}^+ + dx_{peak, 2040}^+ \leq 2000$

More generally, this type of input constraint entry translates to :
$$
\forall n \in \text{nodes}, \quad \sum_{i \in \text{candidates}} dx_{i, n}^+ \leq \text{value}
$$

### ```type: max_retirement_per_node_per_candidate```

Example :

```yaml
  - name: forbid_retirement
    nodes: all
    candidates: all
    type: max_retirement_per_node_per_candidate
    value: 0
```

This entry will result in 8 (8 = |```nodes```| $\times$ |```candidates```|) constraints in the merged problem :

- $dx_{semibase, 2030}^- \leq 0, \quad dx_{peak, 2030}^- \leq 0$,
- $dx_{semibase, 2040}^- \leq 0, \quad dx_{peak, 2040}^- \leq 0$,
- $dx_{semibase, 2050\_A}^- \leq 0, \quad dx_{peak, 2050\_A}^- \leq 0$,
- $dx_{semibase, 2050\_B}^- \leq 0, \quad dx_{peak, 2050\_B}^- \leq 0$

More generally, this type of input constraint entry translates to :
$$
\forall n \in \text{nodes}, \quad \forall i \in \text{candidates}, \quad dx_{i, n}^- \leq \text{value}
$$

**Note :** as we always have $\forall n \in T, \quad \forall i \in C, \quad dx^{+/-}_{i,n} \geq 0$, regardless of the
trajectory constraints added by the user, this last example ends up forbidding any kind of retirement, thus its label.

### ```type: max_cumulative_retirement_per_node```  
Example :

```yaml
  - name: decom_cum_2GW
    nodes: [ "2030", "2040" ]
    candidates: [ peak, semibase ]
    type: max_cumulative_retirement_per_node
    value: 2000
```

This entry would result in 2 (2 = |```nodes```|) constraints in the merged problem :

- $dx_{semibase, 2030}^- + dx_{peak, 2030}^- \leq 2000$,
- $dx_{semibase, 2040}^- + dx_{peak, 2040}^- \leq 2000$

More generally, this type of input constraint entry translates to :
$$
\forall n \in \text{nodes}, \quad \sum_{i \in \text{candidates}} dx_{i, n}^- \leq \text{value}
$$

### ```type: min_investment_per_node_per_candidate```  
Example :

```yaml
  - name: min_invest_1_5GW
    nodes: [ "2030", "2040", "2050_A" ]
    candidates: [ semibase ]
    type: min_investment_per_node_per_candidate
    value: 1500
```

This entry would result in 3 (3 = |```nodes```|) constraints in the merged problem :

- $dx_{semibase, 2030}^+ \geq 1500$,
- $dx_{semibase, 2040}^+ \geq 1500$,
- $dx_{semibase, 2050\_A}^+ \geq 1500$,

More generally, this type of input constraint entry translates to :
$$
\forall n \in \text{nodes}, \quad \forall i \in \text{candidates}, \quad dx_{i, n}^+ \geq \text{value}
$$

### ```type: min_retirement_per_node_per_candidate```  
Example :

```yaml
  - name: min_decom_1GW
    nodes: [ "2030", "2040" ]
    candidates: [ peak ]
    type: min_retirement_per_node_per_candidate
    value: 1000
```

This entry would result in 2 (2 = |```nodes```|) constraints in the merged problem :

- $dx_{peak, 2030}^- \geq 1000$,
- $dx_{peak, 2040}^- \geq 1000$

More generally, this type of input constraint entry translates to :
$$
\forall n \in \text{nodes}, \quad \forall i \in \text{candidates}, \quad dx_{i, n}^- \geq \text{value}
$$

## Input data of local study or input data from `input-trajectory.yaml` ?

Each yearly study defines some input data from either `candidates.ini` and `settings.ini`. Some of this data are overriden by `input-trajectory.yaml` whereas some are reused:

- **Master formulation**: The parameter of each individual study (integer or relaxed given in `settings.ini`) is overriden by the `formulation` given in the `input-trajectory.yaml` so that the same master formulation is applied at each node of the tree.
- **Investment costs**:  In each individual study, investment costs are given in `candidates.ini`. However, in the trajectory case we need to explicitly split investment costs, fixed operation/maintenance costs and retirement costs. Therefore **candidate costs given in `input-trajectory.yaml` override the local study ones**. The candidate costs are given in **euros per MW** in the `candidates_types` section, for each candidate type and each case (investment, operation_maintenance and retirement).
!!! Note
    For each node, the list of candidates present in `candidate_to_type` must exactly match the candidates of the `candidates.ini` file of the corresponding study.

- **Investment bounds**: For each candidate, three variables are used to define the capacity, namely $x_{i,n}$ (overall capacity), $dx_{i,n}^+$ and $dx_{i,n}^-$, respectively added and removed capacity for candidate $i$ at node $n$. Bounds on the overall capacity $x_{i,n}$ come from each individual `candidates.ini` file (section `max-investment` or `max-units` combined with `unit-size`), whereas bounds on $dx$ variables come from the `constraints` section in `input-trajectory.yaml`.
- **Solver parameters**:
    - The solver to use for resolution is given in the command line, see [Execution options](../trajectory-investment/orchestration.md#execution-options).
    - The following solver parameters are set from the `settings.ini` of the root study: absolute gap, relative gap, relaxed gap, separation parameter, max iterations, log level, batch size and time limit, cut coefficient tolerance, master solution tolerance.