# Use two set of env variables for xpress in centos github action

## Accepted

## Context

Unit tests fail by not being able to load xpress licence.
Looking into it, the xpress directory is unreachable in the test step
We can see that we are looking at `/home/runner/work/antares-xpansion/antares-xpansion` but are working
in `/__w/antares-xpansion/antares-xpansion`
This difference comes from running the job in a docker container, however this difference is not expected and is
documented as bug in github runners https://github.com/actions/runner/issues/2058

## Decision

In addition to the original variables
using `${{github.workplace}}` (`/home/runner/work/antares-xpansion/antares-xpansion`), we also define new symmetrical
variables for the container environment using `$GITHUB_WORKPLACE`

## Consequences

Fix issue
