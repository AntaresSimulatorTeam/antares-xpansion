# Remove file support in Launcher

## Rejected [Sep 2024]

## Context

Launcher (Python components) is becoming increasingly complex. Although it is used for the Xpansion business case and the
[the nominal case is to use Antares as a lib](%5B012%5D%20Change_xpansion_nomila_case_to_use_simulator_lib.md)
, for development and R&D, it is useful to work step-by-step and see/edit files between steps.
## Decision

- Keep file mode in launcher [#922](https://github.com/AntaresSimulatorTeam/antares-xpansion/issues/922)

## Consequences

Maintain support for R&S and development cases and simplify them.
