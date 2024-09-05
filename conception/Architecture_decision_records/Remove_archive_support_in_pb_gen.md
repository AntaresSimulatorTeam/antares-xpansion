# Remove archive support in problem generation

## Accepted [09 Feb 2024] (Partial)

## Context

Given that the new nominal case is [to use Antares as a library](Change_xpansion_nomila_case_to_use_simulator_lib.md) we
could remove
support for archive mode in problem generation. Given that archive mode was a way to reduce the size on disk and now
memory mode
completely remove the need to use file we can remove support of archive file as "input" of problem generation or as
output

## Decision

Remove archive support in problem generation

Note: 20 Feb. 2024: awaiting full readiness of study/API mode, archive mode is maintained in the meantime

## Consequences

- Fewer cases to maintain
