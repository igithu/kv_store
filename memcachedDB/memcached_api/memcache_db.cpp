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

void MemcacheDB::SetProtocol(protocol prot) {
    prot_ = prot;
}

bool MemcacheDB::Put(const char* key, const char* value) {
    char* command = (char*)malloc(sizeof(key) + sizeof(value) + 100);
    /*
     * <command name> <key> <flags> <exptime> <bytes>\r\n
     */
    int32_t vlen = sizeof(value);
    sprintf(command, "set %s 0 0 %d\r\n", key, vlen);
    conn c;
    process_command(&c, command);
    if (vlen >= c.rlbytes) {
        fprintf(stderr, "Invalid rlbytes to read: len %d\n", c.rlbytes);
        return false;
    }
    memmove(c.ritem, value, c.rlbytes);
    complete_nread(&c);
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

bool MemcacheDB::Get(const char* key, std::string& value) {
    char* command = (char*)malloc(sizeof(key) + sizeof(value) + 100);
    sprintf(command, "get %s\r\n", key);
    conn c;
    process_command(&c, command);

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
    return true;
}

bool MemcacheDB::Delete(const char* key) {
    return true;
}




/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
