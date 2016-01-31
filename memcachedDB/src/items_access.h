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
// #include "slabs.h"
// #include "assoc.h"
#include "items.h"
#include "trace.h"
#include "hash.h"
#include "util.h"
#include "protocol_binary.h"

#define NREAD_ADD 1
#define NREAD_SET 2
#define NREAD_REPLACE 3
#define NREAD_APPEND 4
#define NREAD_PREPEND 5
#define NREAD_CAS 6

// /* warning: don't use these macros with a function, as it evals its arg twice */
// #define ITEM_get_cas(i) (((i)->it_flags & ITEM_CAS) ? \
//         (i)->data->cas : (uint64_t)0)
// 
// #define ITEM_set_cas(i,v) { \
//     if ((i)->it_flags & ITEM_CAS) { \
//         (i)->data->cas = v; \
//     } \
// }
// 
// #define ITEM_key(item) (((char*)&((item)->data)) \
//          + (((item)->it_flags & ITEM_CAS) ? sizeof(uint64_t) : 0))
// 
// #define ITEM_suffix(item) ((char*) &((item)->data) + (item)->nkey + 1 \
//          + (((item)->it_flags & ITEM_CAS) ? sizeof(uint64_t) : 0))
// 
// #define ITEM_data(item) ((char*) &((item)->data) + (item)->nkey + 1 \
//          + (item)->nsuffix \
//          + (((item)->it_flags & ITEM_CAS) ? sizeof(uint64_t) : 0))
// 
// #define ITEM_ntotal(item) (sizeof(struct _stritem) + (item)->nkey + 1 \
//          + (item)->nsuffix + (item)->nbytes \
//          + (((item)->it_flags & ITEM_CAS) ? sizeof(uint64_t) : 0))
// 
// #define ITEM_clsid(item) ((item)->slabs_clsid & ~(3<<6))
// 
// #define STAT_KEY_LEN 128
// #define STAT_VAL_LEN 128
// 
// /** Append a simple stat with a stat name, value format and value */
// #define APPEND_STAT(name, fmt, val) \
//     append_stat(name, add_stats, c, fmt, val);
// 
// /** Append an indexed stat with a stat name (with format), value format
//     and value */
// #define APPEND_NUM_FMT_STAT(name_fmt, num, name, fmt, val)          \
//     klen = snprintf(key_str, STAT_KEY_LEN, name_fmt, num, name);    \
//     vlen = snprintf(val_str, STAT_VAL_LEN, fmt, val);               \
//     add_stats(key_str, klen, val_str, vlen, c);
// 
// /** Common APPEND_NUM_FMT_STAT format. */
// #define APPEND_NUM_STAT(num, name, fmt, val) \
//     APPEND_NUM_FMT_STAT("%d:%s", num, name, fmt, val)
// 
// /**
//  * Callback for any function producing stats.
//  *
//  * @param key the stat's key
//  * @param klen length of the key
//  * @param val the stat's value in an ascii form (e.g. text form of a number)
//  * @param vlen length of the value
//  * @parm cookie magic callback cookie
//  */
// typedef void (*ADD_STAT)(const char *key, const uint16_t klen,
//                          const char *val, const uint32_t vlen,
//                          const void *cookie);
// 
// 
// 
// /** Time relative to server start. Smaller than time_t on 64-bit systems. */
// typedef unsigned int rel_time_t;
// 
// 
// enum protocol {
//     ascii_prot = 3, /* arbitrary value. */
//     binary_prot,
//     negotiating_prot /* Discovering the protocol */
// };
// 
// enum store_item_type {
//     NOT_STORED=0, STORED, EXISTS, NOT_FOUND
// };
// 
// /** Stats stored per slab (and per thread). */
// struct slab_stats {
//     uint64_t  set_cmds;
//     uint64_t  get_hits;
//     uint64_t  touch_hits;
//     uint64_t  delete_hits;
//     uint64_t  cas_hits;
//     uint64_t  cas_badval;
//     uint64_t  incr_hits;
//     uint64_t  decr_hits;
// };
// 
// /**
//  * Global stats.
//  */
// struct stats {
//     pthread_mutex_t mutex;
//     unsigned int  curr_items;
//     unsigned int  total_items;
//     uint64_t      curr_bytes;
//     unsigned int  curr_conns;
//     unsigned int  total_conns;
//     uint64_t      rejected_conns;
//     uint64_t      malloc_fails;
//     unsigned int  reserved_fds;
//     unsigned int  conn_structs;
//     uint64_t      get_cmds;
//     uint64_t      set_cmds;
//     uint64_t      touch_cmds;
//     uint64_t      get_hits;
//     uint64_t      get_misses;
//     uint64_t      touch_hits;
//     uint64_t      touch_misses;
//     uint64_t      evictions;
//     uint64_t      reclaimed;
//     time_t        started;          /* when the process was started */
//     bool          accepting_conns;  /* whether we are currently accepting */
//     uint64_t      listen_disabled_num;
//     unsigned int  hash_power_level; /* Better hope it's not over 9000 */
//     uint64_t      hash_bytes;       /* size used for hash tables */
//     bool          hash_is_expanding; /* If the hash table is being expanded */
//     uint64_t      expired_unfetched; /* items reclaimed but never touched */
//     uint64_t      evicted_unfetched; /* items evicted but never touched */
//     bool          slab_reassign_running; /* slab reassign in progress */
//     uint64_t      slabs_moved;       /* times slabs were moved around */
//     uint64_t      lru_crawler_starts; /* Number of item crawlers kicked off */
//     bool          lru_crawler_running; /* crawl in progress */
//     uint64_t      lru_maintainer_juggles; /* number of LRU bg pokes */
// };
// 
// /**
//  * Global stats.
//  */
// struct stats {
//     pthread_mutex_t mutex;
//     unsigned int  curr_items;
//     unsigned int  total_items;
//     uint64_t      curr_bytes;
//     unsigned int  curr_conns;
//     unsigned int  total_conns;
//     uint64_t      rejected_conns;
//     uint64_t      malloc_fails;
//     unsigned int  reserved_fds;
//     unsigned int  conn_structs;
//     uint64_t      get_cmds;
//     uint64_t      set_cmds;
//     uint64_t      touch_cmds;
//     uint64_t      get_hits;
//     uint64_t      get_misses;
//     uint64_t      touch_hits;
//     uint64_t      touch_misses;
//     uint64_t      evictions;
//     uint64_t      reclaimed;
//     time_t        started;          /* when the process was started */
//     bool          accepting_conns;  /* whether we are currently accepting */
//     uint64_t      listen_disabled_num;
//     unsigned int  hash_power_level; /* Better hope it's not over 9000 */
//     uint64_t      hash_bytes;       /* size used for hash tables */
//     bool          hash_is_expanding; /* If the hash table is being expanded */
//     uint64_t      expired_unfetched; /* items reclaimed but never touched */
//     uint64_t      evicted_unfetched; /* items evicted but never touched */
//     bool          slab_reassign_running; /* slab reassign in progress */
//     uint64_t      slabs_moved;       /* times slabs were moved around */
//     uint64_t      lru_crawler_starts; /* Number of item crawlers kicked off */
//     bool          lru_crawler_running; /* crawl in progress */
//     uint64_t      lru_maintainer_juggles; /* number of LRU bg pokes */
// };
// 
// #define MAX_VERBOSITY_LEVEL 2
// 
// /* When adding a setting, be sure to update process_stat_settings */
// /**
//  * Globally accessible settings as derived from the commandline.
//  */
// struct settings {
//     size_t maxbytes;
//     int maxconns;
//     int port;
//     int udpport;
//     char *inter;
//     int verbose;
//     rel_time_t oldest_live; /* ignore existing items older than this */
//     uint64_t oldest_cas; /* ignore existing items with CAS values lower than this */
//     int evict_to_free;
//     char *socketpath;   /* path to unix socket if using local socket */
//     int access;  /* access mask (a la chmod) for unix domain socket */
//     double factor;          /* chunk size growth factor */
//     int chunk_size;
//     int num_threads;        /* number of worker (without dispatcher) libevent threads to run */
//     int num_threads_per_udp; /* number of worker threads serving each udp socket */
//     char prefix_delimiter;  /* character that marks a key prefix (for stats) */
//     int detail_enabled;     /* nonzero if we're collecting detailed stats */
//     int reqs_per_event;     /* Maximum number of io to process on each
//                                io-event. */
//     bool use_cas;
//     enum protocol binding_protocol;
//     int backlog;
//     int item_size_max;        /* Maximum item size, and upper end for slabs */
//     bool sasl;              /* SASL on/off */
//     bool maxconns_fast;     /* Whether or not to early close connections */
//     bool lru_crawler;        /* Whether or not to enable the autocrawler thread */
//     bool lru_maintainer_thread; /* LRU maintainer background thread */
//     bool slab_reassign;     /* Whether or not slab reassignment is allowed */
//     int slab_automove;     /* Whether or not to automatically move slabs */
//     int hashpower_init;     /* Starting hash power level */
//     bool shutdown_command; /* allow shutdown command */
//     int tail_repair_time;   /* LRU tail refcount leak repair time */
//     bool flush_enabled;     /* flush_all enabled */
//     char *hash_algorithm;     /* Hash algorithm in use */
//     int lru_crawler_sleep;  /* Microsecond sleep between items */
//     uint32_t lru_crawler_tocrawl; /* Number of items to crawl per run */
//     int hot_lru_pct; /* percentage of slab space for HOT_LRU */
//     int warm_lru_pct; /* percentage of slab space for WARM_LRU */
//     int crawls_persleep; /* Number of LRU crawls to run before sleeping */
//     bool expirezero_does_not_evict; /* exptime == 0 goes into NOEXP_LRU */
// };
// 
// /**
//  * Structure for storing items within memcached.
//  */
// typedef struct _stritem {
//     /* Protected by LRU locks */
//     struct _stritem *next;
//     struct _stritem *prev;
//     /* Rest are protected by an item lock */
//     struct _stritem *h_next;    /* hash chain next */
//     rel_time_t      time;       /* least recent access */
//     rel_time_t      exptime;    /* expire time */
//     int             nbytes;     /* size of data */
//     unsigned short  refcount;
//     uint8_t         nsuffix;    /* length of flags-and-length string */
//     uint8_t         it_flags;   /* ITEM_* above */
//     uint8_t         slabs_clsid;/* which slab class we're in */
//     uint8_t         nkey;       /* key length, w/terminating null and padding */
//     /* this odd type prevents type-punning issues when we do
//      * the little shuffle to save space when not using CAS. */
//     union {
//         uint64_t cas;
//         char end;
//     } data[];
//     /* if it_flags & ITEM_CAS we have 8 bytes CAS */
//     /* then null-terminated key */
//     /* then " flags length\r\n" (no terminating null) */
//     /* then data with terminating \r\n (no terminating null; it's binary!) */
// } item;
// 
typedef struct {
    struct _stritem *next;
    struct _stritem *prev;
    struct _stritem *h_next;    /* hash chain next */
    rel_time_t      time;       /* least recent access */
    rel_time_t      exptime;    /* expire time */
    int             nbytes;     /* size of data */
    unsigned short  refcount;
    uint8_t         nsuffix;    /* length of flags-and-length string */
    uint8_t         it_flags;   /* ITEM_* above */
    uint8_t         slabs_clsid;/* which slab class we're in */
    uint8_t         nkey;       /* key length, w/terminating null and padding */
    uint32_t        remaining;  /* Max keys to crawl per slab per invocation */
} crawler;

struct slab_rebalance {
    void *slab_start;
    void *slab_end;
    void *slab_pos;
    int s_clsid;
    int d_clsid;
    int busy_items;
    uint8_t done;
};


#define mutex_lock(x) pthread_mutex_lock(x)
#define mutex_unlock(x) pthread_mutex_unlock(x)

void item_lock(uint32_t hv);
void *item_trylock(uint32_t hv);
void item_trylock_unlock(void *arg);
void item_unlock(uint32_t hv);

unsigned short refcount_incr(unsigned short *refcount);
unsigned short refcount_decr(unsigned short *refcount);
void STATS_LOCK(void);
void STATS_UNLOCK(void);

item *item_alloc(char *key, size_t nkey, int flags, rel_time_t exptime, int nbytes);
item *item_get(const char *key, const size_t nkey);
item *item_touch(const char *key, const size_t nkey, uint32_t exptime);

int   item_link(item *it);
void  item_remove(item *it);
int   item_replace(item *it, item *new_it, const uint32_t hv);
void  item_unlink(item *it);
void  item_update(item *it);

enum store_item_type store_item(item *item, int op);

static rel_time_t realtime(const time_t exptime);

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
