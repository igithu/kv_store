/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file items_access.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/01/27 00:04:05
 * @brief
 *
 **/




#ifndef __ITEMS_ACCESS_H
#define __ITEMS_ACCESS_H

#include "stats.h"
#include "slabs.h"
#include "assoc.h"
#include "items.h"
#include "trace.h"
#include "hash.h"
#include "util.h"



item *item_alloc(char *key, size_t nkey, int flags, rel_time_t exptime, int nbytes);
item *item_get(const char *key, const size_t nkey);
item *item_touch(const char *key, const size_t nkey, uint32_t exptime);

int   item_link(item *it);
void  item_remove(item *it);
int   item_replace(item *it, item *new_it, const uint32_t hv);

void item_lock(uint32_t hv);
void *item_trylock(uint32_t hv);
void item_trylock_unlock(void *arg);
void item_unlock(uint32_t hv);

unsigned short refcount_incr(unsigned short *refcount);
unsigned short refcount_decr(unsigned short *refcount);
void STATS_LOCK(void);
void STATS_UNLOCK(void);

enum store_item_type store_item(item *item, int op);

#if HAVE_DROP_PRIVILEGES
extern void drop_privileges(void);
#else
#define drop_privileges()
#endif

/* If supported, give compiler hints for branch prediction. */
#if !defined(__GNUC__) || (__GNUC__ == 2 && __GNUC_MINOR__ < 96)
#define __builtin_expect(x, expected_value) (x)
#endif

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)



#endif // __ITEMS_ACCESS_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
