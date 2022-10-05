# Use ADR to track decisions

## Status

**[2022/07/27] Accepted**

## Context

In time, the reasons behind some decision is lost and developers don't know if they have to live with the result for a
good reason or may take a different path. Finding the original reason may prove difficult and time consuming.

## Decision

Put in place Architecture Decision Records (ADR) to keep the history of major decisions and choice on the project.

Template is available in antares-xpansion/conception/Architecture_decision_records folder

All ADR must be stored in antares-xpansion/conception/Architecture_decision_records folder. Organisation inside this folder is free

What must be documented by ADR is left to the team howhever we can encourage to do it in the following cases:
* Adding library or other dependencies
* Introducing architecture patterns
* Introducing or changing tools (like unit test frameworks)
* Architecture decision around features

## Consequences

Every major or impactful choice should be documented. Documentation doesn't need to be heavy, just clear enough so that
we don't waste too much time trying to know the reasons behind a decision.
