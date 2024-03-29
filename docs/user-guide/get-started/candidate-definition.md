# Define the candidates

Once the Antares study has been prepared for the Antares-Xpansion simulation, by the introduction of virtual nodes as described in [Prepare the Antares study](prepare-a-simulation.md#prepare-the-antares-study), the user must use the `candidates.ini` file to characterize the investment candidates.

## Characterization of investment candidates in `candidates.ini`

Not all links in the Antares study are by default investment candidates.
The candidates are defined in the
`candidates.ini` file, which must be located inside the `user/expansion/` directory of the Antares study folder: `antares-study-folder/user/expansion/candidates.ini`. 

### Overview of the `candidates.ini` file

Each investment candidate is characterized
by the following properties:

- [`name`](#name) (mandatory): name of the investment candidate (:warning: must not
  contain spaces and unique),

- [`link`](#link) (mandatory): link on which there is a capacity investment,

- [`annual-cost-per-mw`](#annual-cost-per-mw) (mandatory): investment cost, per year and per MW,

- [`unit-size`](#unit-size): size, in MW, of a single investment unit (e.g. one group
  of 300 MW),

- [`max-units`](#max-units): maximum number of units that can be built,

- [`max-investment`](#max-investment): maximum capacity in MW that can be invested in the candidate,

- [`already-installed-capacity`](#already-installed-capacity): capacity in MW that is
already installed on the investment candidate's link,

- [`direct-link-profile`](#indirect-link-profile): name of a file that links the invested capacity and the available capacity for the direct way,
- [`indirect-link-profile`](#indirect-link-profile): name of a file that links the invested capacity and the available capacity for the indirect way,

- [`already-installed-direct-link-profile`](#already-installed-indirect-link-profile): name of a file that links the already installed capacity and the available capacity for the direct way,
- [`already-installed-indirect-link-profile`](#already-installed-indirect-link-profile): name of a file that links the already installed capacity and the available capacity for the indirect way.

The format is a standard `.ini` and should follow this template:

```ini
[1]
name = pv
link = area1 - pv
annual-cost-per-mw = 46000
max-investment = 1000
already-installed-capacity = 50
direct-link-profile = capa_pv.csv
indirect-direct-link-profile = capa_pv.csv
already-installed-direct-link-profile = direct_installed_capa_pv.csv
already-installed-indirect-link-profile = indirect_installed_capa_pv.csv
```

In Antares-Xpansion, the investment decisions only affect the capacity of the
Antares' links. Investment in physical generation capacity is done with virtual nodes as described in [Prepare the Antares study](prepare-a-simulation.md#prepare-the-antares-study).

### Details of the parameters

#### `name`

String, **mandatory**. Specifies the name of the
investment candidates. Antares-Xpansion uses this name
in the output and the logs.
  > :warning: This field must not contain spaces!
  
#### `link`

String, **mandatory**. Defines the link of the Antares
study candidate for investment, whose capacities (direct and indirect)
will be modified by Antares-Xpansion. The syntax of the link name
includes the names of the two Antares nodes that are connected by the link, separated by "-", for example:
```
origin_area – destination_area
```
Node names that include spaces or dashes are not compatible
with Antares-Xpansion. The origin area corresponds to the first in
the spelling order. The same link may contain several investment
candidates (see [this section](#several-investment-candidates-on-the-same-link )).

#### `annual-cost-per-mw`

Float, **mandatory**. The decimal separator is the
point. It defines the fixed cost annuity of the investment candidate (in
€/MW/year). Depending on the type of candidate,
the fixed cost annuity can include:

- Fixed operation and maintenance costs,
- An investment cost annuity.

#### `max-investment`

Float. Represents the candidate's
potential, i.e. the maximum capacity (in MW) that can be invested in
this candidate. If this parameter is set, the invested capacity can take any value in the interval \\( [0, \texttt{max-investment}]\\).

#### `unit-size`

Float. Defines the nominal capacity
(in MW) of the investment candidate's units.

#### `max-units`

Integer. Corresponds to the candidate's
potential in terms of number of installable units. If
the parameters `max-units` and `unit-size` are set, then the
invested capacity is necessarily a multiple of `unit-size` from the set:

$$
\\{0, \texttt{unit-size}, 2 \cdot \texttt{unit-size}, … , \texttt{max-units} \cdot \texttt{unit-size}\\}.
$$

!!! Warning 
    The definition of an investment candidate must necessarily include
    either (i) a maximum potential in MW (`max-investment`) or (ii) a unit
    size in MW (`unit-size`) and a maximum potential in number of units (`max-units`).

#### `already-installed-capacity`

Float. Corresponds to a capacity that is
already installed on the investment candidate's link. If
Antares-Xpansion finds that the investment in this
candidate is economically relevant, the new capacity will be added to the already installed capacity. The transmission capacities
initially indicated in the Antares study are not considered in the
`already-installed-capacity` parameter and are overwritten by
Antares-Xpansion.


#### `[in]direct-link-profile`

String, specifying the name of a file. This
file must be located in the `user/expansion/capa/` directory of the
Antares study. It must contain at least one column of 8760 numerical
values (the decimal separator is the point), see **Figure 5**. Multiple columns are used in order to use different data depending on the Monte-Carlo year in process, see details [here](#using-different-profiles-depending-on-the-monte-carlo-year).
When only one column is specified, the same profile is used by Antares-Xpansion for
all Monte-Carlo years of the Antares study and all assessed capacities.

!!! Remark
    Both files `direct-link-profile` and `indirect-link-profile` must contain the same number of columns.

The `[in]direct-link-profile` makes the
link between the invested capacity and the capacity that is actually available,
in the [in]direct directions of the Antares link, for the 8760
hours of the year. More details are given in [this section](#link-between-invested-capacity-and-capacity-of-the-antares-study).

The `[in]direct-link-profile` can be used for example to represent
the maintenance of a generation asset via a seasonalized power outage,
or the average load factor of intermittent renewable generation, defined
at hourly intervals.

File `user/expansion/candidates.ini`:
```ini
[1]
name = pv
link = area1 - pv
annual-cost-per-mw = 46000
max-investment = 1000
already-installed-capacity = 50
direct-link-profile = capa_pv.csv
indirect-direct-link-profile = capa_pv.csv
already-installed-direct-link-profile = direct_installed_capa_pv.csv
already-installed-indirect-link-profile = indirect_installed_capa_pv.csv
```
File `user/expansion/capa/capa_pv.csv`:
![](../../assets/media/link_profile.PNG)

**Figure 5** – Example of a file containing a load factor profile in
the Antares-Xpansion format.

#### `already-installed-[in]direct-link-profile`

String, specifying the name of a file. This
file must be located in the `user/expansion/capa/` directory of the
Antares study and has the same format as a `[in]direct-link-profile` file, see **Figure 5**. The `already-installed-[in]direct-link-profile` makes the link between the
already installed capacity and the available capacity, in the
direct and indirect way of the Antares link, for the 8760 hours of the
year. More details are given in [this section](#link-between-invested-capacity-and-capacity-of-the-antares-study).


!!! Note 
    The same file can be used for `[in]direct-link-profile`
    and `already-installed-[in]direct-link-profile` of one or more candidates.

## Link between invested capacity and capacity of the Antares study

!!! Note 
    For simplicity, we often refer to `[already-installed]-direct-link-profile` and `[already-installed]-indirect-link-profile` link profiles as just `[already-installed]-link-profile` in the sequel.

The parameters `link-profile`, `already-installed-capacity` and
`already-installed-link-profile` are used to define the link between the capacity installed by Antares-Xpansion, the already installed capacity and the truly available capacity in the Antares study, hour by hour and in both directions of the link, following the relationship presented in **Figure 6**.

These parameters are only useful if the candidates are grid reinforcement.

![](../../assets/media/image13.png)

**Figure 6** – Link between the capacity invested by
Antares-Xpansion, and the capacity available in the Antares study.

By default, the `link-profile` and
`already-installed-link-profile` contain only 1's, thus assuming perfect
availability of the invested capacity.

The parameters `link-profile` and `already-installed-link-profile` are conventionally used to:

- Take into account an NTC profile on an interconnection, possibly
  seasonalized and having a different impact on the direct and
  indirect directions of the link. It is possible to specify a different NTC profile for each Monte-Carlo year.

- Represent the maintenance of a thermal generation asset by
  considering a reduction of its power, possibly
  different according to the season (see **Figure 7**),

- Model renewable generation by multiplying the invested capacities by
  a load factor chronicle (e.g. an average chronicle
  or the chronicles of some Monte-Carlo years).

![](../../assets/media/image14.png)

**Figure 7** – Available hourly capacity of different types of power
plants due to outages. Antares-Xpansion allows taking into account
either an average hourly availability (purple line),
which is still very different from the actual hourly availability over a
year or a different profile for each Monte-Carlo year (green lines), more realistic.

If you use the average availability, to validate the results after running the Antares-Xpansion, it is
preferable to re-simulate these outages according to a stochastic
process by relaunching an Antares simulation with the capacities
obtained by Antares-Xpansion in order to obtain the real production
program with outages and RES intermittence varying according to the
scenarios.

## Using different profiles depending on the Monte-Carlo year

From Antares-Simulator 8.2, users can define different NTC chronicles for different Monte-Carlo years. 

Briefly, users define any number
of chronicles for each link. For each link, users can select (randomly or not) one 
of the said chronicles in a result we call _scenario_. Refer to [Antares-Simulator documentation](https://antares-simulator.readthedocs.io/en/latest/reference-guide/04-active_windows/#links) for more information.

In Antares-Xpansion, it is possible to use 
those scenarios to select a different link profile depending on the Monte-Carlo year. There are a few pre-requisites:

1. For a given candidate, if specified, `direct-link-profile`, `indirect-link-profile`, `already-installed-direct-link-profile` and `already-installed-indirect-link-profile` must have the same number of columns.
2. For a given link in the Antares-Simulator study, the number of columns for NTC chronicles must be the same as
in the link profiles and already installed profiles for the candidates on this link.
3. It is possible not to provide a link profile or already installed profile. A profile with a value of 1 is used
whichever Monte-Carlo year is chosen.

## Examples of candidates

### Basic example

An example with two investments candidates, one in semi-base generation
and one in network capacity, is given below.

![https://github.com/rte-antares-rpackage/antaresXpansion/raw/master/vignettes/example2nodes.png](../../assets/media/image12.png)

The invested semi-base generation in `area1` is shifted in the virtual
node `invest_semibase`. During the optimization process, the capacity
of the link between `area1` and `invest_semibase` will be updated with
the new invested capacity.

The `candidates.ini` file for this example is the following one:

```ini
[1]
name = semibase
link = area1 - invest_semibase
annual-cost-per-mw = 126000
unit-size = 200
max-units = 5
already_installed_capacity = 200

[2]
name = grid
link = area1 - area2
annual-cost-per-mw = 3000
unit-size = 500
max-units = 4
```

Another example with solar generation in a virtual node:

```ini
[1]
name = solar_power
link = area1 - pv1
annual-cost-per-mw = 100000
max-investment = 10000
direct-link-profile = pv1.txt
indirect-link-profile = pv1.txt
```
where `pv1.txt` is a text file, located in the `user/expansion/capa/`
folder of the study, and which contains the load factor time series of
the candidate, see **Figure 5**. When \\(x\\) MW of the
candidate `solar_power` are invested, the actual time series of
available power is equal to the product of \\(x\\) and the time series `pv1.txt`.

### Several investment candidates on the same link

The same link in an Antares study can be the support of several investment candidates. The interest of such an approach can be :

- To define several potentials with different fixed cost annuities,

- To define several investment opportunities of different unit size.

The example in **Figure 8** shows the case of an investment in photovoltaic production with three potentials of increasing cost.

![](../../assets/media/several_candidates.PNG)
**Figure 8** – Three potentials of increasing investment cost that applies to the same link in the Antares study.

This only works with the Antares-Xpansion algorithm (Benders decomposition) if the costs are increasing and if the investment candidates with the same link have the same `already-installed-capacity` and  `already-installed-link-profile`. 

### Decommissioning candidates

In this example, we show how to consider both an investment candidate and a decommissioning candidate on the same link. Candidates for decommissioning must be moved from their physical location to a virtual node as explained in [Prepare the Antares study](prepare-a-simulation.md#decommissioning-decisions-for-thermal-capacities).

We strongly advise to explicitly specify candidates for decommissioning in the `name` field of the `candidates.ini` file (although this is not required by the tool), see **Figure 9**. This will ease the analysis of Antares-Xpansion output for the user.

By considering [several investment candidates on the same link](#several-investment-candidates-on-the-same-link ), it is possible to model both an investment and a decommissioning candidate.

- The candidate for decommissioning is defined by:

    - A decommissionable capacity, that corresponds to a `max-investment` or `max-units` \\( \times\\) `unit-size`.
    - Fixed operation and maintenance costs (no investment cost) given in the field `annual-cost-per-mw`.

- The candidate for investment is defined as by:

    - An expandable capacity, that corresponds to a `max-investment` or `max-units` \\( \times\\) `unit-size`.
    - A fixed-cost annuity that includes investment costs and fixed operation and maintenance costs given in the field `annual-cost-per-mw`.

An example of production process that can be decommissioned or
expanded is given in **Figure 9**.

![](../../assets/media/image15.png)

**Figure 9** – Candidates for investment and decommissioning on
the same link from an Antares study.

!!! warning
    The hourly availability time series of thermal generation CCG
    in Antares should be higher than the sum of currently
    installed and new potential buildable capacities
    (availability of CCG cluster
    in the virtual node \\(fr_{ccg} > 330 \cdot 19 + 330 \cdot 50\\) in this example).

!!! Remark
    If the user specifies an `already-installed-capacity` for the decommissioning candidate, this capacity will **not** be decommissioned by Antares-Xpansion.

From the modelling of _decommissioning candidates_, it follows that the final result should be read "in the opposite way" compared to _investment candidates_: 

  - If the result displayed in the console for `fr_ccg_decommissioning` is 300 x 19 MW invested, this means that **no unit is decommissioned**. 
  - If the result displayed in the console for `fr_ccg_decommissioning` is 0 MW invested, this means that **all units have been decommissioned**.