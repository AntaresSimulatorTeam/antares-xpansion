# Use a Dependency Injection Container

## Accepted [13 Mar. 2025]

## Context

Some dependencies are used in multiple places, or even everywhere like loggers. If globals should be avoided having to
pass a logger as a parameter of each and every object or function is cumbersome.
A way to declare a global object is to use a singleton. Singleton have the downside to be hard to use ins tests.
A Dependency Injection Container (DIC) is a way to declare a global object that can be replaced in tests.

## Decision

Use a Dependency Injection Container to declare global objects.

## Consequences

The code will be easier to read and maintain. The tests will be easier to write and maintain.