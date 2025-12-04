# Enforcing GCC >= 11

## Status

Accepted

## Context

Antares Xpansion depends on Intel oneTBB for parallelization. oneTBB requires C++20 and libstdc++11 (the C++ standard
library with C++11 support).

GCC versions prior to 11.0 are compliant with legacyt TBB API, not oneTBB API. This causes compilation failures and
linker errors (particularly related to TBB headers such as
`tbb/blocked_range.h`).

Additionally, using an older GCC version with modern TBB versions creates compatibility issues and unpredictable
behavior at runtime due to ABI mismatches.

## Decision

We enforce a minimum GCC version of 11 across the Antares Xpansion project:

- CMake will fail configuration with a clear error message if GCC version is less than 11.0.
- The check is performed early in the CMake configuration process (after setting C++20 standard).
- Documentation has been updated to guide users on installing GCC >= 11 on various platforms (Ubuntu, CentOS).
- For users with older systems, guidance on using Software Collection `devtoolset-11` or alternative toolchains is
  provided.

## Consequences

### Positive

- Ensures all builds use a compatible compiler with full C++20 and libstdc++11 support.
- Prevents cryptic linker errors and runtime issues caused by GCC/TBB incompatibilities.
- Reduces support burden for debugging compiler-related issues.

### Negative

- May limit development on certain legacy systems unless they upgrade their toolchain.
- Requires documentation and clear user guidance to avoid build failures on first-time setup.

### Mitigation

- Provide clear error messages with guidance on obtaining GCC >= 11.

### Limitations

- It would be good to identify clangs versions that are compatible with oneTBB as well, but this is not
  currently enforced.

## Technical Details

### libstdc++ Compatibility

GCC 11 provides libstdc++11, which is the C++ Standard Library implementation required by oneTBB. Earlier GCC versions
(9 and 10) have known incompatibilities with oneTBB:

- **libstdc++ 9 and 10**: Applications using Parallel STL algorithms may fail to compile due to interface changes
  between TBB and oneTBB. Workarounds include disabling Parallel STL support by setting `PSTL_USE_PARALLEL_POLICIES=0`
  (libstdc++ 9) or `_GLIBCXX_USE_TBB_PAR_BACKEND=0` (libstdc++ 10).
- **libstdc++ 11+**: Full compatibility with oneTBB without requiring macro-level workarounds.

By enforcing GCC >= 11, we avoid these compatibility issues entirely and ensure clean, standard-compliant builds.

### Linker Considerations

On Linux, if oneTBB has been installed to a system folder (e.g., `/usr/lib64`), linker search order issues may occur.
While this does not affect program execution, explicit use of the `-L` linker option may be required to specify the
correct oneTBB library location. This is typically handled automatically by CMake's `find_package(TBB)` integration.

## References

- [Intel oneTBB Release Notes - Known Limitations](https://www.intel.com/content/www/us/en/developer/articles/release-notes/intel-oneapi-threading-building-blocks-release-notes.html)
- [StackOverflow: Why GCC 11 is Preferred for oneTBB](https://stackoverflow.com/a/67924408)
- [GCC 11 Standard Library Changelog](https://gcc.gnu.org/releases.html)
