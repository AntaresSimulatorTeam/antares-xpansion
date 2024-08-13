# Split MPS file in one common part and several individual differences

## Status: Rejected [2022/08/05]

Rejected as not technically viable

## Context

MPS files used to communicate between Antares simulator and Antares xpansion take a lot of disk space. They also take a
lot of space during xpansion between _problem_generation_ and _benders_ phases.
MPS files are vastly similar. They share a common part describing the left-hand side of problems and differ on the part describing the right-hand side of problems

## Decision

We are proposing to create a single file for the common part and keep several parts containing only the diverging part.

## Consequences

MPS files formatted this way are not considered valid by solvers. We would need to develop parsers mostly reimplementing MPS parsing which would be a 
misuse of resources, or we would reconstruct valid MPS files thus defeating the goal.
Given the drawback of the proposition it is rejected. Other possibilities will be studied instead.
