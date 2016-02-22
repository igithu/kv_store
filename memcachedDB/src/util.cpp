/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file util.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/22 23:45:44
 * @brief
 *
 **/

#include "util.h"

static pthread_mutex_t cas_id_lock = PTHREAD_MUTEX_INITIALIZER;

void Lock(uint32_t hv) {
}

void *TryLock(uint32_t hv) {
}

void TryLockUnlock(void *arg) {
}

void Unlock(uint32_t hv) {
}

unsigned short RefcountIncr(unsigned short *refcount) {
}

unsigned short RefcountDecr(unsigned short *refcount) {
}

void StatsLock() {
}

void StatsUnlock() {
}

uint64_t GetCasId() {
    static uint64_t cas_id = 0;
    pthread_mutex_lock(&cas_id_lock);
    uint64_t next_id = ++cas_id;
    pthread_mutex_unlock(&cas_id_lock);
    return next_id;
}







/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
