# Use of MPI versus multithreading

## Status: Accepted [2022/05/08]

Retroactive documentation

## Context

Study resolution takes a really long time (measured in hours or days). This resolution can and needs to be parallelized but several technologies exist.
* Multithreading: allows execution of several threads to perform tasks in parallel. Having several CPU cores allows for that much concurrent busy threads.
    * Data sharing between threads is fast
    * Native to language
* MPI: Spawns a copy of the process.
    * Messaging between process has a non-negligible cost
    * Can spawn several processes on different machines or nodes of a cluster

## Decision

Since studies in production are solved on the cluster called Calin, using MPI allows to spawn as many processes as there are nodes available.
Hybrid mode (mixing MPI and multithreading) is not considered for now

## Consequences

Execution in production on cluster uses MPI parallelization
