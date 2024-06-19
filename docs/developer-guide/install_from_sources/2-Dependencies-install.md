# Dependencies install

Antares-Xpansion depends on several mandatory libraries:

- [JsonCpp](https://github.com/open-source-parsers/jsoncpp),
- [Google Test](https://github.com/google/googletest),
- [OR-Tools](https://github.com/AntaresSimulatorTeam/or-tools/tree/rte_dev_sirius),
- [Boost](https://www.boost.org/) : MPI serialization (only for MPI benders compilation), program-options,
- [Doxygen](https://www.doxygen.nl/index.html) for documentation generation,
- [GraphViz](https://graphviz.org/) for Doxygen use.

This section describes the install procedures for the third-party open source libraries used by Antares-Xpansion.
The install procedure can be done:

- By using a package manager,
- By compiling the sources after cloning the official git repository.

## Install with a package manager

=== "Windows (VCPKG)"

    For Windows we use [vcpkg](https://github.com/microsoft/vcpkg) to download and compile the libraries. `vcpkg` is available as a submodule in Antares-Xpansion.
    
    1. You must install the corresponding [vcpkg-triplet](https://vcpkg.readthedocs.io/en/latest/users/integration/#triplet-selection) depending on the Antares version and libraries load:
    
        - ``x64-windows``        : 64 bits version with dynamic libraries load,
        - ``x86-windows``        : 32 bits version with dynamic libraries load,
        - ``x64-windows-static`` : 64 bits version with static libraries load,
        - ``x86-windows-static`` : 32 bits version with static libraries load.
    
        The chosen vcpkg-triplet will be named `<vcpkg-triplet>` in this document.
    
    2. Init submodule and install vcpkg:
    
        ```
        git submodule update --init vcpkg
        cd vcpkg
        .\bootstrap-vcpkg.bat
        ```
        
        All vcpkg command further described must be run from the vcpkg folder. This folder will be named `<vcpkg_root>` later in this document.
    
    
    3. Install dependencies:
        ```
        cd vcpkg
        vcpkg install jsoncpp gtest boost-mpi boost-program-options --triplet <vcpkg-triplet> 
        ```
        
        !!! Note
            Boost-mpi compilation depends on MSMPI redistributable package. Please follow VCPKG procedure :
            ```
            Please install the MSMPI redistributable package before trying to install this port.
            The appropriate installer has been downloaded to:
            <vcpkg_root>/downloads/msmpisetup-10.0.12498.exe
            ``` 

=== "Centos (yum)"

    ```
    sudo yum install 
    sudo yum install environment-modules jsoncpp-devel gtest-devel openmpi-devel boost-openmpi-devel boost-program-options doxygen graphviz redhat-lsb-core
    sudo yum install libuuid-devel
    ```

=== "Ubuntu (apt-get)"

    ```
    sudo apt-get install lsb-release libjsoncpp-dev libgtest-dev libboost-mpi-dev doxygen graphviz libboost-program-options-dev
    sudo apt-get install unzip uuid-dev
    ```
    !!! Note
        Depending on Ubuntu version you might need to compile google test :
        ```
        cd /usr/src/googletest/
        sudo cmake .
        sudo cmake --build . --target install
        ```

## Automatic libraries compilation from git

[Antares dependencies compilation repository](https://github.com/AntaresSimulatorTeam/antares-deps) is used as a git submodule for automatic libraries compilation from git.

ALL dependencies can be built at configure time using the option `-DBUILD_ALL=ON` (`OFF` by default), see [here](3-Build.md#configure). For a list of available option see [Antares dependencies compilation repository](https://github.com/AntaresSimulatorTeam/antares-deps).

Some dependencies cannot be installed with a package manager. They can be built at configure step with a `cmake` option  : `-DBUILD_not_system=ON` (`ON` by default):

!!! warning 
    `boost-mpi` is not compiled with this repository. On windows, VCPKG use is mandatory or you must compile `boost-mpi` by yourself.

### Defining dependency install directory

When using multiple directories for Antares development with multiple branches it can be useful to have a common dependency install directory.

Dependencies install directory can be specified with `-DDEPS_INSTALL_DIR`. By default, the install directory is `<antares_xpansion_checkout_dir>/../rte-antares-deps-<build_type>`.

!!! Note
    `DEPS_INSTALL_DIR` is added to `CMAKE_PREFIX_PATH`.

### Pre-compiled libraries download : release version only
You can download pre-compiled `antares-deps` archive from the [Antares dependencies compilation repository][antares-deps-url]. Only release versions are available.

!!! Note
    For windows, you must you use a MSVC version compatible with the MSVC version used in GitHub Action.

[antares-deps-url]: https://github.com/AntaresSimulatorTeam/antares-deps/releases/tag/v2.0.0-rc2

