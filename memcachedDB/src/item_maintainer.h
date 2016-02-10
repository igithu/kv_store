/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file item_maintainer.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/09 23:29:38
 * @brief
 *
 **/




#ifndef __ITEM_MAINTAINER_H
#define __ITEM_MAINTAINER_H

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
 * I'm told the max length of a 64-bit num converted to string is 20 bytes.
 * Plus a few for spaces, \r\n, \0
 */
#define SUFFIX_SIZE 24

/*
 * Initial size of list of items being returned by "get".
 */
#define ITEM_LIST_INITIAL 200
/*
 * Initial size of list of CAS suffixes appended to "gets" lines.
 */
#define SUFFIX_LIST_INITIAL 20

/*
 * Initial size of the sendmsg() scatter/gather array.
 */
#define IOV_LIST_INITIAL 400

/*
 * Initial number of sendmsg() argument structures to allocate.
 */
#define MSG_LIST_INITIAL 10

/*
 * High water marks for buffer shrinking
 */
#define ITEM_LIST_HIGHWAT 400
#define IOV_LIST_HIGHWAT 600
#define MSG_LIST_HIGHWAT 100
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

/** How long an object can reasonably be assumed to be locked before
    harvesting it on a low memory condition. Default: disabled. */
#define TAIL_REPAIR_TIME_DEFAULT 0

#define STAT_KEY_LEN 128
#define STAT_VAL_LEN 128




/* warning: don't use these macros with a function, as it evals its arg twice */
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
typedef unsigned int rel_time_t;


enum StoreItemType {
    NOT_STORED=0, STORED, EXISTS, NOT_FOUND
};

class ItemMaintainer : public Thread {
    public:
        ItemMaintainer();
        ~ItemMaintainer();

        item *DoItemAlloc(
                char *key,
                const size_t nkey,
                const int flags,
                const rel_time_t exptime,
                const int nbytes,
                const uint32_t cur_hv);

        void FreeItem(item *it);
        bool ItemSizeOk(const size_t nkey, const int flags, const int nbytes);

        /*
         * may fail if transgresses limits
         */
        int  DoItemLink(item *it, const uint32_t hv);
        void DoItemUnlink(item *it, const uint32_t hv);
        void DoItemUnlinkNolock(item *it, const uint32_t hv);
        void DoItemRemove(item *it);

        /*
         * update LRU time to current and reposition
         */
        void DoItemUpdate(item *it);
        void DoItemUpdateNolock(item *it);
        int  DoItemReplace(item *it, item *new_it, const uint32_t hv);

        enum StoreItemType DoStoreItem(const uint32_t hv, item* it, int32_t op);
        item *DoItemGet(const char *key, const size_t nkey, const uint32_t hv);
        item *DoItemTouch(const char *key, const size_t nkey, uint32_t exptime, const uint32_t hv);

        char *ItemCacheDump(const unsigned int slabs_clsid, const unsigned int limit, unsigned int *bytes);
        void ItemStats(ADD_STAT add_stats, void *c);
        void ItemStatsTotals(ADD_STAT add_stats, void *c);
        void ItemStatsSizes(ADD_STAT add_stats, void *c);

    private:
        item *heads[LARGEST_ID];
        item *tails[LARGEST_ID];
        pthread_mutex_t cas_id_lock; //  = PTHREAD_MUTEX_INITIALIZER;
};





#endif // __ITEM_MAINTAINER_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */