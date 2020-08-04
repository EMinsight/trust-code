#!/bin/bash

#
# Installation script for MEDCoupling
#

archive_mc=$1
archive_conf=$2
tool_dir=`readlink -f $(dirname $0)`
src_dir=`basename $archive_mc .tar.gz`
mc_version=`echo $src_dir | sed 's/[^0-9]*\([0-9].[0-9].[0-9]\)/\1/'`
build_root=$TRUST_ROOT/build/lib
build_dir=$build_root/medcoupling_build
install_dir=$TRUST_MEDCOUPLING_ROOT
org=`pwd`

mkdir -p $install_dir
mkdir -p $build_dir
cd $build_dir

# include file:
medcoupling_hxx=$install_dir/include/medcoupling++.h

# MEDCoupling uses DataArrayInt32 not DataArrayInt64, so we disable MEDCoupling when building a 64 bits version of TRUST
# if [ "$TRUST_INT64" = "1" ]
# then
#     echo "@@@@@@@@@@@@ INT64 specific stuff ..."
#     mkdir -p $install_dir/include
#     rm -rf $install_dir/lib
#     echo "MEDCOUPLING DISABLE for 64 bits"
#     echo "#define NO_MEDFIELD " > $install_dir/include/ICoCoMEDField.hxx
#     echo "#undef MEDCOUPLING_"  > $medcoupling_hxx
#     exit 0
# fi

rm -f ICoCoMEDField.hxx
if [ "$TRUST_USE_EXTERNAL_MEDCOUPLING" = "1" ]; then
  echo "@@@@@@@@@@@@ Using external MEDCoupling: '$MEDCOUPLING_ROOT_DIR'"
  echo $MEDCOUPLING_ROOT_DIR
  ln -sf $MEDCOUPLING_ROOT_DIR $install_dir
  exit 0
fi

echo "@@@@@@@@@@@@ Unpacking ..."

cd $build_root

[ ! -f $archive_mc ] && echo $archive_mc no such file && exit 1
[ ! -f $archive_conf ] && echo $archive_conf no such file && exit 1
tar zxf $archive_mc
tar zxf $archive_conf

echo "Patching DisjointDEC"
sed -i 's/throw(INTERP_KERNEL::Exception)//' $(find $src_dir -name  DisjointDEC.hxx )
sed -i 's/throw(INTERP_KERNEL::Exception)//' $(find $src_dir -name  DisjointDEC.cxx )

echo "Patching findClosestTupleId() method"
patch -p1 $(find $src_dir -name MEDCouplingMemArray.cxx ) < $tool_dir/closestTupleId.patch

echo "Patching HDF detection procedure and exit if procedure has changed..."
FindSalomeHDF5=$(find $src_dir/.. -name FindSalomeHDF5.cmake )
sed -i "1,$ s?GET_PROPERTY(?#GET_PROPERTY(?" 			$FindSalomeHDF5 || exit -1 
sed -i "1,$ s?MESSAGE(FATAL_ERROR?#MESSAGE(FATAL_ERROR ?" 	$FindSalomeHDF5 || exit -1 

echo "Patching gatherArraysT() and allGathersArraysT() method"
patch -p1 $(find $src_dir -name CommInterface.hxx ) < $tool_dir/CommInterface.patch

echo "Patching DataArrayInt allocation method (initialisation with numpy array only available if WITH_NUMPY is defined)"
patch -p1 $(find $src_dir -name DataArrayInt.i ) < $tool_dir/DataArrayInt.patch


echo "@@@@@@@@@@@@ Configuring, compiling and installing ..."
cd $build_dir

USE_MPI=ON
[ "$TRUST_DISABLE_MPI" -eq 1 ] && USE_MPI=OFF

