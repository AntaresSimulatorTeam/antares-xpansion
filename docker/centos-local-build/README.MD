# Locally generate a centos 7 asset

The goal of this image is to provide a CentOS 7 environment to build xpansion and generate the asset.
One use case is when wanting to test things on OPF environments. This alleviates the need to have a CentOS 7 machine or rely on CI pipelines to generate the asset.

## Prerequisites

Docker installed, proxy configured if necessary.

## Usage

Simple use:
    
    ```./run.sh <source code directory> <output directory>```

## Content

* dockerfile: Dockerfile to build the image
* build.sh: Script to build and generate asset
* run.sh: Script to "one line" the process

## How does it work?

The dockerfile contains the instruction to build an image with the stables requirements to build Xpansion.
The build.sh script will be part of the image and set as entrypoint of containers created from this image.
The build will happen in a container. This is to avoid rebuilding the image everytime the source code changes.
The build.sh script and thus running a container require two arguments: one for source code and one for the output directory.

## Output directory

The output directory is in fact a binary_directory. It will contain the generated asset but also binary cache for subsequent builds.

## Advance usage

* Build the image. Try to set the current directory to the dockerfile directory. Docker use the current directory as context, and it may slow down your build process.
    
        docker build -t <image_name> .

* Run the container with the image. The container will be removed after the build.

        docker run \
        --mount type=bind,src=<path/to/antares-xpansion>,dst=/mnt/sources \
        --mount type=bind,src=<path/for/outputs>,dst=/mnt/caches \
        <image_name> \
        /mnt/sources /mnt/caches

* Run a container but don't do anything. This is useful to run the build yourself

        docker run -it \
        --entry-point /bin/bash \
        --mount type=bind,src=<path/to/antares-xpansion>,dst=/mnt/sources \
        --mount type=bind,src=<path/for/outputs>,dst=/mnt/caches \
        <image_name>

* Build inside the container.

        source ./build.sh /mnt/sources /mnt/caches
`source` command is used to load the proper environment variables and run the script a first time. Subsequent build can be done with the usual cmake commands.

## Improvements

Some improvements can be made to the process. It would be nice to have md5sum of downloaded dependencies to prevent redownloading them and of course store them in the cach directory.

