/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file kv_store.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/28 22:02:37
 * @brief
 *
 **/




#ifndef __KV_STORE_H
#define __KV_STORE_H


#include <string>


namespace kv_store {

struct Options {
};

struct ReadOptions {
};

struct WriteOptions {
};

class KVStore {
    public:
        KVStore() {
        }

        ~KVStore() {
        }

        bool Start();

        virtual bool Put(const char* key, const char* value) = 0;
        virtual bool Get(const char* key, std::string& value) = 0;
        virtual bool Delete(const char* key) = 0;
};


} // end of namespace kv_store


#endif // __KV_STORE_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
