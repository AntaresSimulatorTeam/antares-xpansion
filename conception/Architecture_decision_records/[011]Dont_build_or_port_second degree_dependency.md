# Don't build second degree dependencies

## Accepted [08/2024]

Supersede [Create custom vcpkg ports for deps](./%5B009%5D%20Create_custom_vcpkg_ports_for_deps.md)

## Context

Let's define direct dependencies as the dependencies that are directly used by the application.
Let's define second degree dependencies as the dependencies that are used by the direct dependencies but not the application itself.

Second degree dependencies management is not the responsibility of the application but of the direct dependencies.
The application should not try to get or build second degree dependencies. Requirements may change inside direct dependencies but should not impact
the application or in the least possible way.
The least possible way is nothing. Realistically, it may be a version number while retrieving a second degree dependency.

Describing ports for second degree dependencies can be confusing because it promotes them as direct dependencies.
We should stick to best practices:
* Fetch_content a direct dependency and let it manage its own dependencies
* Use a dependency manager for direct dependencies and let it manage its own dependencies
* Download pre-built binaries for direct dependencies and matching second degree dependencies as a worst case scenario.

## Decision

* Remove Sirius port. Not a direct dependency, use prebuild binaries or lean on fetch_content
* Download all second degree dependencies if possible (ortools, sirius, etc.)
* When it is easier to manage, use vcpkg ports instead of system package or prebuild binaries for second degree dependencies. Like boost.
It avoids polluting user environment.

## Consequences

Limit the number of ways we're managing dependencies
