AUTOMAKE_OPTIONS=foreign subdir-objects

# top_dir = ${top_srcdir}/..
mdb_dir = ${top_srcdir}/src
test_dir = ${top_srcdir}/test

AM_CPPFLAGS = -I${mdb_dir}

#AM_LDFLAGS = -lz -lrt
AM_LDFLAGS = -lz -lstdc++

# LDADD = ${third_dir}/gperftools/lib/libtcmalloc_minimal.a

MDB_SRC = ${mdb_dir}/libmemcache_db.a \
		  ${test_dir}/memcache_db.cpp \
		  ${test_dir}/trace.h


bin_PROGRAMS = test_singlemdb

test_singlemdb_SOURCES = ${MDB_SRC}

#SUBDIRS = test
