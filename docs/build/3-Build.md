# Build
Before build, make sure that dependencies are installed (see [here](2-Dependencies-install.md) for more informations)

## Environnement settings
On Centos enable `devtoolset-7` and load mpi module:
```
scl enable devtoolset-7 bash
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

Here is a list of available CMake configure option :

|Option | Default|Description |
|:-------|-------|-------|
|`CMAKE_BUILD_TYPE` |`Release`| Define build type. Available values are `Release` and `Debug`  |
|`USE_SEQUENTIAL`|`ON`|Enable build of sequential executable|
|`USE_MPI`|`OFF`|Enable build of bendersmpi executable |
|`DBUILD_antares_solver`|`ON`|Enable build of antares-solver|
|`BUILD_not_system`|`ON`|Enable build of external librairies not available on system package manager|
|`BUILD_ALL`|`ON`|Enable build of ALL external librairies|
|`DEPS_INSTALL_DIR`|`../rte-antares-deps-<CMAKE_BUILD_TYPE>`|Define dependencies libraries install directory|
|`BUILD_TESTING`|`OFF`|Enable test build|

Additionnal options for windows

|Option |Description |
|:-------|-------|
|`CMAKE_TOOLCHAIN_FILE`|Define vcpkg toolchain file |
|`VCPKG_TARGET_TRIPLET`|Define [vcpkg-triplet] |

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
    cmake3 --build _build --config Release -j8
    ```
Note :
> Compilation can be done on several processor with ```-j``` option.
