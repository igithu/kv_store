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


MemcacheDB::MemcacheDB() : prot_(ascii_prot), start_lru_crawler_(false), start_lru_maintainer_(false) {
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
    } else if (binary_prot == prot) {
    }


/*
    char* command = (char*)malloc(sizeof(key) + sizeof(value) + 100);
     * <command name> <key> <flags> <exptime> <bytes>\r\n
    int32_t vlen = sizeof(value);
    sprintf(command, "set %s 0 0 %d\r\n", key, vlen);
    conn c;
    process_command(&c, command);
    if (vlen >= c.rlbytes) {
        fprintf(stderr, "Invalid rlbytes to read: len %d\n", c.rlbytes);
        return false;
    }
    memmove(c.ritem, value, c.rlbytes);
    c.protocol = prot_;
    complete_nread(&c);
*/
/*
    conn c;
    if (ascii_prot == prot_) {
        complete_nread_ascii(c);
    } else if (binary_prot == prot_) {
        complete_nread_binary(c);
    }
*/
    return true;
}

bool MemcacheDB::Get(ReadOptions& r_options, const char* key, std::string& value) {
    char* command = (char*)malloc(sizeof(key) + sizeof(value) + 100);
    sprintf(command, "get %s\r\n", key);
    conn c;
    process_command(&c, command);

/*
    bool transmit_running = true
    while (transmit_running) {
        transmit_result trans_res = transmit(c);
        switch (trans_res) {
            case TRANSMIT_COMPLETE:
                conn_release_items(&c)
                break;
            case TRANSMIT_SOFT_ERROR:
                return false;
            case TRANSMIT_INCOMPLETE:
            case TRANSMIT_HARD_ERROR:
                transmit_running = false;
                break;
        }
    }
*/
    return true;
}

bool MemcacheDB::Delete(WriteOptions& w_options, const char* key) {
    char* command = (char*)malloc(sizeof(key) + sizeof(value) + 100);
    sprintf(command, "delete %s\r\n", key);
    conn c;
    process_command(&c, command);
    return true;
}




/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
