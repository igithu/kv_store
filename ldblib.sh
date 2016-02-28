#!/bin/sh

# make the levelDB lib and copy the include and libs in the current file

BUILD_ROOT=$PWD
LIB_LDB=$BUILD_ROOT/libldb
LDB_LIB=$LIB_LDB/lib
LDB_INCLUDE=$LIB_LDB/include

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

# clean old files
if [ -d $LIB_LDB ]
then
    rm -rf $LIB_LDB
fi
mkdir -p $LIB_LDB

echo "copy the leveldb lib and include file"

cp -rf leveldb/include $LIB_LDB

mkdir -p $LIB_LDB/shared
cp $SOURCE_DY_LIB $LIB_LDB/shared

mkdir -p $LIB_LDB/static
cp $SOURCE_STATIC_LIB $LIB_LDB/static

cd leveldb && make clean && cd ..
