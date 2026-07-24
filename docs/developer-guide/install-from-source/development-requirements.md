# Development requirements

## C++ version

The compilation of Antares Xpansion requires C++20 and GCC >= 11.

_mpicxx_ is used to build some executables with MPI support. _mpicxx_ relies on `/usr/bin/c++` on Linux to compile the
code. Please ensure that `/usr/bin/c++` points to a C++20-capable compiler (GCC >= 11) or adjust your PATH/symlink
accordingly. If not, you may get an error linking to TBB: `fatal error: tbb/blocked_range.h`.

Note: CMake enforces GCC >= 11. If a GNU compiler with version less than 11 is detected, configuration will fail with
the following message (X is the detected version):

```
Xpansion requires GCC >= 11, your version is X. GCC >= 11 is required for compatibility with Intel TBB oneAPI
```

=== "Windows"

    Compilation is tested on Visual Studio 17 2022.

=== "Ubuntu"

    ```
    sudo apt update
    sudo apt install build-essential gcc-11 g++-11
    # Make gcc-11/g++-11 the default system compiler (optional but recommended):
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100
    ```

## [CMake version](#cmake-version)

CMake 3.x must be used.

=== "Windows"

    You can download latest Windows version directly from [CMake website](https://cmake.org/download/).

=== "Ubuntu"

    ```
    sudo apt install cmake
    ```

    Or

    ```
    python -m pip install cmake
    ```

## [Python version](#python-version)

Python 3.x must be used.

=== "Windows"

    You can download latest Windows version directly from [Python website](https://www.python.org/downloads/windows/).

=== "Ubuntu"

    ```
    sudo apt-get install python3 python3-pip
    ```

Required python modules can be installed with:

```
pip install -r requirements-tests.txt
```

## [Git version](#git-version)

Git version must be above 2.15 for external dependencies build because `--ignore-whitespace` is not used by default and
we have an issue with OR-Tools compilation of ZLib and application of patch on Windows (
see [https://github.com/google/or-tools/issues/1193](https://github.com/google/or-tools/issues/1193)).

=== "Windows"

    You can download latest Windows version directly from [Git website](https://gitforwindows.org/).

=== "Ubuntu"

    ```
    sudo apt-get install git
    ```

Now you can clone the repository:

```
git clone https://github.com/AntaresSimulatorTeam/antares-xpansion.git
```

Most of the rest of the guide will assume that you are in the root directory of the repository.
