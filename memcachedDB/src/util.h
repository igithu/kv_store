/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file util.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/10 22:07:53
 * @brief
 *
 **/




#ifndef __UTIL_H
#define __UTIL_H

void Lock(uint32_t hv);
void *TryLock(uint32_t hv);
void TryLockUnlock(void *arg);
void Unlock(uint32_t hv);

unsigned short RefcountIncr(unsigned short *refcount);
unsigned short RefcountDecr(unsigned short *refcount);
void StatsLock(void);
void StatsUnlock(void);

/*
 * Get the next CAS id for a new item.
 * TODO: refactor some atomics for this.
 */
uint64_t GetCasId();


/*
 * stats interface
 */
void stats_prefix_init(void);
void stats_prefix_clear(void);
void stats_prefix_record_get(const char *key, const size_t nkey, const bool is_hit);
void stats_prefix_record_delete(const char *key, const size_t nkey);
void stats_prefix_record_set(const char *key, const size_t nkey);
/*@null@*/
char *stats_prefix_dump(int *length);

#endif // __UTIL_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
