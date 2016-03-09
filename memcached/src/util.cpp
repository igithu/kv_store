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

#include <pthread.h>
#ifdef __sun
#include <atomic.h>
#endif

static pthread_mutex_t cas_id_lock = PTHREAD_MUTEX_INITIALIZER;

#if !defined(HAVE_GCC_ATOMICS) && !defined(__sun)
static pthread_mutex_t atomics_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/*
 * Lock for global stats
 */
static pthread_mutex_t stats_lock = PTHREAD_MUTEX_INITIALIZER;

unsigned short RefcountIncr(unsigned short *refcount) {
#ifdef HAVE_GCC_ATOMICS
    return __sync_add_and_fetch(refcount, 1);
#elif defined(__sun)
    return atomic_inc_ushort_nv(refcount);
#else
    unsigned short res;
    pthread_mutex_lock(&atomics_mutex);
    (*refcount)++;
    res = *refcount;
    pthread_mutex_unlock(&atomics_mutex);
    return res;
#endif
}

unsigned short RefcountDecr(unsigned short *refcount) {
#ifdef HAVE_GCC_ATOMICS
    return __sync_sub_and_fetch(refcount, 1);
#elif defined(__sun)
    return atomic_dec_ushort_nv(refcount);
#else
    unsigned short res;
    pthread_mutex_lock(&atomics_mutex);
    (*refcount)--;
    res = *refcount;
    pthread_mutex_unlock(&atomics_mutex);
    return res;
#endif
}

void StatsLock() {
    pthread_mutex_lock(&stats_lock);
}

void StatsUnlock() {
    pthread_mutex_unlock(&stats_lock);
}

uint64_t GetCasId() {
    static uint64_t cas_id = 0;
    pthread_mutex_lock(&cas_id_lock);
    uint64_t next_id = ++cas_id;
    pthread_mutex_unlock(&cas_id_lock);
    return next_id;
}







/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
