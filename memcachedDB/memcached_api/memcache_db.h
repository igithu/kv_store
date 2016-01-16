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
        void SetProtocol(protocol prot);
        Status Put(const char* key, const char* value);
        Status Get(const char* key, std::string& value);
        Status Delete(const char* key);

    private:
        protocol prot_;
};




#endif // __MEMCACHE_DB_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
