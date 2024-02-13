# Change Xpansion nominal case to use Antares as a library

## Accepted [09 Feb 2024]

## Context

The current nominal case is to execute Antares which output MPS files and read them in subsequent xpansion component (
lpnamer).
Having to write files to reread them is a performance pitfall.
A way to address this is to have Antares work as lib linked by Xpansion components. This way problems data can be shared
through API calls and common interface or data structures.

## Decision

- Make a lib of Antares and expose an API to retrieve problems data
- Link to this API in Xpansion and retrieve data through said API instead of reading files
- Handle this case as the nominal case for an Xpansion execution, mainly with Launcher.py

## Consequences

- No intermediate files written between Antares and Xpansion
- Improve performance in data transmission between component
- Antares' simulation performed in Xpansion executable. Could lead to decrease performance with MPI if an MPI process
  is running in only one core. Meaning Antares simulation will not be or poorly parallelized.
