/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file item_manager.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/09 23:29:38
 * @brief
 *
 **/



#ifndef __ITEM_MAINTAINER_H
#define __ITEM_MAINTAINER_H

#include <atomic.h>

#include<ev.h>

#include "disallow_copy_and_assign.h"

/* unistd.h is here */
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

/*
 * Maximum length of a key.
 */
#define KEY_MAX_LENGTH 250

/*
 * Size of an incr buf.
 */
#define INCR_MAX_STORAGE_LEN 24

/*
 * Initial size of list of items being returned by "get".
 */
#define ITEM_LIST_INITIAL 200

/*
 * Binary protocol stuff
 */
#define MIN_BIN_PKT_LENGTH 16
#define BIN_PKT_HDR_WORDS (MIN_BIN_PKT_LENGTH/sizeof(uint32_t))
/*
 * We only reposition items in the LRU queue if they haven't been repositioned
 * in this many seconds. That saves us from churning on frequently-accessed
 * items.
 */
#define ITEM_UPDATE_INTERVAL 60
/*
 * Slab sizing definitions.
 */
#define POWER_SMALLEST 1
#define POWER_LARGEST  256 /* actual cap is 255 */
#define CHUNK_ALIGN_BYTES 8
/* slab class max is a 6-bit number, -1. */
#define MAX_NUMBER_OF_SLAB_CLASSES (63 + 1)

#define STAT_KEY_LEN 128
#define STAT_VAL_LEN 128

#define ITEM_LINKED 1
#define ITEM_CAS 2

/* temp */
#define ITEM_SLABBED 4

/* Item was fetched at least once in its lifetime */
#define ITEM_FETCHED 8
/* Appended on fetch, removed on LRU shuffling */
#define ITEM_ACTIVE 16

#define LARGEST_ID POWER_LARGEST

/*
 * warning: don't use these macros with a function, as it evals its arg twice
 */
#define ITEM_get_cas(i) (((i)->it_flags & ITEM_CAS) ? \
        (i)->data->cas : (uint64_t)0)

#define ITEM_set_cas(i,v) { \
    if ((i)->it_flags & ITEM_CAS) { \
        (i)->data->cas = v; \
    } \
}

#define ITEM_key(item) (((char*)&((item)->data)) \
         + (((item)->it_flags & ITEM_CAS) ? sizeof(uint64_t) : 0))

#define ITEM_suffix(item) ((char*) &((item)->data) + (item)->nkey + 1 \
         + (((item)->it_flags & ITEM_CAS) ? sizeof(uint64_t) : 0))

#define ITEM_data(item) ((char*) &((item)->data) + (item)->nkey + 1 \
         + (item)->nsuffix \
         + (((item)->it_flags & ITEM_CAS) ? sizeof(uint64_t) : 0))

#define ITEM_ntotal(item) (sizeof(struct _stritem) + (item)->nkey + 1 \
         + (item)->nsuffix + (item)->nbytes \
         + (((item)->it_flags & ITEM_CAS) ? sizeof(uint64_t) : 0))

#define ITEM_clsid(item) ((item)->slabs_clsid & ~(3<<6))

/** Append an indexed stat with a stat name (with format), value format
    and value */
#define APPEND_NUM_FMT_STAT(name_fmt, num, name, fmt, val)          \
    klen = snprintf(key_str, STAT_KEY_LEN, name_fmt, num, name);    \
    vlen = snprintf(val_str, STAT_VAL_LEN, fmt, val);               \
    add_stats(key_str, klen, val_str, vlen, c);

/** Common APPEND_NUM_FMT_STAT format. */
#define APPEND_NUM_STAT(num, name, fmt, val) \
    APPEND_NUM_FMT_STAT("%d:%s", num, name, fmt, val)

/**
 * Callback for any function producing stats.
 *
 * @param key the stat's key
 * @param klen length of the key
 * @param val the stat's value in an ascii form (e.g. text form of a number)
 * @param vlen length of the value
 * @parm cookie magic callback cookie
 */
typedef void (*ADD_STAT)(const char *key, const uint16_t klen,
                         const char *val, const uint32_t vlen,
                         const void *cookie);

/*
 * Time relative to server start. Smaller than time_t on 64-bit systems.
 */
typedef unsigned std::atomic<int> rel_time_t;

enum Protocol {
    ascii_prot = 3, /* arbitrary value. */
    binary_prot,
    negotiating_prot /* Discovering the protocol */
};

enum DeltaResultType {
    OK, NON_NUMERIC, EOM, DELTA_ITEM_NOT_FOUND, DELTA_ITEM_CAS_MISMATCH
};

enum StoreItemType {
    NOT_STORED = 0, STORED, EXISTS, NOT_FOUND
};

enum NreadOpType {
    NREAD_ADD = 1, NREAD_SET, NREAD_REPLACE, NREAD_APPEND, NREAD_PREPEND, NREAD_CAS
};

/*
 * Structure for storing items within memcached.
 */
struct Item {
    /*
     * Protected by LRU locks
     */
    struct _stritem *next;
    struct _stritem *prev;
    /*
     * Rest are protected by an item lock
     */
    struct _stritem *h_next;    /* hash chain next */
    rel_time_t      time;       /* least recent access */
    rel_time_t      exptime;    /* expire time */
    int             nbytes;     /* size of data */
    unsigned short  refcount;
    uint8_t         nsuffix;    /* length of flags-and-length string */
    uint8_t         it_flags;   /* ITEM_* above */
    uint8_t         slabs_clsid;/* which slab class we're in */
    uint8_t         nkey;       /* key length, w/terminating null and padding */
    /*
     * this odd type prevents type-punning issues when we do
     * the little shuffle to save space when not using CAS.
     */
    union {
        uint64_t cas;
        char end;
    } data[];
    /* if it_flags & ITEM_CAS we have 8 bytes CAS */
    /* then null-terminated key */
    /* then " flags length\r\n" (no terminating null) */
    /* then data with terminating \r\n (no terminating null; it's binary!) */
};


