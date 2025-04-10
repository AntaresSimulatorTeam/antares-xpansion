# Vectorise problem creation in LPNamer

## Accepted [31 Mar. 2025]

## Context

LPNamer build all problems before processing them. When there are a lot of problems to process we can hit the limit of
memory. This is the same kind of issue encountered during Benders.
To avoid this issue we must not build all problems at once but only when necessary.

## Decision

Build problems in the loop processing them

## Consequence

* LPNamer memory peak is reduced

## Reference

* [ANT-2210](https://gopro-tickets.rte-france.com/browse/ANT-2210)