# AGENTS

Antares-Xpansion performs investment simulations for Antares studies.

Package managers (non-npm):
- C++ dependencies: vcpkg (see docs/agents/dependencies.md)
- Python dependencies: pip via requirements files (see docs/agents/dependencies.md)

Non-standard build/test commands:
- Configure/build with CMake and vcpkg (see docs/agents/build.md)
- Tests are run via ctest, enable with BUILD_TESTING (see docs/agents/tests.md)

Global notes:
- Codebase is C++20 with a Python runner; build uses CMake (details in docs/agents/toolchain.md)

Further instructions:
- docs/agents/README.md
- docs/agents/build.md
- docs/agents/tests.md
- docs/agents/dependencies.md
- docs/agents/toolchain.md

