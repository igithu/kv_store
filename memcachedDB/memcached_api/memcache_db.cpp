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


MemcacheDB::MemcacheDB() : prot_(ascii_prot), start_lru_crawler(false), start_lru_maintainer(false) {
}

MemcacheDB::~MemcacheDB() {
}

bool MemcacheDB::MemcacheDBInit() {
    if (start_assoc_maintenance_thread() == -1) {
        exit(EXIT_FAILURE);
    }

    if (start_lru_crawler && start_item_crawler_thread() != 0) {
        fprintf(stderr, "Failed to enable LRU crawler thread\n");
        exit(EXIT_FAILURE);
    }

    if (start_lru_maintainer && start_lru_maintainer_thread() != 0) {
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

Status MemcacheDB::Put(const char* key, const char* value) {
    char* command = (char*)malloc(sizeof(key) + sizeof(value) + 100);
    /*
     * <command name> <key> <flags> <exptime> <bytes>\r\n
     */
    sprintf(command, "set %s\r\n%s", key, value);
    conn c;
    process_command(&c, command);
/*
    conn c;
    if (ascii_prot == prot_) {
        complete_nread_ascii(c);
    } else if (binary_prot == prot_) {
        complete_nread_binary(c);
    }
*/
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
