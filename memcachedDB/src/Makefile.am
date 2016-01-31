AUTOMAKE_OPTIONS=foreign subdir-objects

# top_dir = ${top_srcdir}/..
mdb_dir = ${top_srcdir}/src

AM_CPPFLAGS = -I${mdb_dir}

#AM_LDFLAGS = -lz -lrt
AM_LDFLAGS = -lz -lstdc++

# LDADD = ${third_dir}/gperftools/lib/libtcmalloc_minimal.a

MDB_SRC = ${mdb_dir}/memcache_db.h \
		  ${mdb_dir}/memcache_db.cpp \
		  ${mdb_dir}/items_access.h \
		  ${mdb_dir}/items_access.cpp \
		  ${mdb_dir}/items.h \
		  ${mdb_dir}/items.cpp \
		  ${mdb_dir}/assoc.h \
		  ${mdb_dir}/assoc.c \
		  ${mdb_dir}/slabs.h \
		  ${mdb_dir}/slabs.c \
		  ${mdb_dir}/hash.h \
		  ${mdb_dir}/hash.c \
		  ${mdb_dir}/murmur3_hash.h \
		  ${mdb_dir}/murmur3_hash.c \
		  ${mdb_dir}/jenkins_hash.h \
		  ${mdb_dir}/jenkins_hash.c \
		  ${mdb_dir}/stats.h \
		  ${mdb_dir}/stats.c \
		  ${mdb_dir}/util.h \
		  ${mdb_dir}/util.c \
		  ${mdb_dir}/protocol_binary.h \
		  ${mdb_dir}/trace.h


noinst_LIBRARIES = libmemcache_db.a

libmemcache_db_a_SOURCES = ${MDB_SRC}

#SUBDIRS = test
