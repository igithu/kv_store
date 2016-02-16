/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file item_maintainer.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/11 23:40:59
 * @brief
 *
 **/


#include "item_maintainer.h"

static struct ev_loop *ItemMaintainer::time_loop_ = ev_default_loop(0);

ItemMaintainer::ItemMaintainer() {
}

ItemMaintainer::~ItemMaintainer() {
}

ItemMaintainer& ItemMaintainer::GetInstance() {
    static ItemMaintainer im_instance;
    return im_instance;
}

Item *ItemMaintainer::DoItemAlloc(
        char *key,
        const size_t nkey,
        const int flags,
        const rel_time_t exptime,
        const int nbytes,
        const uint32_t cur_hv) {
}

void ItemMaintainer::FreeItem(Item *it) {
}

bool ItemMaintainer::ItemSizeOk(const size_t nkey, const int flags, const int nbytes) {
}

int  ItemMaintainer::DoItemLink(Item *it, const uint32_t hv) {
}

void ItemMaintainer::DoItemUnlink(Item *it, const uint32_t hv) {
}

void ItemMaintainer::DoItemUnlinkNolock(Item *it, const uint32_t hv) {
}

void ItemMaintainer::DoItemRemove(Item *it) {
}

void ItemMaintainer::DoItemUpdate(Item *it) {
}

void ItemMaintainer::DoItemUpdateNolock(Item *it) {
}

int  ItemMaintainer::DoItemReplace(Item *it, Item *new_it, const uint32_t hv) {
}

enum StoreItemType ItemMaintainer::DoStoreItem(const uint32_t hv, Item* it, int32_t op) {
}

Item *ItemMaintainer::DoItemGet(const char *key, const size_t nkey, const uint32_t hv) {
}

Item *ItemMaintainer::DoItemTouch(const char *key, const size_t nkey, uint32_t exptime, const uint32_t hv) {
}

char *ItemMaintainer::ItemCacheDump(const unsigned int slabs_clsid, const unsigned int limit, unsigned int *bytes) {
}

void ItemMaintainer::ItemStats(ADD_STAT add_stats, void *c) {
}

void ItemMaintainer::ItemStatsTotals(ADD_STAT add_stats, void *c) {
}

void ItemMaintainer::ItemStatsSizes(ADD_STAT add_stats, void *c) {
}

Item *ItemMaintainer::GetItemSizeByIndex(int32_t index) {
}

void ItemMaintainer::ItemSizeIncrement(int32_t index) {
}

void ItemMaintainer::ItemSizeDecrement(int32_t index) {
}

Item *ItemMaintainer::GetItemHeadByIndex(int32_t index) {
    pthread_mutex_lock(&lru_locks[index]);
    item* it = heads[index];
    pthread_mutex_lock(&lru_locks[index]);
    return it;
}

Item *ItemMaintainer::GetItemTailByIndex(int32_t index) {
    pthread_mutex_lock(&lru_locks[index]);
    item* it = tails[index];
    pthread_mutex_lock(&lru_locks[index]);
    return it;
}

void ItemMaintainer::ItemLinkQ(Item* it) {
    Item **head, **tail;
    assert(it->it_flags == 1);
    assert(it->nbytes == 0);
    int32_t lock_id = it->slabs_clsid;

    head = &heads[lock_id];
    tail = &tails[lock_id];

    assert(*tail != 0);
    assert(it != *tail);
    assert((*head && *tail) || (*head == 0 && *tail == 0));

    it->prev = *tail;
    it->next = 0;
    if (it->prev) {
        assert(it->prev->next == 0);
        it->prev->next = it;
    }
    *tail = it;
    if (*head == 0) {
        *head = it;
    }
}

void ItemMaintainer::ItemUnlinkQ(Item* it) {
    Item **head, **tail;
    int32_t lock_id = it->slabs_clsid;
    head = &heads_[lock_id];
    tail = &tails[lock_id];

    if (*head == it) {
        assert(it->prev == 0);
        *head = it->next;
    }
    if (*tail == it) {
        assert(it->next == 0);
        *tail = it->prev;
    }
    assert(it->next != it);
    assert(it->prev != it);

    if (it->next) {
        it->next->prev = it->prev;
    }
    if (it->prev) {
        it->prev->next = it->next;
    }
}

void ItemMaintainer::CacheLock(int32_t lock_id) {
    pthread_mutex_lock(&cache_locks_[lock_id]);
}

void ItemMaintainer::CacheUnlock(int32_t lock_id) {
    pthread_mutex_unlock(&cache_locks_[lock_id]);
}

Item *ItemMaintainer::ItemAlloc(char *key, size_t nkey, int flags, rel_time_t exptime, int nbytes) {
}

Item *ItemMaintainer::ItemGet(const char *key, const size_t nkey) {
}

Item *ItemMaintainer::ItemTouch(const char *key, const size_t nkey, uint32_t exptime) {
}

int ItemMaintainer::ItemLink(Item *it) {
}

void ItemMaintainer::ItemRemove(Item *it) {
}

int ItemMaintainer::ItemReplace(Item *it, Item *new_it, const uint32_t hv) {
}

void ItemMaintainer::ItemUnlink(Item *it) {
}

void ItemMaintainer::ItemUpdate(Item *it) {
}

enum StoreItemType ItemMaintainer::StoreItem(Item *item, int op) {
}

rel_time_t ItemMaintainer::GetCurrentTime() {
    return current_time_;
}

void ItemMaintainer::ClockHandler(struct ev_loop *loop,ev_timer *timer_w,int e) {
    struct timeval t = {.tv_sec = 1, .tv_usec = 0};
    static bool initialized = false;
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)
    static bool monotonic = false;
    static time_t monotonic_start;
#endif

    if (!initialized) {
        initialized = true;
        /* process_started is initialized to time() - 2. We initialize to 1 so
         * flush_all won't underflow during tests. */
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
            monotonic = true;
            monotonic_start = ts.tv_sec - ITEM_UPDATE_INTERVAL - 2;
        }
#endif
        ev_init(&timer_w_, ItemMaintainer::ClockHandler);
        ev_timer_set(&timer_w_, 2, 0);
        ev_timer_start(time_loop, &timer_w_);
        ev_run(time_loop, 0);
    }

#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)
    if (monotonic) {
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
            return;
        current_time = (rel_time_t) (ts.tv_sec - monotonic_start);
        return;
    }
#endif
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        current_time = (rel_time_t) (tv.tv_sec - process_started);
    }
}












/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
