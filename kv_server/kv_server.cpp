/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file kv_server.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/29 22:04:22
 * @brief
 *
 **/




#include "kv_server.h"

#include "ldb_store.h"

namespace kv_store {

KVServer::KVServer() : kv_store_ptr_(NULL) {
}

KVServer::~KVServer() {
    if (NULL != kv_store_ptr_) {
        delete kv_store_ptr_;
    }
}

bool KVServer::InitKVStore() {
    /*
     * set the ldb for now
     */
    kv_store_ptr_ = new LdbStore();

    return true;
}


bool KVServer::Put(const char* key, const char* value) {
    if (NULL == kv_store_ptr_) {
        return false
    }
    return kv_store_ptr_->Put(key, value);
}

bool KVServer::Get(const char* key, std::string& value) {
    if (NULL == kv_store_ptr_) {
        return false
    }
    return kv_store_ptr_->Get(key, value);
}

bool KVServer::Delete(const char* key) {
    if (NULL == kv_store_ptr_) {
        return false
    }
    return kv_store_ptr_->Delete(key);
}





}  // end of namespace kv_store










/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
