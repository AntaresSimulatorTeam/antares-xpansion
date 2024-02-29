# Decision record template by Michael Nygard

This is the template
in [Documenting architecture decisions - Michael Nygard](http://thinkrelevance.com/blog/2011/11/15/documenting-architecture-decisions).
You can use [adr-tools](https://github.com/npryce/adr-tools) for managing the ADR files.

In each ADR file, write these sections:

# Install jsoncpp library with xpansion

## Status: Accepted [28 Feb. 2024]

## Context

In the context of migrating from centOS7 to oracle linux8 it was observed that managing jsoncpp library installation on
target environment
can be difficult.

## Decision

Deliver jsoncpp library with xpansion package

## Consequences

jsoncpp does not need to be installed on the target system. No version conflict
