/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file mdb_store.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/29 23:35:36
 * @brief
 *
 **/




#ifndef __MDB_STORE_H
#define __MDB_STORE_H


#include <leveldb/db.h>

#include "kv_store.h"

namespace kv_store {

class MdbStore : public KVStore {
    public:
        MdbStore();
        ~MdbStore();

        virtual bool Put(const char* key, const char* value);
        virtual bool Get(const char* key, std::string& value);
        virtual bool Delete(const char* key);

    private:
        bool InitMdbStore();

};

}  // end of namespace kv_store






#endif // __MDB_STORE_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
