# Cache problems during Benders

## Accepted [31 Mar. 2025]

## Context

Benders builds all problems. Workers are assigned problems to work on. On a single machine or node and for large studies
with lots of problems we can hit memory limitation.
We propose a mode "cache" where problems are not pre-loaded but loaded on demand.

# Decision

Create new mode "Cache" where problems are not pre-loaded but loaded on demand.

# Consequence

* No change by default
* New mode "cache"
    * Load problem on demand
    * Reduce memory peak
    * Increase cpu time: I/O to read the problem and time to construct the problem
