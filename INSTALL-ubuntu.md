# *antares-xpansion* CMake Build Instructions

 [CMake version](#cmake-version) | [Environnement build install](#env-build-install)| [Dependencies](#dependencies) | [Building](#building-antares-solution) |

## C/I status
[![Status][ubuntu_system_svg]][ubuntu_system_link] 

[ubuntu_system_svg]: https://github.com/AntaresSimulatorTeam/Antares_Simulator/workflows/Ubuntu%20CI%20(system%20libs)/badge.svg

[ubuntu_system_link]: https://github.com/AntaresSimulatorTeam/Antares_Simulator/actions?query=workflow%3A"Ubuntu%20CI%20(system%20libs)"

## [CMake version](#cmake-version)
CMake 3.x must be used.

There are several ways to get CMake 3.x on your system.

#### Using snap
```
sudo snap install cmake --classic
```
#### Compiling from sources

```
sudo apt install build-essential libssl-dev
wget https://github.com/Kitware/CMake/releases/download/v3.16.5/cmake-3.18.1.tar.gz
tar -zxvf cmake-3.18.1.tar.gz
cd cmake-3.18.1
./bootstrap
make 
sudo make install
```
Note:
> You can use a different version of CMake. Check CMake website for available version and chang ``wget`` url.

You can then tell Ubuntu that a new version of cmake should be used :
```
sudo update-alternatives --install /usr/bin/cmake cmake /usr/local/bin/cmake 1 --force
```

## [Environnement build install](#env-build-install)
```
sudo apt-get install build-essential
```

## [Python version](#python-version)
Python 3.x must be used.

```
sudo apt-get install python3 python3-pip
```

Required python modules can be installed with :
```
pip3 install -r src/src_python/requirements.txt
pip3 install -r src/src_python/tests/examples/requirements.txt
```

## [Dependencies](#deps)
*antares-xpansion* depends on several mandatory libraries. 
 - [JsonCpp](https://github.com/open-source-parsers/jsoncpp)
 - [Google Test](https://github.com/google/googletest)
 - [OR-Tools](https://github.com/AntaresSimulatorTeam/or-tools/tree/rte_dev_sirius)
 - Boost : mpi serialization (Only for MPI benders compilation)
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
sudo apt-get install libjsoncpp-dev libgtest-dev libboost-mpi-dev doxygen graphviz
sudo apt install uuid-dev libcurl4-openssl-dev libssl-dev
```
Note :
> Depending on Ubuntu version you might need to compile google test :
> ```
> cd /usr/src/googletest/
> sudo cmake .
> sudo cmake --build . --target install
> ```


### Automatic librairies compilation from git
[Antares dependencies compilation repository](https://github.com/AntaresSimulatorTeam/antares-deps) is used as a git submodule for automatic librairies compilation from git.

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

## [antares-solver build](antares-solver-build)
*antares-xpansion* needs the *antares-solver* binary in order to execute the whole simulation process. A Cmake option allows compilation of *antares-solver* at configure : `-DBUILD_antares_solver=ON` (default `ON`)

## [Building *antares-xpansion* V2](#build)
- update git submodule for dependency build :
```
git submodule update --init antares-deps
```

- Configure build with cmake
```
cmake -B _build -S [antares_src] -DCMAKE_BUILD_TYPE=Release -DUSE_SEQUENTIAL=true -DUSE_MPI=true
```
- Build
 ```
cmake --build _build --config Release -j8
```
Note :
>Compilation can be done on several processor with ```-j``` option.

## [Installer creation](#installer)
CPack can be used to create the installer after the build phase :
 ```
cd _build
cpack -G TGZ
```
There are still some system librairies that must be installed if you want to use *antares-xpansion*:

```
sudo apt-get install libcurl4 libjsoncpp1 libboost-mpi-dev
```