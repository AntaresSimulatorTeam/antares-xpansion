# Download Simulator assets during cmake configuration

## Status

Accepted

## Context

When installing a new environment, recovering from mistake or upgrading to a new version of antares simulator the full Simulator repository is build.
This build take a lot of time.

It is possible to manually download the release assets and extract them in _DEPS_INSTALL_DIR_. It's not inherently difficult,
but you need to remember to do it, and it must be made by hand.

## Decision

Instead of compiling Silmulator repository we propose to download the released asset.

Downloading and compiling are optional, allowing developers to choose one or the other.

## Consequences

Developers don't need to manually download Simulator assets anymore to save time and are not affected by long compilation time of simulator anymore
