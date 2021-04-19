# *antares-xpansion* CMake Build Instructions

[Build tools](#build-tools) | [Dependencies](#dependencies) | [Building](#building-antares-solution) | [Tests](#tests) | [Installer creation](#installer)

## C/I status

[![Status][ubuntu_system_svg]][ubuntu_system_link]

[ubuntu_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/linux-system.yml/badge.svg

[ubuntu_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/linux-system.yml

## [Build tools](#build-tools)

```
sudo apt install build-essential libssl-dev cmake
```

## [Python version](#python-version)
Python 3.x must be used.

```
sudo apt-get install python3 python3-pip
```

Required python modules can be installed with :
```
pip3 install -r requirements-tests.txt
```

## [Dependencies](#deps)
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
- by using a package manager. Depending on the OS we propose a solution
  - using VCPKG (Only tested on windows)
  - using the official package manager of the linux distribution


#### Ubuntu

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
> boost-mpi is not compiled with this repository.

#### Defining dependency install directory
When using multiple directories for antares development with multiple branches it can be useful to have a common dependency install directory.

Dependency install directory can be specified with `DEPS_INSTALL_DIR`. By default install directory is `<antares_xpansion_checkout_dir>/../rte-antares-deps-<build_type>`

Note :
> `DEPS_INSTALL_DIR` is added to `CMAKE_PREFIX_PATH`

### Pre-compiled libraries download : release version only
You can download pre-compiled antares-deps archive from [Antares dependencies compilation repository](https://github.com/AntaresSimulatorTeam/antares-deps/releases/tag/v1.1.0). Only release versions are available.

There are still some system libraries that must be installed :

```
sudo apt-get install libuuid1 uuid-dev libssh2-1 libssh2-1-dev libidn2-0 libidn2-dev libidn11 libidn11-dev gtk2.0 libb64-dev libjpeg-dev libtiff-dev libsecret-1-dev
```
## [Building *antares-xpansion* V2](#build)
- update git submodule for dependency build :
```
git submodule update --init antares-deps
```

- Configure build with cmake

Here is a list of available CMake configure option :

|Option | Description |
|:-------|-------|
|`CMAKE_BUILD_TYPE` | Define build type. Available values are `Release` and `Debug`  |
|`USE_SEQUENTIAL`|Enable build of sequential executable (default `OFF`)|
|`USE_MPI`|Enable build of bendersmpi executable (default `OFF`)|
|`DBUILD_antares_solver`|Enable build of antares-solver (default `ON`)|
|`BUILD_not_system`|Enable build of external librairies not available on system package manager (default `ON`)|
|`BUILD_ALL`|Enable build of ALL external librairies (default `OFF`)|
|`DEPS_INSTALL_DIR`|Define dependencies libraries install directory|
|`BUILD_TESTING`| Enable test build (default `OFF`)| 


```
cmake -B _build -S . -DCMAKE_BUILD_TYPE=Release -DUSE_SEQUENTIAL=true -DUSE_MPI=true
```
- Build
 ```
cmake --build _build --config Release -j8
```
Note :
>Compilation can be done on several processor with ```-j``` option.

## [Tests](#tests)

Tests compilation can be enabled at configure time using the option `-DBUILD_TESTING=ON` (`OFF` by default)

After build, tests can be run with ``ctest`` :
 ```
cd _build
ctest -C Release --output-on-failure
```
All tests are associated to a label and multiple labels can be defined. You can choose which tests will be executed at ctest run.

This is the list of the available labels :

| Name     | Label |Description |
|:-------|-----|-----|
| `unit_ortools`  | `unit`  | Unit test for OR-Tools use|
| `unit_launcher`  | `unit`  |Unit test antares-xpansion python launcher|
| `examples_medium`  | `medium`  |End to end tests with examples antares study (medium duration)|
| `examples_long`  | `long`  |End to end tests with examples antares study (long duration)|
| `benders_end_to_end`  | `benders`  |End to end tests for benders optimization|
Note :
> Use `ctest -N` to see all available tests

Here is an example for running only example tests (use of `Name` with `-R` option):
```
ctest -C Release --output-on-failure -R example
```` 
Here is an example for running only units tests (use of `Label` with `-L` option):
```
ctest -C Release --output-on-failure -L unit
```` 

To run all test, don't indicate any label or name:
```
ctest -C Release --output-on-failure
```` 
## [Installer creation](#installer)
CPack can be used to create the installer after the build phase :

## Ubuntu .deb (Experimental)
 ```
cd _build
cpack -G DEB .
```

## Linux .tar.gz
 ```
cd _build
cpack -G TGZ
```
There are still some system libraries that must be installed if you want to use *antares-xpansion*:

```
sudo apt-get install libcurl4 libjsoncpp1 libboost-mpi-dev
```
Note :
>These libraries Compilation can be done on several processor with ```-j``` option.
