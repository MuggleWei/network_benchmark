#!/bin/bash

# handle argv
if [ "$#" -lt 1 ]; then
	echo "[ERROR] build.sh without build type"
	echo "[ERROR] Usage: build.sh <Debug|Release|RelWithDebInfo|Coverage> [shared|static]"
	exit 1
else
	# to lowercase
	BUILD_TYPE=$(echo $1 | tr '[:upper:]' '[:lower:]')

	if [ "$#" -gt 1 ]; then
		LIB_TYPE=$(echo $2 | tr '[:upper:]' '[:lower:]')
	else
		LIB_TYPE="shared"
	fi
fi

if [ "$LIB_TYPE" = "shared" ]; then
	CMAKE_SHARED_LIBS=ON
else
	CMAKE_SHARED_LIBS=OFF
fi

echo "build type: $BUILD_TYPE"
echo "lib type: $LIB_TYPE"
echo "CMAKE_SHARED_LIB: $CMAKE_SHARED_LIBS"

# variables
origin_dir="$(dirname "$(readlink -f "$0")")"
build_dir=$origin_dir/build
deps_dir=$build_dir/_deps
install_dir=$origin_dir/dist/$BUILD_TYPE-$LIB_TYPE

if [ -d $build_dir ]; then
	rm -rf $build_dir
fi
mkdir -p $build_dir

cmake \
	-S $origin_dir -B $build_dir \
	-DCMAKE_BUILD_TYPE=$BUILD_TYPE \
	-DBUILD_SHARED_LIBS=$CMAKE_SHARED_LIBS \
	-DCMAKE_INSTALL_PREFIX=$install_dir
cmake --build $build_dir --target install

#cp -r $deps_dir/mugglec-src/* $install_dir
#cp -r $deps_dir/haclog-src/* $install_dir
