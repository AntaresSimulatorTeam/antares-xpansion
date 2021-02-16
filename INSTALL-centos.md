# antares-xpansion CMake Build Instructions

 [CMake version](#cmake-version) | [GCC version](#gcc-version) [Dependencies](#dependencies) | [Building](#building-antares-solution) |
 
## [CMake version](#cmake-version)
CMake 3.x must be used.
```
sudo yum install cmake3
```
Note:
> All ``cmake``  command must be replaced by ``cmake3``.

## [GCC version](#gcc-version)
By default, GCC version of Centos7 is 4.8.5.
For compiling antares-xpansion we need C++17 support.

You can use a more recent version of GCC by enabling devtoolset :
```
sudo yum install centos-release-scl
sudo yum install devtoolset-7
```

Before compiling antares-xpansion we must launch a new shell with `scl` tool :
```
scl enable devtoolset-7 bash
```

## [Dependencies](#deps)
antares-xpansion depends on severals mandatory libraries. 
 - [JsonCpp](https://github.com/open-source-parsers/jsoncpp)
 - [Google Test](https://github.com/google/googletest)
 - [OR-Tools](https://github.com/AntaresSimulatorTeam/or-tools/tree/rte_dev_sirius)
 - Boost : mpi serialization (Only for MPI benders compilation)
 - [Doxygen](https://www.doxygen.nl/index.html) for documentation generation
 - [GraphViz](https://graphviz.org/) for doxygen use

This section describes the install procedures for the third-party Open source libraries used by antares-xpansion.
The install procedure can be done
- by compiling the sources after cloning the official git repository
- by using yum

### Yum commands

```
sudo yum install jsoncpp gtest boost-openmpi-devel doxygen graphviz
```

Note :
> Some external repositories must be enabled : EPEL and PowerTools (for boost-mpi-devel on centos8)
> ```
> sudo yum install epel-release
> sudo yum install dnf-plugins-core
> sudo yum config-manager --set-enabled PowerTools
> ``` 

### Automatic librairies compilation from git
[Antares dependencies compilation repository](https://github.com/AntaresSimulatorTeam/antares-deps) is used as a git submodule for automatic librairies compilation from git.

ALL dependency can be built at configure time using the option `-DBUILD_ALL=ON` (`OFF` by default). For a list of available option see [Antares dependencies compilation repository](https://github.com/AntaresSimulatorTeam/antares-deps).

Some dependencies can't be installed with a package manager. They can be built at configure step with a cmake option  : `-DBUILD_not_system=ON` (`OFF` by default):
```
cmake3 -B _build -S . -DCMAKE_BUILD_TYPE=Release -DUSE_SEQUENTIAL=true -DUSE_MPI=true -DBUILD_not_system=ON -DDEPS_INSTALL_DIR=<deps_install_dir>
```
**Warning :**
> boost-mpi is not compiled with this repository.

#### Defining dependency install directory
When using multiple directories for antares development with multiple branches it can be useful to have a common dependency install directory.

Dependency install directory can be specified with `DEPS_INSTALL_DIR`. By default install directory is `<antares_xpansion_checkout_dir>/../rte-antares-deps-<build_type>`

Note :
> `DEPS_INSTALL_DIR` is added to `CMAKE_PREFIX_PATH`

## [Building antares-xpansion](#build)
- Enable `devtoolset-7` :
```
scl enable devtoolset-7 bash
```
- Update git submodule for dependency build :
```
git submodule update --init antares-deps
```

- Configure build with cmake
```
cmake3 -B _build -S [antares_src] -DCMAKE_BUILD_TYPE=Release -DUSE_SEQUENTIAL=true -DUSE_MPI=true
```
- Build
 ```
cmake3 --build _build --config Release -j8
```
Note :
>Compilation can be done on several processor with ```-j``` option.
