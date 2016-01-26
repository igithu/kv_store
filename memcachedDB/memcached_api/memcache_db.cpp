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


MemcacheDB::MemcacheDB() : start_lru_crawler_(false), start_lru_maintainer_(false) {
}

MemcacheDB::~MemcacheDB() {
}

bool MemcacheDB::MemcacheDBInit() {
    if (start_assoc_maintenance_thread() == -1) {
        exit(EXIT_FAILURE);
    }

    if (start_lru_crawler_ && start_item_crawler_thread() != 0) {
        fprintf(stderr, "Failed to enable LRU crawler thread\n");
        exit(EXIT_FAILURE);
    }

    if (start_lru_maintainer_ && start_lru_maintainer_thread() != 0) {
        fprintf(stderr, "Failed to enable LRU maintainer thread\n");
        return false;
    }

    if (start_slab_maintenance_thread() == -1) {
        exit(EXIT_FAILURE);
    }
    return true;
}

bool MemcacheDB::StopMemcacheDB() {
    stop_slab_maintenance_thread();

    if (stop_lru_maintainer_thread() != 0) {
        fprintf(stderr, "ERROR failed to stop lru thread\n");
    }

    if (stop_item_crawler_thread() != 0) {
        fprintf(stderr, "ERROR failed to stop lru crawler thread\n");
    }

    stop_assoc_maintenance_thread()
    return true;
}

bool MemcacheDB::OpenDB() {
    return true;
}

bool MemcacheDB::Put(WriteOptions& w_options, const char* key, const char* value) {
    int32_t req_cas_id = w_options.cas_id;
    time_t exptime = w_options.exptime;

    int32_t klen = strlen(key);
    int32_t vlen = strlen(value);
    vlen += 2;
    item* it = item_alloc(key, klen, flags, realtime(exptime), vlen);
    if (NULL == it) {
        it = item_get(key, nkey);
        if (NULL != it) {
            item_unlink(it);
            item_remove(it);
        }
        return false;
    }
    ITEM_set_cas(it, req_cas_id);

    char* data = ITEM_data(it);
    memmove(data, value, vlen);

    protocol prot = w_options.prot;
    if (ascii_prot == prot) {
        strore_item(it, NREAD_REPLACE);
        item_remove(it);
    } else if (binary_prot == prot) {
        /* We don't actually receive the trailing two characters in the bin
         * protocol, so we're going to just set them here */
        *(ITEM_data(it) + it->nbytes - 2) = '\r';
        *(ITEM_data(it) + it->nbytes - 1) = '\n';
        strore_item(it, NREAD_REPLACE);
    }

    return true;
}

bool MemcacheDB::Get(ReadOptions& r_options, const char* key, std::string& value) {
    //int32_t req_cas_id = w_options.cas_id;
    bool need_cas = r_options.need_cas;
    int32_t klen = strlen(key);
    int32_t vlen = strlen(value);

    item* it = item_get(key, nkey);
    if (NULL == it) {
        return false;
    }

    if (need_cas) {
        value.assign(ITEM_data(it), it->nbytes);
    } else {
        value.assign(ITEM_suffix(it), it->nsuffix + it->nbytes);
    }
    item_remove(it);
    item_update(it);
    return true;
}

bool MemcacheDB::Delete(WriteOptions& w_options, const char* key) {
    int32_t klen = strlen(key);
    int32_t vlen = strlen(value);

    item* it = item_get(key, nkey);
    if (NULL == it) {
        return false;
    }
    item_unlink(it);
    item_remove(it);      /* release our reference */
    return true;
}




/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
