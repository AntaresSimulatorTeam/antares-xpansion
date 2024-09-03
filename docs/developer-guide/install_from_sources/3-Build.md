# Build

## Environnement settings

On Centos, enable `devtoolset-10` and `rh-git227` and load `mpi` module:
```
scl enable devtoolset-10 bash
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
    cmake -B _build -S . -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
    ```
=== "Centos"

    ```
    cmake3 -B _build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux-release
    ```
=== "Ubuntu"

    ```
    cmake -B _build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux-release
    ```

Here is a list of available CMake configure options:

|Option | Default|Description |
|:-------|-------|-------|
|`CMAKE_BUILD_TYPE` |`Release`| Define build type. Available values are `Release` and `Debug`.  |
|`DBUILD_antares_solver`|`ON`|Enable build of antares-solver.|
|`BUILD_not_system`|`ON`|Enable build of external librairies not available on system package manager.|
|`BUILD_ALL`|`OFF`|Enable build of ALL external librairies.|
|`BUILD_TESTING`|`OFF`|Enable test build.|
|`BUILD_UI`|`OFF`|Enable UI build.|
|`ALLOW_RUN_AS_ROOT`|`OFF`|allow mpi to run as root for centOs docker.|

Additionnal vcpkg options:

|Option |Description |
|:-------|-------|
|`CMAKE_TOOLCHAIN_FILE`|Define `vcpkg` toolchain file. |
|`VCPKG_TARGET_TRIPLET`|Define `<vcpkg-triplet>`. |

Additionnal options for Xpress use:

|Option | Default|Description |
|:-------|-------|-------|
|`XPRESS`|`OFF`| Enable Xpress support. |
|`XPRESS_ROOT`|`C:/xpressmp` on Windows, `/opt/xpressmp` on Unix. | Define Xpress installation directory. |

## Build
=== "Windows"

    ```
    cmake --build _build --config Release -j8 --target install
    ```
=== "Centos"

    ```
    cmake3 --build _build --config Release -j8 --target install
    ```
=== "Ubuntu"

    ```
    cmake --build _build --config Release -j8 --target install
    ```
!!! Note
    Compilation can be done on several processor with the `-j` option.
