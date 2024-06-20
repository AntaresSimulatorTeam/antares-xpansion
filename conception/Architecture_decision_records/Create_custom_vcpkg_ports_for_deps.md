# Manage dependencies with custom vcpkg ports

## Status: Accepted [06/2024]


## Context

The various dependencies of the application are managed through different ways:
- some are managed through the system package manager
- some are managed through vcpkg
- some are downloaded and compiled by the application itself through cmake or by the developer

This disparity is further enhanced by the unavailability of some dependencies in packet managers for some plateforme (e.g. CentOS 7).

## Decision

We should strive to use only one methode to manage dependencies. We choose to use vcpkg for all dependencies.
In some cases, we will have to create custom vcpkg ports for dependencies that are not available in vcpkg.

## Consequences

- The dependencies will be managed in a more consistent way
- The dependencies will be centralized in one place (vcpkg manifest)
- Several dependencies moved to vcpkg via official or custom ports.

## Limitations

- Some dependencies (sirius) are not first degree dependencies but N-degree, mainly for simulator.
  While a custom port or registry of Antares Simulator is build,
we need to manage those dependencies.
- Ortools is challenging to port to vcpkg. Work in progress.