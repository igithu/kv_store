#!/bin/sh

# make the levelDB lib and copy the include and libs in the current file

BUILD_ROOT=$PWD
LDB_LIB=$BUILD_ROOT/lib
LDB_INCLUDE=$BUILD_ROOT/include

SOURCE_DY_LIB=$BUILD_ROOT/leveldb/out-shared/libleveldb.*
SOURCE_STATIC_LIB=$BUILD_ROOT/leveldb/out-static/*.a
SOURCE_LDB_INCLUDE=$BUILD_ROOT/leveldb/include

cd leveldb && make

echo "make the leveldb successful!"

if [ $? -ne 0 ]
then
    echo "make the ldb lib failed!"
    exit -1
fi

cd ..

# clean old libs
if [ -d $LDB_LIB ]
then
    rm -rf $LDB_LIB
fi
if [ -d $LDB_INCLUDE ]
then
    rm -rf $LDB_INCLUDE
fi

echo "copy the leveldb lib and include file"

cp -rf leveldb/include $BUILD_ROOT/

mkdir -p lib/shared
cp $SOURCE_DY_LIB lib/shared

mkdir -p lib/static
cp $SOURCE_STATIC_LIB lib/static

cd leveldb && make clean && cd ..
