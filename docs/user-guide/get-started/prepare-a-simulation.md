# Set up an Antares-Xpansion study

## Overview

The Antares-Xpansion package is based on the Antares software and
its data format.

Antares-Xpansion is based on an already existing Antares study. Some
of the capacities of this study, usually fixed as input in the Antares
paradigm, will be optimized by the investment optimization module of the
Antares-Xpansion package.

In order to run the investment optimization module, the Antares dataset
must be enriched with - at least - two new files:

- A `candidates.ini` file which contains the definition of
  investment candidates (which capacities of the Antares study are
  expandable? at what cost? with what limits? and so on),

- A `settings.ini` file which contains the settings of the
  Antares-Xpansion algorithm.

These two files must be located in the `user/expansion/` directory of
the Antares study (see **Figure 1**). The data they contain
are neither visible nor modifiable in the Antares man-machine interface.
These two files must therefore be handcrafted.

```
antares-study
└─── input
└─── layers
└─── logs
└─── output
└─── settings
└─── user
│   └───expansion
│       │   candidates.ini
│       │   settings.ini
│       │   ...
```
**Figure 1** - Structure of an Antares study folder ready for an Antares-Xpansion optimization

Depending on the type of investment considered in the study, some modifications of the Antares study are necessary before filling the `candidates.ini` and `settings.ini` files.

## Prepare the Antares study

The first step to set up an Antares-Xpansion study consists in defining investment candidates.
**Candidate capacities for investment are
necessarily links from an Antares study**. Investment candidates can be generation assets, or even
flexibilities, by adopting a virtual node logic as described below.

| ![](../../assets/media/image6.png) | ![](../../assets/media/image7.png) | ![](../../assets/media/image8.png) |
| :--------------------------------: | :--------------------------------: | :----------------------------------: |
| **(a)**                               | **(b)**                               | **(c)**                               |

**Figure 3** - Configuration of the Antares study for an
investment in **(a)** transmission capacity (new line or grid
reinforcement), **(b)** generation units and **(c)** storage.

The following sections describe the Antares modelling for different types of investment:

  - [Transmission capcity between two areas](#investment-in-transmission-capacity-between-two-areas)
  - [Thermal generation capacity](#investment-in-thermal-generation-capacity)
  - [Renewable generation capacity](#investment-in-renewable-generation-capacity)
  - [Flexibility](#investment-in-flexibility)

### Investment in transmission capacity between two areas

The Antares link candidate for investment, shown in red in
**Figure 3 (a)**, is directly the interconnection for which the interest
in increasing capacity is being studied.

- In the case of the construction of a new line, a link must be added
  in the Antares study between the two involved areas.

- In the case of a grid reinforcement between two already
  interconnected areas, the link between these two areas already
  exists in the Antares study. The parameter
  [`already-installed-capacity`](candidate-definition.md#already-installed-capacity) will be used in the `candidates.ini` file to specify the capacity of the grid that already exists between the two zones. In this way, Antares-Xpansion will assess the economic interest of increasing this capacity beyond what is already installed.

### Investment in thermal generation capacity

Suppose that the thermal generation capacity subject to expansion is 
physically located in `area1` for the example in **Figure 3 (b)**. As Antares-Xpansion only performs investment through links of the study, the modelling trick consists in creating a virtual node, here `invest_semibase`, connected to the physical node `area1`. The Antares link of the investment candidate is the link between these two nodes.

The generation unit, candidate for investment, must be defined by a thermal cluster with
the following technical and economic parameters:

- It is located in the virtual node `invest_semibase`.

- Its *market bid* is equal to its *marginal cost*, which is
  the variable operating cost (in €/MWh) of the generation
  unit.

- It has an hourly availability time series
  that is **always higher than the potential** of the candidate, where by *potential* we mean the capacity correponding to the maximum investment that is allowed by the user (i.e. [`max-investment`](candidate-definition.md#max-investment) or [`max-units`](candidate-definition.md#max-units) \\( \times\\) [`unit-size`](candidate-definition.md#unit-size)): 
  
    - If the hourly
      availability time series of thermal generation are "*ready-made*” in
      Antares, then the values of the time series must be filled in such a
      way that they **are always higher**
      than the candidate's potential. 
      
    - If the times series of thermal
      generation are “*stochastic*” i.e. generated by Antares, then the
      parameters for the generation of the series must be defined so that the availability is always higher than the potential
      (*number of units* \\( \times\\) *nominal capacity* \\( >\\) *potential*, no outages
      rate).

Other cluster parameters (*pmin*, *start-up costs*, etc.) can also be
defined. However, they will only be taken into account by
Antares-Xpansion if the unit-commitment type is set to [`expansion_accurate`](settings-definition.md#uc_type).

### Investment in renewable generation capacity

Similarly as for thermal generation, the renewable
generation capacity subject to expansion, physically located for the
example in **Figure 3 (b)** in `area1`, must be moved to a virtual node
connected to the physical node `area1`. The Antares link for the investment candidate is the link between these two nodes.

For the type of renewable production of interest (wind or solar), a corresponding
production time-series (Antares wind or solar tab) must be defined in
the virtual node. The production time-series must be deterministic,
constant, and **higher than the
potential** (`max-investment`) of the candidate. The
parameters [`[in]direct-link-profile`](candidate-definition.md#indirect-link-profile) from the `candidates.ini` file will then be used to define the
hourly load factor.

### Investment in flexibility

The modeling of flexibilities, such as pumped storage, is generally based in Antares
on a set of virtual nodes/links and coupling constraints. To make
flexibility an investment candidate, a link must be identified in the
Antares modelling whose transmission capacity corresponds to the
capacity of the flexibility (e.g. its maximum power or the size
of a stock for example). 

In the case of pumped storage in
**Figure 3 (c)**, the capacity of the pumped storage (equal to its pumping
and turbining capacity) is defined by the maximum possible flow on the
link between `area1` and `hub`: the investment
in the flexibility will be characterized by this link. The classical binding
constraints must be added in the Antares simulation to represent the
storage: for example a negative `ROW Balance` in `psp-in`,
positive `ROW Balance` in `psp-out` and the following constraint:

![](../../assets/media/image9.png)

### Decommissioning decisions for thermal capacities

With Antares-Xpansion, it is possible to consider decommissioning decisions, the corresponding assets are referred as _decommissioning candidates_. The difference between _investment candidates_ and _decommissioning candidates_ lies in the fixed-cost annuity.

#### Fixed-cost annuity for investment candidates

The annuity of an _investment candidate_ includes the sum of:

  - Annualized investment costs,
  - Fixed annual operation and maintenance costs.

In this configuration, Antares-Xpansion makes an economic choice by comparing the sum of these costs and the reduction in variable operating
costs (mainly fuel costs and penalties associated with loss of load)
due to the new investment.

#### Fixed-cost annuity for decommissioning candidates

The annuity for _decommissioning candidates_ only includes the fixed annual
operation and maintenance costs. There is indeed no investment cost, since the decision consists only in choosing whether to maintain operation with the associated maintenance costs.

In this configuration,
Antares-Xpansion makes an economic choice by comparing the operation
and maintenance costs of a generation or transmission asset and the
savings induced on the variable costs of power system operation thanks to this asset.

The annualized investment costs are in this case considered
stranded and are not taken into account in this economic choice. The
potential of this type of candidate (i.e. its [`max-investment`](candidate-definition.md#max-investment) or [`max-units`](candidate-definition.md#max-units) \\( \times\\) [`unit-size`](candidate-definition.md#unit-size)) corresponds to its decommissionable capacity,
or in other words, the candidate's already installed capacity that could be shut
down if it is no longer profitable for the power system.

Antares-Xpansion is not able to decommission generation units that are installed in the Antares study (i.e. located in "physical nodes"). However, we can use a modelling of _decommissioning candidates_ with the same virtual node logic as the [investment in thermal generation capacity](#investment-in-thermal-generation-capacity). Decommissioning candidates are existing physical facilities that should be moved to a virtual node. 

For example, suppose that we aim at taking a decommissioning decision for thermal generation capacities that are physically located in `area1` of **Figure 3 (b)**. To be considered _decommissioning candidates_, these generation units must be moved to a virtual node (`invest_semibase` in **Figure 3 (b)**) with an hourly availability time series higher than their potential. 

The decommissioning decision is made in Antares-Xpansion through the capacity of the link between `area1` and `invest_semibase`. Thus, the capacity invested by Antares-Xpansion on the link corresponds to the capacity that is **not** decommissioned.

Details on how to fill in the file `candidates.ini` for _decommissioning candidates_ are given in the [next part](candidate-definition.md#decommissioning-candidates).

### Additional characteristics for links of investment candidates

In the four aforementioned cases, the link used to define investment
candidates (in red in **Figure 3**):

- must have the parameter  `transmission capacities = use
  transmission capacities`, and not `set to null` or `set to
  infinite`,

- may have a *hurdle cost*, which will then be well taken into account
  in the economic optimization of Antares-Xpansion,

- may be subject to binding constraints - provided that the Antares
  version used is at least v6.1.3 - which will be well taken into
  account in the simulations of system operation. These constraints
  can possibly be constructed by the Kirchhoff constraint generator
  and the information given in the impedances, loop flow and phase
  shift columns of the link.

The direct and indirect transmission capacities of the link will be
modified by Antares-Xpansion. The values initially entered in the
*Trans.* *Capacity Direct* and *Trans. Capacity Indirect* columns do not
matter since they will be overwritten when the expansion problem is
solved. Note that the capacities of existing structures must be filled
in with the [`already-installed-capacity`](candidate-definition.md#already-installed-capacity) parameter in the `candidates.ini`
file and not in the definition of the links in the Antares
study.