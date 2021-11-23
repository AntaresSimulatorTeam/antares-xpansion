# Introduction

[Antares-Xpansion](https://antares-simulator.org/)  package works along with RTE's adequacy software Antares:
<https://antares.rte-france.com/>


Antares-Xpansion is the package which optimizes the installed
capacities of an Antares study.

Typical uses of the package are for:

- **long-term scenario building**: build an economically consistent
long-term generation mix

- **transmission expansion planning**: compute the network
development which maximizes social welfare

The investment decisions are optimized by running Antares' simulations
iteratively. At each iteration, the installed capacity of the
investments are updated, and the simulations are repeated until the
total costs have converged to a minimum. The total cost evaluated in
this problem are the sum of **the expected operation cost during one
year** and **the investment annuity**.

Antares-Xpansion is currently under development. Feel free to submit
any issue.