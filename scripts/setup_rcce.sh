#!/bin/sh -e

RCCE_TAG_VER="RCCE_V1.0.13"
OMP=0
PWR=0
API="nongory"

exit_error() { echo $1; exit; }

check_sane()
{
    # we need svn
    command -v svn >/dev/null 2>&1 || exit_error "Command svn not found!"
    # we need gcc
    command -v gcc >/dev/null 2>&1 || exit_error "Command gcc not found!"
    # we need g++
    command -v g++ >/dev/null 2>&1 || exit_error "Command g++ not found!"
    # make dir
    [ -e /shared/$USER ] || exit_error "please make directory /shared/$USER with read/write permissions!"
}

# Download the latest stable branch of RCCE
download_rcce()
{
    svn co http://marcbug.scc-dc.com/svn/repository/tags/$RCCE_TAG_VER/
}

build_rcce_emulator()
{
    cd $RCCE_TAG_VER
    # set the location of RCCE, compiler and platform flags
    sed -e "s:_INSERT_ROOTDIR_INSERT_:$PWD:g" -i common/symbols.in
    sed -e "s:CCOMPILE=icc:CCOMPILE=g++:g" -i common/symbols.in
    sed -e "s:CCOMPILE=icpc:CCOMPILE=g++:g" -i common/symbols.in
    sed -e "s:PLATFORMFLAGS=-openmp:PLATFORMFLAGS=-fopenmp:g" -i common/symbols.in
    sed -e "s:-mcpu=pentium::g" -i common/symbols.in
    sed -e "s:-gcc-version=340: -fpermissive:g" -i common/symbols.in
    echo "emulator"
    # specify the build type
    ./configure emulator
    # build the library emulator mode
    ./makeall
    echo "SCC_LINUX"
    # build the library SCC_LINUX mode
#    ./configure SCC_LINUX
#    make OMP_EMULATOR=$OMP PWRMGMT=$PWR API=$API
}

build_rcce_SCC()
{
    cd $RCCE_TAG_VER
    # set the location of RCCE, compiler and platform flags
    sed -e "s:_INSERT_ROOTDIR_INSERT_:$PWD:g" -i common/symbols.in
    sed -e "s:CCOMPILE=icc:CCOMPILE=g++:g" -i common/symbols.in
    sed -e "s:CCOMPILE=icpc:CCOMPILE=g++:g" -i common/symbols.in
    sed -e "s:PLATFORMFLAGS=-openmp:PLATFORMFLAGS=-fopenmp:g" -i common/symbols.in
    sed -e "s:-mcpu=pentium::g" -i common/symbols.in
    sed -e "s:-gcc-version=340: -fpermissive:g" -i common/symbols.in
    echo "emulator"
    # specify the build type
#    ./configure emulator
    # build the library emulator mode
#    ./makeall
#    echo "SCC_LINUX"
    # build the library SCC_LINUX mode
    ./configure SCC_LINUX
    make OMP_EMULATOR=$OMP PWRMGMT=$PWR API=$API
}

#### Processing ####
echo "checking sanity ..."
check_sane
echo "downloading the latest stable tag of RCCE ..."
#download_rcce
echo "patching settings and building RCCE emulator & SCC_LINUX mode ..."
#build_rcce_emulator
build_rcce_SCC
##
