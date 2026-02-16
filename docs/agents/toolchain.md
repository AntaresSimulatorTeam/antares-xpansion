# Toolchain and platform requirements

Core requirements:
- C++20 compiler. On Linux, GCC >= 11 is required.
- CMake 3.x.
- Python 3.x.
- Git >= 2.15.

MPI toolchain note (Linux):
- mpicxx relies on /usr/bin/c++ and must point to a C++20-capable compiler.
- If /usr/bin/c++ targets an older compiler, adjust PATH or the symlink.

Platform notes:
- Windows builds are tested with Visual Studio 2022.
- CentOS 7 is EoL; prefer a more recent distribution such as Oracle Linux 8.

