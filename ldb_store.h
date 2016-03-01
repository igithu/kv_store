/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file ldb_store.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/29 23:09:56
 * @brief
 *
 **/




#ifndef __LDB_STORE_H
#define __LDB_STORE_H

#include <string>

#include <leveldb/db.h>

#include "kv_store.h"

namespace kv_store {

class LdbStore : public KVStore {
    public:
        LdbStore();
        ~LdbStore();

        virtual bool Put(const char* key, const char* value);
        virtual bool Get(const char* key, std::string& value);
        virtual bool Delete(const char* key);

    private:
        bool InitLdbStore();
        bool CreateDir(const char* path);

    private:
        leveldb::Options options_;
        leveldb::WriteOptions w_options_;
        leveldb::DB* ldb_ptr_;
};

}  // end of namespace kv_store


#endif // __LDB_STORE_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
