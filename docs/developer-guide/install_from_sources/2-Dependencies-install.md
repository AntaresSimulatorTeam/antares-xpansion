# Dependencies install

Antares-Xpansion depends on several mandatory libraries:

- [JsonCpp](https://github.com/open-source-parsers/jsoncpp),
- [Google Test](https://github.com/google/googletest),
- [Boost](https://www.boost.org/) : MPI serialization (only for MPI benders compilation), program-options,
- [Doxygen](https://www.doxygen.nl/index.html) for documentation generation,
- [GraphViz](https://graphviz.org/) for Doxygen use.
- [Antares Simulator](https://github.com/AntaresSimulatorTeam/Antares_Simulator) for Antares simulation.

This section describes the installation procedures for the third-party open source libraries used by Antares-Xpansion.
The installation procedure can be done:

- By using a package manager,
- By compiling the sources after cloning the official git repository.

## Install with a VCPKG package manager

This is the preferred method for installing dependencies.
Dependencies are described in the `vcpkg.json` file.

### Configure VCPKG

```
git submodule update --init vcpkg
cd vcpkg
./bootstrap-vcpkg.sh -disableMetrics
# Or for windows
.\bootstrap-vcpkg.bat -disableMetrics
```

### Installing dependencies

Dependencies can be installed with the following command but is not the prefered method

```
#Repository root
./vcpkg/vcpkg install
```

The preferred method is to install and update dependencies at configure time with cmake like in the following example:

```
#Repository root
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

#### Runtime dependency

When running Antares-Xpansion in parallel, mpirun is used. It is installed through VCPKG but needs to be added to the PATH.

```
export PATH=$PATH:<path_to_vcpkg>/installed/<triplet>/tools/openmpi/bin
```

Alternatively you can install openmpi yourself 

## Other dependencies
- Antares Simulator: either build it from source, download precompiled binaries or use the automatic build method
- Or-tools: either build it from source, download precompiled binaries or use the automatic build method
  - If you are downloading pre-compiled binaries of or-tools you need to select a version supporting Sirius solver

### Using pre-build dependency
If using built from source or pre-built release of Simulator, Or-tools-rte or other dependency, you can specify the path to the dependency in the CMake configuration.

```
cmake -B build -S . -DCMAKE_PREFIX_PATH="<path_to_simulator>;<path_to_or_tools>"
```
### Versions
- [Antares Simulator](https://github.com/AntaresSimulatorTeam/Antares_Simulator): usually [the latest version](https://github.com/AntaresSimulatorTeam/Antares_Simulator/releases/latest) is recommended. You can check antares-version.json for the specific version tied to your code version.
- [Or-tools-rte](https://github.com/rte-france/or-tools-rte): Or-tools-rte is not a direct dependency of Xpansion but one of Simulator, the version or or-tools-rte should match the version used by your version of Simulator. You can check antares-version.json for the specific version tied to your code version. Select the <your-plateform>_static_sirius.zip file. Other variants may work but are not tested.

## Automatic Antares Simulator build
If Antares Simulator is not installed, it will be automatically downloaded and built by the build system.