/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file ldb_store.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/03/01 23:06:41
 * @brief
 *
 **/

#include "ldb_store.h"


namespace kv_store {

LdbStore::LdbStore() : db_ptr_(NULL) {
}

LdbStore::~LdbStore() {
}

bool LdbStore::InitLdbStore(const char* db_path) {
    if (!CreateDir(db_path)) {
        fprintf(stderr, "create the dir %s failed!", db_path);
        return false;
    }
    options_.create_if_missing = true;
    w_options_.sync = true;
    leveldb::DB::Open(options_, db_path, &db_ptr_);
    if (NULL == db_ptr_) {
        fprintf(stderr, "level_db init error! db_ptr_ is null!");
        return false;
    }
    return true;
}

bool LdbStore::Put(const char* key, const char* value) {
    if (NULL == db_ptr_) {
        fprintf(stderr, "level_db put op error! db_ptr_ is null!");
        return false;
    }
    leveldb::Status status = db_ptr_->Put(w_options_, key, value);
    if (!status.ok()) {
        fprintf(stderr, "put op failed!");
        return false;
    }
    return true;
}

bool LdbStore::Get(const char* key, std::string& value) {
    if (NULL == key /*|| "" = *key*/) {
        fprintf(stderr, "key is null");
        return false;
    }

    if (NULL == db_ptr_) {
        fprintf(stderr, "level_db get op error! db_ptr_ is null!");
        return false;
    }

    leveldb::Status status = db_ptr_->Get(leveldb::ReadOptions(), key, &value);
    if (!status.IsNotFound() && !status.ok()) {
        fprintf(stderr, "get op failed!");
        return false;
    } else if (status.IsNotFound()) {
        value = "";
    }
    return true;
}

bool LdbStore::Delete(const char* key) {
    if (NULL == key /*|| "" = *key*/) {
        //fprintf(stderr, "key is null");
        return false;
    }
    if (NULL == db_ptr_) {
        fprintf(stderr, "level_db delete op error! db_ptr_ is null!");
        return false;
    }
    leveldb::Status status = db_ptr_->Delete(w_options_, key);
    if (!status.ok()) {
        fprintf(stderr, "delete op failed!");
        return false;
    }
    return true;
}

bool LdbStore::CreateDir(const char* path) {
    char cmd[256];
    sprintf(cmd, "mkdir -p %s", path);
    if (system(cmd) == -1) {
        return false;
    }
    return true;
}



}  // end of namespace kv_store





/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
