#!/bin/bash
set -x
#Define source directory
xpansion_sources=$1
ln -s $xpansion_sources /workspace/antares-xpansion

#Define binary cache directory
cache_dir=$2
ccache_cache_dir=$(realpath "$cache_dir/ccache")
vcpkg_cache_dir=$(realpath "$cache_dir/vcpkg_cache")
build_dir=$(realpath "$cache_dir/build")
install_dir=$(realpath "$cache_dir/install")
mkdir $ccache_cache_dir
mkdir $vcpkg_cache_dir
export CCACHE_DIR=$ccache_cache_dir/ccache/.ccache
export CCACHE_BASEDIR=$ccache_cache_dir/ccache
export CCACHE_COMPRESS=1
export PATH="/usr/lib/ccache:$PATH"

pip3 install --upgrade pip
pip3 install wheel
pip3 install -r /workspace/antares-xpansion/requirements-tests.txt

export VCPKG_ROOT=/workspace/antares-xpansion/vcpkg
ORTOOLS_TAG=$(cat /workspace/antares-xpansion/antares-version.json | jq -r '."or-tools-rte"' )
echo "OR-Tools tag :" $ORTOOLS_TAG
URL_ORTOOLS=https://github.com/rte-france/or-tools-rte/releases/download/v$ORTOOLS_TAG/ortools_cxx_centos7_static_sirius.zip
echo "Downloading " $URL_ORTOOLS
mkdir -p ortools
pushd ortools
wget -O ortools.zip $URL_ORTOOLS
echo "Done"
unzip -q ortools.zip
rm -f ortools.zip
popd

ANTARES_VERSION=$(cat /workspace/antares-xpansion/antares-version.json | jq -r '."antares_version"' )
echo "ANTARES_VERSION=$ANTARES_VERSION"
mkdir -p deps
URL_ANTARES=https://github.com/AntaresSimulatorTeam/Antares_Simulator/releases/download/v$ANTARES_VERSION/antares-${ANTARES_VERSION}-CentOS-7.9.2009.tar.gz
wget $URL_ANTARES
tar -xvf antares-${ANTARES_VERSION}-CentOS-7.9.2009.tar.gz -C deps --strip-components=1 &&\
rm -rf antares-${ANTARES_VERSION}-CentOS-7.9.2009.tar.gz