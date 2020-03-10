#!/bin/bash
#
# HDF5 installation procedure
#

if [ "x$TRUST_ROOT" = "x" ]; then
  echo TRUST_ROOT not defined!
  exit -1
fi

archive=$1
install_dir=$TRUST_MED_ROOT

###############################
# Compilation and installation
###############################
if [ "x$TRUST_USE_EXTERNAL_HDF" = "x" ]; then
  echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
  echo "Installing HDF5 from externalpackages : $archive"
  echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"

  build_root=$TRUST_ROOT/build/lib
  build_dir=$build_root/hdf5_build
  actual_install_dir=$install_dir/hdf5_install

  mkdir -p $build_dir
  mkdir -p $actual_install_dir
  cd $build_root
  
  echo "Un-packing ..."
  tar xzf $archive
  if ! [ $? -eq 0 ]; then
    echo "HDF5 installation: archive '$archive' could not be found or could not be extracted!!!"
    exit -1
  fi
  
  archive_short=$(basename $archive)
  src_dir=${archive_short%.tar.gz}
  cd $build_dir
  
  echo "Configuring ..."
  # All of those options are already set with the following values by default, but just to be sure force them again ...:
  options="-DCMAKE_C_COMPILER=$TRUST_cc_BASE -DCMAKE_CXX_COMPILER=$TRUST_CC_BASE"
  options="$options -DBUILD_SHARED_LIBS=ON -DHDF5_BUILD_CPP_LIB=ON -DHDF5_BUILD_HL_LIB=ON"
  options="$options -DHDF5_BUILD_TOOLS=ON -DHDF5_ENABLE_USING_MEMCHECKER=ON -DHDF5_ENABLE_DIRECT_VFD=OFF"
  cmake $options -DCMAKE_INSTALL_PREFIX="$actual_install_dir" -DCMAKE_BUILD_TYPE=Release ../$src_dir || exit -1
  
  $TRUST_MAKE  || exit -1
  make install || exit -1 
else  
  if ! [ -d "$TRUST_USE_EXTERNAL_HDF" ]; then
    echo "Variable TRUST_USE_EXTERNAL_HDF has been defined but points to an invalid directory: $TRUST_USE_EXTERNAL_HDF"
    exit -1
  fi
  actual_install_dir=$TRUST_USE_EXTERNAL_HDF
fi

#################
# Links creation
#################
# If we point to an external installation the 'actual_install_dir' variable will point to the right place
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
echo "Creating HDF5 links ..."
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"

for ze_dir in bin lib include; do
  mkdir -p $install_dir/$ze_dir
  cd $install_dir/$ze_dir
  for f in $actual_install_dir/$ze_dir/*; do
    link_name=$(basename $f)
    #link_name=${link_name/_debug/}    # remove "_debug" if HDF5 is compiled in debug mode
    ln -nsf $f $link_name || exit -1
  done  
done

echo "All done !"


