# Remove archive support in Launcher

## Accepted [09 Feb 2024] (Partial)

## Context

Launcher (python components) is becoming more and more complex.
Given that Launcher is used for the Xpansion business case
(by opposition to R&D) and
that [the nominal case is to use Antares as a lib](%5B012%5D%20Change_xpansion_nomila_case_to_use_simulator_lib.md)
it is not necessary to support archive mode in Launcher

## Decision

- Remove archive mode in launcher

Note: 20 Feb. 2024: awaiting full readiness of study/API mode, archive mode is maintained in the meantime

## Consequences

Simplify and clarify Launcher code and usage
