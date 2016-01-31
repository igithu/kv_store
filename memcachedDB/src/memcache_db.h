/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file memcache_db.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/01/12 00:11:54
 * @brief
 *
 **/




#ifndef __MEMCACHE_DB_H
#define __MEMCACHE_DB_H

#include <string>

#include "items_access.h"

enum Status {
};

struct Options {
};

struct ReadOptions {
    bool need_cas;
    char* suffix;
};

struct WriteOptions {
    protocol prot;
    int32_t cas_id;
    time_t exptime;
};

class MemcacheDB {
    public:
        MemcacheDB();
        ~MemcacheDB();

        bool OpenDB();
        bool MemcacheDBInit();
        bool StopMemcacheDB();

        bool Put(WriteOptions& w_options, char* key, const char* value);
        bool Get(ReadOptions& r_options, const char* key, std::string& value);
        bool Delete(WriteOptions& w_options, const char* key);

    private:
        bool start_lru_crawler_;
        bool start_lru_maintainer_;
};




#endif // __MEMCACHE_DB_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
