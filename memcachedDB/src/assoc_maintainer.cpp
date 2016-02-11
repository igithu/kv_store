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
    expand_bucket(0),
    maintenance_cond_(PTHREAD_COND_INITIALIZER),
    maintenance_lock_(PTHREAD_MUTEX_INITIALIZER),
    hash_items_counter_lock_(PTHREAD_MUTEX_INITIALIZER) {
}

AssocMaintainer::~AssocMaintainer() {
}

void AssocMaintainer::InitAssoc(const int hashpower_init) {
}

Item* AssocMaintainer::AssocFind(const char *key, const size_t nkey, const uint32_t hv) {
}

int32_t AssocMaintainer::AssocInsert() {
}

void AssocMaintainer::AssocDelete(const char *key, const size_t nkey, const uint32_t hv) {
}

void AssocMaintainer::Run() {
}










/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
