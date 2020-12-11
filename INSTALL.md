# Antares XPansion V2 CMake Build Instructions

 [CMake version](#cmake-version) | [Git version](#git-version) | [Dependencies](#dependencies) | [Building](#building-antares-solution) |
 
## [CMake version](#cmake-version)
CMake 3.x must be used.
On some OS it is not available by default on the system.

### Ubuntu
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

### RHEL / Centos
```
sudo yum install cmake3
```
Note:
> All ``cmake``  command must be replaced by ``cmake3``.

## [Git version](#git-version)
Git version must be above 2.15 for external dependencies build because `--ignore-whitespace` is not used by default and we have an issue with OR-Tools compilation of ZLib and application of patch on Windows (see https://github.com/google/or-tools/issues/1193).

## [Dependencies](#deps)
 ANTARES XPansion V2 depends on severals mandatory libraries. 
 - [JsonCpp](https://github.com/open-source-parsers/jsoncpp)
 - [Google Test](https://github.com/google/googletest)
 - [OR-Tools](https://github.com/AntaresSimulatorTeam/or-tools/tree/rte_dev_sirius)
 - Boost mpi (Only for MPI benders compilation)
 - [Doxygen](https://www.doxygen.nl/index.html) for documentation generation
 - [GraphViz](https://graphviz.org/) for doxygen use

This section describes the install procedures for the third-party Open source libraries used by ANTARES XPansion V2.
The install procedure can be done
- by compiling the sources after cloning the official git repository
- by using a package manager. Depending on the OS we propose a solution
  - using VCPKG (Only tested on windows)
  - using the official package manager of the linux distribution


### [Using VCPKG](#vcpkg)

For Windows we will use [vcpkg](https://github.com/microsoft/vcpkg) to download and compile the librairies.

You must install the corresponding [vcpkg-triplet](https://vcpkg.readthedocs.io/en/latest/users/integration/#triplet-selection) depending on Antares version and libraries load:

- ``x64-windows``        : 64 bits version with dynamic librairies load
- ``x86-windows``        : 32 bits version with dynamic librairies load
- ``x64-windows-static`` : 64 bits version with static librairies load
- ``x86-windows-static`` : 32 bits version with static librairies load

The vcpkg-triplet used will be named [vcpg-triplet] later in this document.

#### 1 Install vcpkg 

vcpkg can be installed anywhere on your computer :

```
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

Note :
> all vcpkg command further described must be run from vcpkg folder. This folder will be named [vcpkg_root] later in this document.


#### 2 Install dependencies
```
cd [vcpkg_root]
vcpkg install jsoncpp:[vcpg-triplet] 
vcpkg install gtest:[vcpg-triplet] 
vcpkg install boost-mpi:[vcpg-triplet] 
```

Note :
> On Windows, boost-mpi compilation depends on MSMPI redistributable package. Please follow VCPKG procedure :
> ```
> Please install the MSMPI redistributable package before trying to install this port.
>     The appropriate installer has been downloaded to:
>      [vcpkg_root]/downloads/msmpisetup-10.0.12498.exe
> ``` 
### [Using a package manager](#linux_manager)
On linux you can use a package manger to download the precompiled librairies.

#### Ubuntu

```
sudo apt-get install libjsoncpp-dev libgtest-dev libboost-mpi-dev doxygen graphviz
```
Note :
> Depending on Ubuntu version you might need to compile google test :
> ```
> cd /usr/src/googletest/
> sudo cmake .
> sudo cmake --build . --target install
> ```
#### RHEL / Centos

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

### [Automatic librairies compilation from git](#git_compil)
[Antares dependencies compilation repository](https://github.com/AntaresSimulatorTeam/antares-deps) is used as a git submodule for automatic librairies compilation from git.

Dependency can be built at configure time using the option `-DBUILD_ALL=ON` (`OFF` by default). For a list of available option see [Antares dependencies compilation repository](https://github.com/AntaresSimulatorTeam/antares-deps).

We recommand you to define the options below at your first antares xpansion checkout :

* Sirius solver (`-DBUILD_sirius=ON`)
* OR-Tools (`-DBUILD_ortools=ON`)

This allow the compilation of the librairies that are not available in package manager.


#### Defining dependency install directory
When using multiple directories for antares development with multiple branches it can be useful to have a common dependency install directory.

Dependency install directory can be specified with `DEPS_INSTALL_DIR`. By default install directory is `<antares_xpansion_checkout_dir>/../rte-antares-deps-<build_type>`

Note :
> `DEPS_INSTALL_DIR` is added to `CMAKE_PREFIX_PATH`

## [Building Antares XPansion V2](#build)
First you need to update git submodule for dependency build :
```
git submodule update --init antares-deps
```


You can define build type with ```-DCMAKE_BUILD_TYPE``` and ```--config``` option.

- release

```
cmake -B _build -S . -DCMAKE_BUILD_TYPE=Release -DUSE_SEQUENTIAL=true -DUSE_MPI=true
cmake --build _build --config Release
```
-  debug
 ```
cmake -B _build -S . -DCMAKE_BUILD_TYPE=Debug -DUSE_SEQUENTIAL=true -DUSE_MPI=true
cmake --build _build --config Debug
```

### OR-Tools linking

By default OR-Tools is NOT compiled with Antares XPansion V2.
You can enable compilation with `-DBUILD_ortools=ON` when you configure build with cmake.

We recommand you to use `-DDEPS_INSTALL_DIR` option so you can use these builds in another antares-xpansion checkout directory.

In this case you can specify dependency install directory with :
```
cmake -DDEPS_INSTALL_DIR=<deps_install_dir>
````
### Linux using system libs
- Install dependencies [using package manager](#using-a-package-manager).

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

### Window using vcpkg
- Install dependencies [using VCPKG](#using-vcpkg).
- Choose [vcpkg-triplet]

- Configure build with cmake

```
cmake -B _build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg_root]/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=[vcpkg-triplet] -DCMAKE_BUILD_TYPE=Release -DUSE_SEQUENTIAL=true -DUSE_MPI=true
```
- Build
 ```
cmake --build _build --config Release -j8
```
Note :
> Compilation can be done on several processor with ```-j``` option.