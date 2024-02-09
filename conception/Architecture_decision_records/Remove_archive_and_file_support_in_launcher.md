# Remove archive and file support in Launcher

## Accepted [09 Feb 2024]

## Context

Launcher (python components) is becoming more and more complexe. Given that Launcher is used for Xpansion business case
(by opposition to R&D) and
that [the nominal case is to use Antares as a lib](Change_xpansion_nomila_case_to_use_simulator_lib.md)
it is not necessary to support archive mode or file mode in Launcher

## Decision

- Remove archive mode in launcher
- Remove file mode in launcher

## Consequences

Simplify and clarify Launcher code and usage