# We use now python and SWIG from conda so:
OPTIONS="-DMEDCOUPLING_USE_MPI=$USE_MPI -DMPI_ROOT_DIR=$MPI_ROOT -DCMAKE_CXX_COMPILER=$TRUST_CC -DCMAKE_C_COMPILER=$TRUST_cc "
OPTIONS="$OPTIONS -DHDF5_ROOT_DIR=$TRUST_MED_ROOT  -DMEDFILE_ROOT_DIR=$TRUST_MED_ROOT -DMEDCOUPLING_BUILD_DOC=OFF  -DMEDCOUPLING_PARTITIONER_METIS=OFF "
OPTIONS="$OPTIONS -DMEDCOUPLING_PARTITIONER_SCOTCH=OFF -DMEDCOUPLING_ENABLE_RENUMBER=OFF -DMEDCOUPLING_ENABLE_PARTITIONER=OFF -DMEDCOUPLING_BUILD_TESTS=OFF "
OPTIONS="$OPTIONS -DMEDCOUPLING_WITH_FILE_EXAMPLES=OFF -DCONFIGURATION_ROOT_DIR=../configuration-$mc_version -DSWIG_EXECUTABLE=$TRUST_ROOT/exec/python/bin/swig "
OPTIONS="$OPTIONS -DMEDCOUPLING_MEDLOADER_USE_XDR=OFF -DMEDCOUPLING_BUILD_STATIC=ON -DMEDCOUPLING_ENABLE_PYTHON=ON"
# NO_CXX1 pour cygwin
OPTIONS="$OPTIONS -DNO_CXX11_SUPPORT=OFF"
# Error->Warning pour ARM:
OPTIONS="$OPTIONS -DCMAKE_CXX_FLAGS=-Wno-narrowing"
#INT64
if [ "$TRUST_INT64" = "1" ]
then
    OPTIONS="$OPTIONS -DMEDCOUPLING_USE_64BIT_IDS=ON"
fi

echo "About to execute CMake -- options are: $OPTIONS"
echo "Current directory is : `pwd`"
cmake ../$src_dir $OPTIONS -DCMAKE_INSTALL_PREFIX=$install_dir -DCMAKE_BUILD_TYPE=Release

# The current CMake of MEDCoupling is badly written: dependencies on .pyc generation do not properly capture SWIG generated Python modules.
# So we need to do make twice ...
$TRUST_MAKE
make install
make install
status=$?

if ! [ $status -eq 0 ]; then
  echo "@@@@@@@@@@@@@@@@@@@@@"
  echo "Error at compilation."
  exit -1
fi

# Clean build folder
( cd .. ; rm -rf configuration* medcoupling* )

# Creation of env file. Done in a temporary file, because the final env.sh is the main target of the Makefile
# but we need an env file for the test below ... 
echo "@@@@@@@@@@@@ Creating env file ..."
MC_ENV_FILE_tmp=$install_dir/env_tmp.sh
MC_ENV_FILE=$install_dir/env.sh
version=`python  -c "import sys; print (sys.version[:3])"`
echo "export MED_COUPLING_ROOT=$install_dir"> $MC_ENV_FILE_tmp
echo "export LD_LIBRARY_PATH=$install_dir/lib/:$TRUST_MED_ROOT/lib:$TRUST_ROOT/exec/python/lib/:\${LD_LIBRARY_PATH}" >> $MC_ENV_FILE_tmp
echo "export PYTHONPATH=$install_dir/bin/:$install_dir/lib/python$version/site-packages/:\$PYTHONPATH" >> $MC_ENV_FILE_tmp

echo "@@@@@@@@@@@@ Testing install ..."
if [ $status -eq 0 ]  # install was successful
then
  ##
  ## Test de fonctionnement
  ##
  source $MC_ENV_FILE_tmp
  python -c "import medcoupling"
  if [ $? -eq 0 ]
  then
    echo "MEDCoupling library OK"
  else
    echo "MEDCoupling library KO"
    exit -1
  fi
  python -c "import MEDLoader"
  if [ $? -eq 0 ]
  then
    echo "MEDLoader library OK"
  else
    echo "MEDLoader library KO"
    exit -1
  fi
fi

echo "@@@@@@@@@@@@ Updating TRUST include files ..."
touch $install_dir/include/*

[ ! -f $icocomedfield_hxx ] && echo "#define NO_MEDFIELD " > $icocomedfield_hxx
if [ "$TRUST_INT64" = "1" ]
then
    [ ! -f $medcoupling_hxx ]  && printf "#define MEDCOUPLING_\n#define MEDCOUPLING_USE_64BIT_IDS" > $medcoupling_hxx
else
    [ ! -f $medcoupling_hxx ]  && echo "#define MEDCOUPLING_" > $medcoupling_hxx
fi

# Update env file:
mv $MC_ENV_FILE_tmp $MC_ENV_FILE

echo "All done!"


