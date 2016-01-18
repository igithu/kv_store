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

#include "memcache.h"

enum Status {
};

class MemcacheDB {
    public:
        MemcacheDB();
        ~MemcacheDB();

        bool OpenDB();
        bool MemcacheDBInit();
        bool StopMemcacheDB();
        void SetProtocol(protocol prot);

        bool Put(const char* key, const char* value);
        bool Get(const char* key, std::string& value);
        bool Delete(const char* key);

    private:
        protocol prot_;
        bool start_lru_crawler_;
        bool start_lru_maintainer_;
};




#endif // __MEMCACHE_DB_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
