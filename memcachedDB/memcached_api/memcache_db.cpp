/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file memcache_db.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/01/12 00:18:53
 * @brief
 *
 **/


#include "memcache_db.h"


MemcacheDB::MemcacheDB() {
}

MemcacheDB::~MemcacheDB() {
}


bool MemcacheDB::OpenDB() {
    return true;
}

Status MemcacheDB::Put(const char* key, const char* value) {
    Status stat;
    return stat;
}

Status MemcacheDB::Get(const char* key, std::string& value) {
    Status stat;
    return stat;
}

Status MemcacheDB::Delete(const char* key) {
    Status stat;
    return stat;
}




/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
