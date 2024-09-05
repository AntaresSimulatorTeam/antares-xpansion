# Keep file mode support in problem generation

## Accepted [09 Feb 2024]

## Context

Given that the new nominal case is [to use Antares as a library](Change_xpansion_nomila_case_to_use_simulator_lib.md) we
could remove
support for file mode in problem generation. However, it can be useful to keep this mode for debugging purpose where we
want to compare problems from Antares (written as files) and debug lpnamer. One case particularly would be if
Antares simulation
takes a lot of time, in this case we may want to write mps files to prevent rerunning Antares simulation each time

## Decision

Keep support for file mode in lpnamer

## Consequences

- Keep the possibility to debug lpnamer using mps file
