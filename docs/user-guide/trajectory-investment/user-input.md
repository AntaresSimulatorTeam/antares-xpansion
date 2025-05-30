# User input file parsing

## User input file

We give here an example of a user input file corresponding with the illustration given in the problem presentation.  
**user-input.yaml** :

```yaml
# Global trajectory data
global:
  discount_rate: 0.064
  first_investment_year: 2030
  end_of_horizon: 2060
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
  - name : add_max_1_GW_semibase
    nodes: ["2030", "2040", "2050_A", "2050_B"]
    candidates: [semibase]
    type: max_investment_per_node_per_candidate
    value: 1000
  - name : add_max_1.5GW_peak
    nodes: ["2030", "2040", "2050_A", "2050_B"]
    candidates : [peak]
    type: max_investment_per_node_per_candidate
    value: 1500
  - name : add_cum_2GW
    nodes: ["2030", "2040"]
    candidates: [peak, semibase]
    type: max_cumulative_investment_per_node
    value: 2000
  - name : forbid_retirement
    nodes: all
    candidates: all
    type: max_retirement_per_node_per_candidate
    value : 0


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

## Input file parser & translator

usage : TBA

- Parses the user input file and verifies that the data given in the file matches with the studies (To be implemented : check that every candidate is present in both the study and the node's info).
- Computes the relevant data :
    - Node duration
    - Node complete probability
    - Node weight
    - Discounted investment, retirement and operational costs
- Translates the constraints to their mathematical formulation (see  the [trajectory constraints section](./merge-master.md#trajectory-constraints) of the master merger.)
- Formats and outputs the ```master_structure_file.json``` used as input in both the [merged master problem generator](./merge-master.md) and [merged weights file generator](./merge-weights.md).


## Trajectory constraints translation

The trajectory constraints translator implements 3 types of constraints for now.  
We implement the ```all``` keyword for ease of readability. ```all``` will be expanded to a list containing all of the nodes or all of the candidates depending on the context.  

### ```type: max_investment_per_node_per_candidate```  
Example : 
```yaml
  - name : add_max_1_GW_semibase
    nodes: ["2030", "2040", "2050_A", "2050_B"]
    candidates: [semibase]
    type: max_investment_per_node_per_candidate
    value: 1000
```
This entry will result in 4 (4 = |```nodes```| $\times$ |```candidates```|) constraints in the merged problem :  
    - $dx_{2030, semibase}^+ \leq 1000$,  
    - $dx_{2040, semibase}^+ \leq 1000$,  
    - $dx_{2050\_A, semibase}^+ \leq 1000$,  
    - $dx_{2050\_B, semibase}^+ \leq 1000$

More generally, this type of input constraint entry translates to :
$$
\forall n \in \text{nodes}, \quad \forall c \in \text{candidates}, \quad dx_{n, c}^+ \leq \text{value}
$$

### ```type: max_cumulative_investment_per_node```  
Example :
```yaml
  - name : add_cum_2GW
    nodes: ["2030", "2040"]
    candidates: [peak, semibase]
    type: max_cumulative_investment_per_node
    value: 2000
```
This entry will result in 2 (2 = |```nodes```|) constraints in the merged problem :
    - $dx_{2030, semibase}^+ + dx_{2030, peak}^+ \leq 2000$
    - $dx_{2040, semibase}^+ + dx_{2040, peak}^+ \leq 2000$

More generally, this type of input constraint entry translates to :
$$
\forall n \in \text{nodes}, \quad \sum_{c \in \text{candidates}} dx_{n, c}^+ \leq \text{value}
$$

### ```type: max_retirement_per_node_per_candidate```  
Example :
```yaml
  - name : forbid_retirement
    nodes: all
    candidates: all
    type: max_retirement_per_node_per_candidate
    value : 0
```
This entry will result in 8 (8 = |```nodes```| $\times$ |```candidates```|) constraints in the merged problem :  
    - $dx_{2030, semibase}^- \leq 0, \quad dx_{2030, peak}^- \leq 0$,  
    - $dx_{2040, semibase}^- \leq 0, \quad dx_{2040, peak}^- \leq 0$,
    - $dx_{2050\_A, semibase}^- \leq 0, \quad dx_{2050\_A, peak}^- \leq 0$,
    - $dx_{2050\_B, semibase}^- \leq 0, \quad dx_{2050\_B, peak}^- \leq 0$

More generally, this type of input constraint entry translates to :
$$
\forall n \in \text{nodes}, \quad \forall c \in \text{candidates}, \quad dx_{n, c}^- \leq \text{value}
$$