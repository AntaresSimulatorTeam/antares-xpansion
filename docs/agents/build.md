# Build and configuration

Antares-Xpansion is built with CMake and vcpkg. If using pre-built dependencies, set CMAKE_PREFIX_PATH accordingly.

Configure (examples):
- Windows:
  cmake -B _build -S . -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
- CentOS:
  cmake3 -B _build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux-release
- Ubuntu:
  cmake -B _build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux-release

Common configure options:
- CMAKE_BUILD_TYPE: Release or Debug
- DBUILD_antares_solver (default ON): build antares-solver
- BUILD_not_system (default ON): build external libraries not available via system package manager
- BUILD_ALL (default OFF): build all external libraries
- BUILD_TESTING (default OFF): enable tests
- ALLOW_RUN_AS_ROOT (default OFF): allow MPI to run as root for CentOS Docker

  Pour compiler utilise :
  /home/jmarechal/miniconda3/bin/cmake --build /home/jmarechal/CLionProjects/build_xpansion_relwithdebinfo --target all -j 6

Notes:
- First vcpkg build can be long; subsequent builds are faster due to caching.
- On Ubuntu, you may need lsb-release (sudo apt install lsb-release).

