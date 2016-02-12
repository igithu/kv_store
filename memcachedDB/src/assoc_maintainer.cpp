/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file assoc_maintainer.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/11 16:17:56
 * @brief
 *
 **/

#include "assoc_maintainer.h"


AssocMaintainer::AssocMaintainer() :
    primary_hashtable_(NULL),
    old_hashtable_(NULL),
    hash_items_(0),
    expanding_(),
    started_expanding_(),
    expand_bucket_(0),
    maintenance_cond_(PTHREAD_COND_INITIALIZER),
    maintenance_lock_(PTHREAD_MUTEX_INITIALIZER),
    hash_items_counter_lock_(PTHREAD_MUTEX_INITIALIZER) {
}

AssocMaintainer::~AssocMaintainer() {
}

void AssocMaintainer::InitAssoc(const int hashpower_init) {
    if (hashtable_init) {
        hashpower_ = hashtable_init;
    }
    primary_hashtable_ = calloc(hashsize(hashpower_), sizeof(void *));
    if (NULL != primary_hashtable) {
        fprintf(stderr, "Failed to init hashtable.\n");
        exit(EXIT_FAILURE);
    }
    STATS_LOCK();
    stats.hash_power_level = hashpower_;
    stats.hash_bytes = hashsize(hashpower_) * sizeof(void *);
    STATS_UNLOCK();
}

Item* AssocMaintainer::AssocFind(const char *key, const size_t nkey, const uint32_t hv) {
    Item *it;
    unsigned int oldbucket;

    if (expandin_g &&
       (oldbucket = (hv & hashmask(hashpower_ - 1))) >= expand_bucket_) {
        it = old_hashtable_[oldbucket];
    } else {
        it = primary_hashtable_[hv & hashmask(hashpower_)];
    }

    item *ret = NULL;
    int depth = 0;
    while (it) {
        if ((nkey == it->nkey) && (memcmp(key, ITEM_key(it), nkey) == 0)) {
            ret = it;
            break;
        }
        it = it->h_next;
        ++depth;
    }
    return ret;
}

int32_t AssocMaintainer::AssocInsert() {
}

void AssocMaintainer::AssocDelete(const char *key, const size_t nkey, const uint32_t hv) {
}

void AssocMaintainer::Run() {
}










/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