struct ItemStats {
    uint64_t evicted;
    uint64_t evicted_nonzero;
    uint64_t reclaimed;
    uint64_t outofmemory;
    uint64_t tailrepairs;
    uint64_t expired_unfetched;
    uint64_t evicted_unfetched;
    uint64_t crawler_reclaimed;
    uint64_t crawler_items_checked;
    uint64_t lrutail_reflocked;
    uint64_t moves_to_cold;
    uint64_t moves_to_warm;
    uint64_t moves_within_lru;
    uint64_t direct_reclaims;
    rel_time_t evicted_time;
};


class ItemManager {
    public:
        ~ItemManager();

        static ItemManager& GetInstance();

        bool Start();
        // bool SwitchLRUCrawler(bool status);
        // bool SwitchLRUMaintainer(bool status);

        Item *DoItemAlloc(
                char *key,
                const size_t nkey,
                const int flags,
                const rel_time_t exptime,
                const int nbytes,
                const uint32_t cur_hv);

        void FreeItem(Item *it);
        bool ItemSizeOk(const size_t nkey, const int flags, const int nbytes);

        /*
         * may fail if transgresses limits
         */
        int  DoItemLink(Item *it, const uint32_t hv);
        void DoItemUnlink(Item *it, const uint32_t hv);
        void DoItemUnlinkNolock(Item *it, const uint32_t hv);
        void DoItemRemove(Item *it);

        /*
         * update LRU time to current and reposition
         */
        void DoItemUpdate(Item *it);
        void DoItemUpdateNolock(Item *it);
        int  DoItemReplace(Item *it, Item *new_it, const uint32_t hv);

        enum StoreItemType DoStoreItem(const uint32_t hv, Item* it, NreadOpType op);
        Item *DoItemGet(const char *key, const size_t nkey, const uint32_t hv);
        Item *DoItemTouch(const char *key, const size_t nkey, uint32_t exptime, const uint32_t hv);
        void DoItemLinkQ(Item* it, bool is_crawler = false);
        void DoItemUnlinkQ(Item* it, bool is_crawler = false);

        char *ItemCacheDump(const unsigned int slabs_clsid, const unsigned int limit, unsigned int *bytes);
        void ItemStats(ADD_STAT add_stats, void *c);
        void ItemStatsTotals(ADD_STAT add_stats, void *c);
        void ItemStatsSizes(ADD_STAT add_stats, void *c);

        /*
         * item_sizes_ interface
         */
        int32_t GetItemSizeByIndex(int32_t index);
        void ItemSizeIncrement(int32_t index);
        void ItemSizeDecrement(int32_t index);

        Item *GetItemHeadByIndex(int32_t index);
        Item *GetItemTailByIndex(int32_t index);
        void ItemLinkQ(Item* it);
        void ItemUnlinkQ(Item* it);

        void CacheLock(int32_t lock_id);
        void CacheUnlock(int32_t lock_id);

        bool ItemEvaluate(Item *eval_item, uint32_t hv, int32_t is_index);

        void Lock(uint32_t hv);
        void *TryLock(uint32_t hv);
        void TryLockUnlock(void *arg);
        void Unlock(uint32_t hv);

    private:
        ItemManager();

        Item *ItemAlloc(char *key, size_t nkey, int flags, rel_time_t exptime, int nbytes);
        /*
         * Returns an item if it hasn't been marked as expired,
         * lazy-expiring as needed.
         */
        Item *ItemGet(const char *key, const size_t nkey);
        Item *ItemTouch(const char *key, const size_t nkey, uint32_t exptime);

        /*
         * Links an item into the LRU and hashtable.
         */
        int   ItemLink(Item *it);
        /*
         * Decrements the reference count on an item and adds it to the freelist if
         * needed.
         */
        void  ItemRemove(Item *it);
        /*
         * Replaces one item with another in the hashtable.
         * Unprotected by a mutex lock since the core server does not require
         * it to be thread-safe.
         */
        int ItemReplace(Item *it, Item *new_it, const uint32_t hv);
        /*
         * Unlinks an item from the LRU and hashtable.
         */
        void  ItemUnlink(Item *it);
        /*
         * Moves an item to the back of the LRU queue.
         */
        void ItemUpdate(Item *it);

        /*
         * Stores an item in the cache (high level, obeys set/add/replace semantics)
         */
        enum StoreItemType StoreItem(Item *item, NreadOpType op);

        rel_time_t GetCurrentTime();

        bool IsFlushed(Item* it);
        size_t ItemMakeHeader(
                const uint8_t nkey,
                const int flags,
                const int nbytes,
                char *suffix,
                uint8_t *nsuffix);

        /*
         * Returns number of items remove, expired, or evicted.
         */
        int32_t ItemLRUPullTail(
                const int orig_id,
                const LRUStatus cur_lru,
                const unsigned int total_chunks,
                const bool do_evict,
                const uint32_t cur_hv);


        static void ClockHandler(struct ev_loop *loop, ev_timer *timer_w,int e);

        DISALLOW_COPY_AND_ASSIGN(ItemManager);

    private:
        Item **heads_;
        Item **tails_;

        unsigned int *item_sizes_;
        ItemStats *item_stats_;
        pthread_mutex_t *cache_locks_;

        rel_time_t current_time_;
        static struct ev_loop *time_loop_;
        static ev_timer timer_w_;

        pthread_mutex_t *item_locks_;

        bool start_lru_crawler_;
        bool start_lru_maintainer_;
};





#endif // __ITEM_MAINTAINER_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
