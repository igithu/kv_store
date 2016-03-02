/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file kv_server.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/28 22:29:57
 * @brief
 *
 **/




#ifndef __KV_SERVER_H
#define __KV_SERVER_H

#include "kv_store.h"

namespace kv_store {

class KVServer {
    public:
        KVServer();
        ~KVServer();

        bool InitKVStore();

        bool Put(const char* key, const char* value);
        bool Get(const char* key, std::string& value);
        bool Delete(const char* key);

    private:
        KVStore* kv_store_ptr_;
};

}  // end of namespace kv_store


#endif // __KV_SERVER_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
