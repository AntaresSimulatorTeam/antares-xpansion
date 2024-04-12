# [Windows package] Export Mpi alongside Xpansion


## Status

Accepted (2024/04/12)

## Context

Xpansion packages are not standalone, some dependencies must be pre-installed on the running machines ex MPI. 

## Decision

The decision was taken to release a portable Xpansion Package on Windows only.
Such a thing is not desired and not even recommended on Linux dist, as Mpi rely on the host machines configuration.

## Consequences

A ready-to-use Xpansion package on Windows  