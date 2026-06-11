# Installer creation
CPack can be used to create the installer after the build phase :

=== "Windows"
    ```
    cd _build
    cpack -G ZIP
    ```
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
    There are still some system libraries that must be installed if you want to use Antares-Xpansion:
    ```
    sudo apt-get install libcurl4 libjsoncpp1 libboost-mpi-dev libboost-program-options-dev
    ```
