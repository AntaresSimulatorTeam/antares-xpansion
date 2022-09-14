# Build

## Environnement settings
On Centos, enable `devtoolset-9` and `rh-git227` and load `mpi` module:
```
scl enable devtoolset-9 bash
source /opt/rh/rh-git227/enable
module load mpi
```
## Update git submodule
```
git submodule update --init antares-deps
```
## Configure build with CMake
=== "Windows"

    ```
    cmake -B _build -S . -DUSE_MPI=ON -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
    ```
=== "Centos"

    ```
    cmake3 -B _build -S . -DCMAKE_BUILD_TYPE=Release -DUSE_MPI=ON
    ```
=== "Ubuntu"

    ```
    cmake -B _build -S . -DCMAKE_BUILD_TYPE=Release -DUSE_MPI=ON
    ```

Here is a list of available CMake configure options:

| Option                     | Default                                  | Description                                                                  |
|:---------------------------|------------------------------------------|------------------------------------------------------------------------------|
| `CMAKE_BUILD_TYPE`         | `Release`                                | Define build type. Available values are `Release` and `Debug`.               |
| `USE_SEQUENTIAL`           | `ON`                                     | Enable build of sequential executable.                                       |
| `USE_MPI`                  | `OFF`                                    | Enable build of bendersmpi executable.                                       |
| `DBUILD_antares_solver`    | `OFF`                                    | Enable build of antares-solver.                                              |
| `DDOWNLOAD_antares_solver` | `ON`                                     | Enable download of antares-solver.                                           |
| `BUILD_not_system`         | `ON`                                     | Enable build of external librairies not available on system package manager. |
| `BUILD_ALL`                | `OFF`                                    | Enable build of ALL external librairies.                                     |
| `DEPS_INSTALL_DIR`         | `../rte-antares-deps-<CMAKE_BUILD_TYPE>` | Define dependencies install directory.                                       |
| `BUILD_TESTING`            | `OFF`                                    | Enable test build.                                                           |
| `BUILD_UI`                 | `OFF`                                    | Enable UI build.                                                             |
| `ALLOW_RUN_AS_ROOT`        | `OFF`                                    | allow mpi to run as root for centOs docker.                                  |

Additionnal options for Windows:

|Option |Description |
|:-------|-------|
|`CMAKE_TOOLCHAIN_FILE`|Define `vcpkg` toolchain file. |
|`VCPKG_TARGET_TRIPLET`|Define `<vcpkg-triplet>`. |

Additionnal options for Xpress use:

|Option | Default|Description |
|:-------|-------|-------|
|`XPRESS`|`OFF`| Enable Xpress support. |
|`XPRESS_ROOT`|`C:/xpressmp` on Windows, `/opt/xpressmp` on Unix. | Define Xpress installation directory. |

### Antares solver dependency
You may have noticed two options related to antares-solver: _BUILD_antares_solver_ and _DOWNLOAD_antares_solver_

In case cmake can't find _antares-solver_ package it will check if _DOWNLOAD_antares_solver_ is ON. In this case it will try 
to download the asset from [Antares Simulator repository](https://github.com/AntaresSimulatorTeam/Antares_Simulator/releases).
The asset downloaded is always a release version. The version number is the one set in antares-version.json at the root of XPansion repository

In case the package is still not found (download failed, or you didn't enable it) and if _BUILD_antares_solver_ is ON cmake will 
fetch [Antares Simulator repository](https://github.com/AntaresSimulatorTeam/Antares_Simulator/) and build it. The build type (debug|release)
depends on the build chosen for Xpansion. To build Antares Simulator you will need to have its dependency ortool
available to cmake find_package()

## Build
=== "Windows"

    ```
    cmake --build _build --config Release -j8
    ```
=== "Centos"

    ```
    cmake3 --build _build --config Release -j8
    ```
=== "Ubuntu"

    ```
    cmake --build _build --config Release -j8
    ```
!!! Note
    Compilation can be done on several processor with the `-j` option.
