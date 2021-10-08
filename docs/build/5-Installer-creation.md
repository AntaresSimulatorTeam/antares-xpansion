# Installer creation
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