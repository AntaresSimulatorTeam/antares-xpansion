# Development requirements

## [C++ version](#c++-version)
The compilation of Antares-Xpansion requires C++20.

=== "Windows"

    Compilation is tested on Visual Studio 17 2022.

=== "Centos" :warning: CentOS is considered EoL (end of life) and is not supported anymore. We recommend using a more recent distribution like Oracle Linux 8

    By default, GCC version of Centos7 is 4.8.5. Some external repositories must be enabled:

    === "Centos 7 (EPEL)"    
        ``` 
        sudo yum install epel-release
        sudo yum install centos-release-scl
        ```
    
    You can then use a more recent version of GCC by enabling `devtoolset-10`:
    ```
    sudo yum install devtoolset-10
    ```
    
    Before compiling Antares-Xpansion, we must launch a new shell with `scl` tool :
    ```
    scl enable devtoolset-10 bash
    ```
=== "Ubuntu"

    ```
    sudo apt install build-essential gcc10 g++10
    ```
## [CMake version](#cmake-version)
CMake 3.x must be used.

=== "Windows"

    You can download latest Windows version directly from [CMake website](https://cmake.org/download/).
=== "Centos"

    ```
    sudo yum install epel-release
    sudo yum install cmake3
    ```
Or
    ```
    python -m pip install cmake
    ```
    
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
=== "Centos"

    ```
    sudo yum install python3 python3-pip
    ```
=== "Ubuntu"

    ```
    sudo apt-get install python3 python3-pip
    ```

Required python modules can be installed with:
```
pip install -r requirements-tests.txt
```

## [Git version](#git-version)
Git version must be above 2.15 for external dependencies build because `--ignore-whitespace` is not used by default and we have an issue with OR-Tools compilation of ZLib and application of patch on Windows (see [https://github.com/google/or-tools/issues/1193](https://github.com/google/or-tools/issues/1193)).

=== "Windows"

    You can download latest Windows version directly from [Git website](https://gitforwindows.org/).
=== "Centos"

    ```
    sudo yum install rh-git227-git
    sudo yum install git
    ```
    
    Sometimes we need a 2.x version of git. To enable to git 2.27:
    ```
    source /opt/rh/rh-git227/enable
    ```
    
=== "Ubuntu"

    ```
    sudo apt-get install git
    ```
