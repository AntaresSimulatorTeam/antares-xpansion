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

source /opt/rh/devtoolset-11/enable
source /opt/rh/rh-git227/enable
export VCPKG_BINARY_SOURCES="clear;files,$vcpkg_cache_dir,readwrite"
cmake -B $build_dir -S /workspace/antares-xpansion \
  -DBUILD_TESTING=OFF \
  -DCMAKE_C_COMPILER_LAUNCHER=ccache \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DCMAKE_PREFIX_PATH="/workspace/deps;/workspace/ortools/install" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$install_dir \
  -DALLOW_RUN_AS_ROOT=ON \
  -DVCPKG_TARGET_TRIPLET=x64-linux-release \
  -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_INSTALL_OPTIONS="--x-buildtrees-root=$build_dir/vcpkg_buildtrees"

cmake --build $build_dir --config Release -j`nproc`

cd $build_dir
cmake --install .
cd $install_dir

rm -f ./antares-xpansion-launcher*
pyinstaller -F /workspace/antares-xpansion/src/python/launch.py -n antares-xpansion-launcher --add-data "/workspace/antares-xpansion/src/python/config.yaml:." --add-data "./bin/:bin"
mv ./dist/antares-xpansion-launcher* .
rm -rf bin
rm -rf build
rm -rf dist
rm -f *.spec
cd ..
tar -czf antares-xpansion-centos.tar.gz -C $install_dir . --exclude='examples'
chmod 777 antares-xpansion-centos.tar.gz