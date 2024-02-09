# Hold link profile values in vector instead of array

## Accepted

## Vector on Heap, arrays on Stack

Semantically it makes sens to use std::array to hold **link profile** values. However, **link profiles** hold 8760
values of type double.
_std::array_ allocate their values on the stack, _std::vector_ allocate their value on the heap.
On Windows during some tests stack allocation was overflown due to too many **link profiles** declared.
This issue could very well happen in production in other circumstances.

## Decision

Use _std::vector_ instead of _std::array_ to hold **link profiles** values

## Consequences

Prevent stack overflow to happen, at least during tests.
