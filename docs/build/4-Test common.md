# Instructions
[Supported OS](#supported-os) | [C++ version](#c-version) | [CMake version](#cmake-version) | [Python version](#python-version) | [Git version](#git-version) | [Dependencies](#dependencies) | [Tests](#tests) | [Installer creation](#installer-creation)

## C/I status
 [![Status][ubuntu_system_svg]][ubuntu_system_link] [![Status][windows_vcpkg_svg]][windows_vcpkg_link] [![Status][centos_system_svg]][centos_system_link]

## [CMake version](#cmake-version)
CMake 3.x must be used. 

=== "Windows"
    You can download latest Windows version directly from [CMake website](https://cmake.org/download/).
=== "Centos"
    ```
    sudo yum install epel-release
    sudo yum install cmake3
    ```
=== "Ubuntu"
    ```
    sudo apt install cmake
    ```

## [Python version](#python-version)
Python 3.x must be used.

=== "Windows"
    You can download latest Windows version directly from [Python website](https://www.python.org/downloads/windows/).
=== "Centos"
    ```
    sudo yum install python3 python3-pip
    ```
=== "Ubuntu"
    ```
    sudo apt-get install python3 python3-pip
    ```

Required python modules can be installed with :
```
pip install -r requirements-tests.txt
```

## [Dependencies](#dependencies)
*antares-xpansion* depends on several mandatory libraries. 

 - [JsonCpp](https://github.com/open-source-parsers/jsoncpp)
 - [Google Test](https://github.com/google/googletest)
 - [OR-Tools](https://github.com/AntaresSimulatorTeam/or-tools/tree/rte_dev_sirius)
 - Boost : mpi serialization (Only for MPI benders compilation), program-options
 - [Doxygen](https://www.doxygen.nl/index.html) for documentation generation
 - [GraphViz](https://graphviz.org/) for doxygen use

This section describes the install procedures for the third-party Open source libraries used by *antares-xpansion*.
The install procedure can be done

- by compiling the sources after cloning the official git repository
- by using a package manager

=== "Windows (VCPKG)"    
    For Windows we will use [vcpkg](https://github.com/microsoft/vcpkg) to download and compile the libraries. vcpkg is available as a submodule in antares-xpansion.
    
    You must install the corresponding [vcpkg-triplet](https://vcpkg.readthedocs.io/en/latest/users/integration/#triplet-selection) depending on Antares version and libraries load:
    
    - ``x64-windows``        : 64 bits version with dynamic libraries load
    - ``x86-windows``        : 32 bits version with dynamic libraries load
    - ``x64-windows-static`` : 64 bits version with static libraries load
    - ``x86-windows-static`` : 32 bits version with static libraries load
    
    The vcpkg-triplet used will be named [vcpg-triplet] later in this document.
    
    - Init submodule and install vcpkg 
    
    ```
    git submodule update --init vcpkg
    cd vcpkg
    .\bootstrap-vcpkg.bat
    ```
    
    Note :
    > all vcpkg command further described must be run from vcpkg folder. This folder will be named [vcpkg_root] later in this document.
    
    
    - Install dependencies
    ```
    cd vcpkg
    vcpkg install jsoncpp gtest boost-mpi boost-program-options --triplet [vcpg-triplet] 
    ```
    
    Note :
    > On boost-mpi compilation depends on MSMPI redistributable package. Please follow VCPKG procedure :
    > ```
    > Please install the MSMPI redistributable package before trying to install this port.
    >     The appropriate installer has been downloaded to:
    >      [vcpkg_root]/downloads/msmpisetup-10.0.12498.exe
    > ``` 

=== "Centos (yum)"   
    ```
    sudo yum install jsoncpp-devel gtest-devel openmpi-devel boost-openmpi-devel boost-program-options doxygen graphviz redhat-lsb-core
    sudo yum install openssl-devel curl-devel libuuid-devel
    ```
    
    Some external repositories must be enabled
    
    === "Centos 7 (EPEL)"
    
        ``` 
        sudo yum install epel-release
        ```
    
    === "Centos 8 (PowerTools)"
    
        ```
        sudo yum install dnf-plugins-core
        sudo yum config-manager --set-enabled PowerTools
        ```

=== "Ubuntu (apt-get)"

    ```
    sudo apt-get install lsb-release libjsoncpp-dev libgtest-dev libboost-mpi-dev doxygen graphviz libboost-program-options-dev
    sudo apt-get install unzip uuid-dev libcurl4-openssl-dev libssl-dev
    ```
    Note :
    > Depending on Ubuntu version you might need to compile google test :
    > ```
    > cd /usr/src/googletest/
    > sudo cmake .
    > sudo cmake --build . --target install
    > ```

### Automatic libraries compilation from git
[Antares dependencies compilation repository](https://github.com/AntaresSimulatorTeam/antares-deps) is used as a git submodule for automatic libraries compilation from git.

ALL dependency can be built at configure time using the option `-DBUILD_ALL=ON` (`OFF` by default). For a list of available option see [Antares dependencies compilation repository](https://github.com/AntaresSimulatorTeam/antares-deps).

Some dependencies can't be installed with a package manager. They can be built at configure step with a cmake option  : `-DBUILD_not_system=ON` (`OFF` by default):
```
cmake -B _build -S . -DCMAKE_BUILD_TYPE=Release -DUSE_SEQUENTIAL=true -DUSE_MPI=true -DBUILD_not_system=ON -DDEPS_INSTALL_DIR=<deps_install_dir>
```

**Warning :**
> boost-mpi is not compiled with this repository. On windows, VCPKG use is mandatory or you must compile boost-mpi by yourself.

#### Defining dependency install directory
When using multiple directories for antares development with multiple branches it can be useful to have a common dependency install directory.

Dependency install directory can be specified with `DEPS_INSTALL_DIR`. By default install directory is `<antares_xpansion_checkout_dir>/../rte-antares-deps-<build_type>`

Note :
> `DEPS_INSTALL_DIR` is added to `CMAKE_PREFIX_PATH`

### Pre-compiled libraries download : release version only
You can download pre-compiled antares-deps archive from [Antares dependencies compilation repository][antares-deps-url]. Only release versions are available. 

Note:
> For windows, you must you use a MSVC version compatible with MSVC version used in GitHub Action.

## [Building](#building)

- On Centos enable `devtoolset-7` and load mpi module:
```
scl enable devtoolset-7 bash
module load mpi
```

- Update git submodule for dependency build :
```
git submodule update --init antares-deps
```
- Configure build with CMake
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

- Build
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

## [Tests](#tests)

Tests compilation  can be enabled at configure time using the option `-DBUILD_TESTING=ON` (`OFF` by default)

After build, tests can be run with ``ctest`` :

```
cd _build
ctest -C Release --output-on-failure
```

All tests are associated to a label and multiple labels can be defined. You can choose which tests will be executed at ctest run.

This is the list of the available labels :

| Name     | Label |Description |
|:-------|-----|-----|
| `unit_tests`  | `unit`  | Unit test for OR-Tools use and lpnamer|
| `unit_logger`  | `unit`  | Unit test for logger use|
| `unit_launcher`  | `unit`  |Unit test antares-xpansion python launcher|
| `examples_medium`  | `medium`  |End to end tests with examples antares study (medium duration)|
| `examples_long`  | `long`  |End to end tests with examples antares study (long duration)|
| `benders_end_to_end`  | `benders`  |End to end tests for benders optimization|

Note :
> Use `ctest -N` to see all available tests

Here is an example for running only examples_medium tests (use of `Name` with `-R` option):

```
ctest -C Release --output-on-failure -R examples_medium
```

Here is an example for running only units tests (use of `Label` with `-L` option):

```
ctest -C Release --output-on-failure -L unit
```

To run all test, don't indicate any label or name:

```
ctest -C Release --output-on-failure
```

## [Installer creation](#installer-creation)
CPack can be used to create the installer after the build phase :

=== "Windows"
    ```
    cd _build
    cpack -G ZIP
    ```
=== "Centos"
    ### RHEL .rpm (Experimental)
    ```
    cd _build
    cpack3 -G RPM .
    ```
    Note :
    > `rpm-build` must be installed for RPM creation :  `sudo yum install rpm-build`
    
    ### Linux .tar.gz
    ```
    cd _build
    cpack3 -G TGZ
    ```
    
    ### Required system libraries
    There are still some system libraries that must be installed if you want to use *antares-xpansion*:
    
    ```
    sudo yum install epel-release
    sudo yum install openmpi jsoncpp boost-openmpi
    ```
    
    Before launching *antares-xpansion* with mpi for parallel launch (method `mpibenders`), you must load mpi module :
    ```
    scl enable devtoolset-7 bash
    module load mpi
    ```
    
    Note :
    > `mpirun` can't be used as root on Centos7. Be sure to launch antares-xpansion without root user.

=== "Ubuntu"
    ### Ubuntu .deb (Experimental)
    ```
    cd _build
    cpack -G DEB .
    ```
    
    ### Linux .tar.gz
    ```
    cd _build
    cpack -G TGZ
    ```
    
    ### Required system libraries
    There are still some system libraries that must be installed if you want to use *antares-xpansion*:
    ```
    sudo apt-get install libcurl4 libjsoncpp1 libboost-mpi-dev
    ```
[ubuntu_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/linux-system.yml/badge.svg
[ubuntu_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/linux-system.yml
[windows_vcpkg_svg]: https://github.com/AntaresSimulatorTeam/Antares_Simulator/workflows/Windows%20CI%20(VCPKG)/badge.svg
[windows_vcpkg_link]: https://github.com/AntaresSimulatorTeam/Antares_Simulator/actions?query=workflow%3A"Windows%20CI%20(VCPKG)"
[centos_system_svg]: https://github.com/AntaresSimulatorTeam/Antares_Simulator/workflows/Centos7%20CI%20(system%20libs)/badge.svg
[centos_system_link]: https://github.com/AntaresSimulatorTeam/Antares_Simulator/actions?query=workflow%3A"Centos7%20CI%20(system%20libs)"
[antares-deps-url]: https://github.com/AntaresSimulatorTeam/antares-deps/releases/tag/v2.0.0-rc2
