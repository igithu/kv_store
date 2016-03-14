AUTOMAKE_OPTIONS=foreign subdir-objects

# top_dir = ${top_srcdir}/..
mdb_dir = ${top_srcdir}/src

AM_CPPFLAGS = -I${mdb_dir}

#AM_LDFLAGS = -lz -lrt
AM_LDFLAGS = -lz -lstdc++

# LDADD = ${third_dir}/gperftools/lib/libtcmalloc_minimal.a

MDB_SRC = ${mdb_dir}/mdb.h \
		  ${mdb_dir}/mdb.cpp \
		  ${mdb_dir}/item_manager.h \
		  ${mdb_dir}/item_manager.cpp \
		  ${mdb_dir}/lru_crawler.h \
		  ${mdb_dir}/lru_crawler.cpp \
		  ${mdb_dir}/lru_maintainer.h \
		  ${mdb_dir}/lru_maintainer.cpp \
		  ${mdb_dir}/slabs_manager.h \
		  ${mdb_dir}/slabs_manager.cpp \
		  ${mdb_dir}/slabs_maintainer.h \
		  ${mdb_dir}/slabs_maintainer.cpp \
		  ${mdb_dir}/slabs_rebalancer.h \
		  ${mdb_dir}/slabs_rebalancer.cpp \
		  ${mdb_dir}/assoc_maintainer.h \
		  ${mdb_dir}/assoc_maintainer.cpp \
		  ${mdb_dir}/hash.h \
		  ${mdb_dir}/hash.cpp \
		  ${mdb_dir}/murmur3_hash.h \
		  ${mdb_dir}/murmur3_hash.c \
		  ${mdb_dir}/util.h \
		  ${mdb_dir}/util.cpp \
		  ${mdb_dir}/global.h \
		  ${mdb_dir}/disallow_copy_and_assign.h \
		  ${mdb_dir}/thread.h \
		  ${mdb_dir}/global.h


noinst_LIBRARIES = libmemcached.a

libmemcached_a_SOURCES = ${MDB_SRC}

#SUBDIRS = test
