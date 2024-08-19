# Change solver's basis format to avoid undefined behavior (stack overflow)

## Status: Accepted [2022/10/07]

Accepted as the format change from *extra accuracy* to *normal* remains acceptable.
## Context
Writing basis for the master problem with *extra accuracy* (i.e. parameter *format = 1* of function *ClpSimplex::writeBasis* called from  *SolverCbc::write_basis* and *SolverClp::write_basis*) can lead to undefined behavior, especially on Windows.
In fact the implementation of *ClpSimplex::writeBasis* is a call to *ClpSimplexOther::writeBasis* which declares an array of char with the wrong size (**char number[20]**). This variable is sent to another function that expects a larger array (*CoinConvertDouble (int section, int formatType, double value, **char outputValue[24]**)*). If *format=1* then *CoinConvertDouble* attempt to *memset* 24 bytes of variable **number** which has only 20 then the behavior of [memset](https://en.cppreference.com/w/cpp/string/byte/memset) is undefined. 

There is no overflow if *normal format* is chosen.
## Decision

We are proposing to change the basis format to normal.

## Consequences

The accuracy of master basis is lower, but acceptable.
