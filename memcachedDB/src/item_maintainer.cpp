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

#include "lru_maintainer.h"
#include "assoc_maintainer.h"
#include "slabs.h"
#include "global.h"
#include "util.h"


static SlabsManager& sm_instance = SlabsManager::GetInstance();
static AssocMaintainer& am_instance = AssocMaintainer::GetInstance();
static int32_t power = 13; // for now

static struct ev_loop *ItemMaintainer::time_loop_ = ev_default_loop(0);

ItemMaintainer::ItemMaintainer() {
    heads_ = calloc(LARGEST_ID, sizeof(Item*));
    tails_ = calloc(LARGEST_ID, sizeof(Item*));

    item_sizes_ = calloc(LARGEST_ID, sizeof(unsigned int));
    item_stats_ = calloc(LARGEST_ID, sizeof(ItemStats));

    cache_locks_ = calloc(POWER_LARGEST, sizeof(pthread_mutex_t));

    int32_t lock_cnt = hashsize(power);
    item_locks_ = calloc(lock_cnt, sizeof(pthread_mutex_t));

    for (int32_t i = 0; i < LARGEST_ID; ++i) {
        memset(&item_sizes_[i], 0, sizeof(unsigned int));
        memset(&item_stats_[i], 0, sizeof(ItemStats));
    }

    for (i = 0; i < POWER_LARGEST; ++i) {
        pthread_mutex_init(&cache_locks_[i], NULL);
    }

    for (i = 0; i < lock_cnt; ++i) {
        pthread_mutex_init(&item_locks_[i], NULL);
    }
}

ItemMaintainer::~ItemMaintainer() {
    if (NULL != heads_) {
        free(heads_);
    }

    if (NULL != tails_) {
        free(tails_);
    }

    if (NULL != cache_locks_) {
        free(cache_locks_);
    }

    if (NULL != item_locks_) {
        free(item_locks_);
    }
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
    SlabsManager& sm_instance = SlabsManager::GetInstance();
    uint8_t nsuffix;
    char suffix[40];
    size_t ntotal = ItemMakeHeader(nkey + 1, flags, nbytes, suffix, &nsuffix);
    unsigned int id = sm_instance.SlabsClsid(ntotal);
    if (0 == id) {
        return NULL;
    }

    if (g_settings.use_cas) {
        ntotal += sizeof(uint64_t);
    }

    /* If no memory is available, attempt a direct LRU juggle/eviction */
    /*
     * This is a race in order to simplify lru_pull_tail; in cases where
     * locked items are on the tail, you want them to fall out and cause
     * occasional OOM's, rather than internally work around them.
     * This also gives one fewer code path for slab alloc/free
     */
    Item *it = NULL;
    unsigned int total_chunks = 0;
    int loop_times = 0
    for (; loop_times < 5; ++loop_times) {
        /* Try to reclaim memory first */
        if (!g_settings.lru_maintainer_thread) {
            ItemLRUPullTail(id, COLD_LRU, 0, false, cur_hv);
        }
        it = sm_instance.SlabsAllocator(ntotal, id, &total_chunks);
        if (g_settings.expirezero_does_not_evict) {
            CacheLock(id);
            total_chunks -= item_sizes_[CLEAR_LRU(id)];
            CacheUnlock(id);
        }
        if (NULL != it) {
            break
        }
        if (g_settings.lru_maintainer_thread) {
            ItemLRUPullTail(id, HOT_LRU, total_chunks, false, cur_hv);
            ItemLRUPullTail(id, WARM_LRU, total_chunks, false, cur_hv);
            ItemLRUPullTail(id, COLD_LRU, total_chunks, true, cur_hv);
        } else {
            ItemLRUPullTail(id, COLD_LRU, 0, true, cur_hv);
        }
    }

    if (loop_times > 0) {
        CacheLock(id);
        item_stats_[id].direct_reclaims += loop_times;
        CacheUnlock(id);
    }

    if (it == NULL) {
        CacheLock(id);
        itemstats[id].outofmemory++;
        CacheUnlock(id);
        return NULL;
    }

    assert(it->slabs_clsid == 0);

    /*
     * Refcount is seeded to 1 by slabs_alloc()
     */
    it->next = it->prev = it->h_next = 0;
    /*
     * Items are initially loaded into the HOT_LRU. This is '0' but I want at
     * least a note here. Compiler (hopefully?) optimizes this out.
     */
    if (g_settings.lru_maintainer_thread) {
        if (exptime == 0 && g_settings.expirezero_does_not_evict) {
            id |= NOEXP_LRU;
        } else {
            id |= HOT_LRU;
        }
    } else {
        /* There is only COLD in compat-mode */
        id |= COLD_LRU;
    }
    it->slabs_clsid = id;

    memcpy(ITEM_key(it), key, nkey);
    memcpy(ITEM_suffix(it), suffix, (size_t)nsuffix);
    it->it_flags = g_settings.use_cas ? ITEM_CAS : 0;
    it->nkey = nkey;
    it->nbytes = nbytes;
    it->exptime = exptime;
    it->nsuffix = nsuffix;
    return it;
}

void ItemMaintainer::FreeItem(Item *it) {
    assert((it->it_flags & ITEM_LINKED) == 0);
    assert(it != heads[it->slabs_clsid]);
    assert(it != tails[it->slabs_clsid]);
    assert(it->refcount == 0);

    /*
     * so slab size changer can tell later if item is already free or not
     */
    sm_instance.FreeSlabs(it, ITEM_ntotal(it), ITEM_clsid(it));
}

bool ItemMaintainer::ItemSizeOk(const size_t nkey, const int flags, const int nbytes) {
    char prefix[40];
    uint8_t nsuffix;

    size_t ntotal = ItemMakeHeader(nkey + 1, flags, nbytes, prefix, &nsuffix);
    if (g_settings.use_cas) {
        ntotal += sizeof(uint64_t);
    }

    return im_instance.SlabsClsid(ntotal) != 0;
}

int  ItemMaintainer::DoItemLink(Item *it, const uint32_t hv) {
    assert((it->it_flags & (ITEM_LINKED | ITEM_SLABBED)) == 0);
    it->it_flags |= ITEM_LINKED;
    it->time = current_time_;

    StatsLock();
    g_stats.curr_bytes += ITEM_ntotal(it);
    g_stats.curr_items += 1;
    g_stats.total_items += 1;
    StatsUnlock();

    /* Allocate a new CAS ID on link. */
    ITEM_set_cas(it, (g_settings.use_cas) ? GetCasId() : 0);
    am_instance.AssocInsert(it, hv);
    ItemLinkQ(it);
    RefcountIncrr(&it->refcount);
    return 1;
}

void ItemMaintainer::DoItemUnlink(Item *it, const uint32_t hv) {
    if ((it->it_flags & ITEM_LINKED) == 0) {
        return;
    }
    it->it_flags &= ~ITEM_LINKED;
    StatsLock();
    g_stats.curr_bytes -= ITEM_ntotal(it);
    g_stats.curr_items -= 1;
    StatsUnlock();
    am_instance.AssocDelete(ITEM_key(it), it->nkey, hv);
    ItemUnlinkQ(it);
    DoItemRemove(it);
}

void ItemMaintainer::DoItemUnlinkNolock(Item *it, const uint32_t hv) {
    /*
     * memcached code
     */
    DoItemUnlink(it, hv);
}

void ItemMaintainer::DoItemRemove(Item *it) {
    assert((it->it_flags & ITEM_SLABBED) == 0);
    assert(it->refcount > 0);

    if (RefcountDecr(&it->refcount) == 0) {
        FreeItem(it);
    }
}

void ItemMaintainer::DoItemUpdate(Item *it) {
    if (it->time >= current_tim_ - ITEM_UPDATE_INTERVAL) {
        return;
    }
    assert((it->it_flags & ITEM_SLABBED) == 0);

    if ((it->it_flags & ITEM_LINKED) == 0) {
        return
    }
    it->time = current_time_;
    if (!g_settings.lru_maintainer_thread) {
        ItemUnlinkQ(it);
        ItemLinkQ(it);
    }
}

void ItemMaintainer::DoItemUpdateNolock(Item *it) {
    if (it->time >= current_time_ - ITEM_UPDATE_INTERVAL) {
        return;
    }
    assert((it->it_flags & ITEM_SLABBED) == 0);

    if ((it->it_flags & ITEM_LINKED) != 0) {
        DoItemUnlinkQ(it);
        it->time = current_time_;
        DoItemLinkQ(it);
    }
}

int  ItemMaintainer::DoItemReplace(Item *it, Item *new_it, const uint32_t hv) {
    assert((it->it_flags & ITEM_SLABBED) == 0);
    DoItemUnlink(it, hv);
    return DoItemLink(new_it, hv);

}

enum StoreItemType ItemMaintainer::DoStoreItem(const uint32_t hv, Item* it, int32_t op) {
}

Item *ItemMaintainer::DoItemGet(const char *key, const size_t nkey, const uint32_t hv) {
}

Item *ItemMaintainer::DoItemTouch(const char *key, const size_t nkey, uint32_t exptime, const uint32_t hv) {
}

void ItemMaintainer::ItemLinkQ(Item* it) {
    CacheLock(it->slabs_clsid);
    DoItemLinkQ(it);
    CacheUnlock(it->slabs_clsid):
}

void ItemMaintainer::ItemUnlinkQ(Item* it) {
    CacheLock(it->slabs_clsid);
    DoItemUnlinkQ(it);
    CacheUnlock(it->slabs_clsid):
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
    CacheLock(index);
    item* it = heads_[index];
    CacheUnlock(index);
    return it;
}

Item *ItemMaintainer::GetItemTailByIndex(int32_t index) {
    CacheLock(index);
    item* it = tails_[index];
    CacheUnlock(index);
    return it;
}

void ItemMaintainer::DoItemLinkQ(Item* it, bool is_crawler) {
    Item **head, **tail;
    assert(it->it_flags == 1);
    assert(it->nbytes == 0);
    int32_t lock_id = it->slabs_clsid;

    head = &heads_[lock_id];
    tail = &tails_[lock_id];

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
    if (!is_crawler) {
        ++item_sizes_[lock_id];
    }
}

void ItemMaintainer::DoItemUnlinkQ(Item* it, bool is_crawler) {
    Item **head, **tail;
    int32_t lock_id = it->slabs_clsid;
    head = &heads_[lock_id];
    tail = &tails_[lock_id];

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
    if (!is_crawler) {
        --item_sizes_[lock_id];
    }
}

void ItemMaintainer::CacheLock(int32_t lock_id) {
    pthread_mutex_lock(&cache_locks_[lock_id]);
}

void ItemMaintainer::CacheUnlock(int32_t lock_id) {
    pthread_mutex_unlock(&cache_locks_[lock_id]);
}

bool ItemMaintainer::ItemEvaluate(Item *eval_item, uint32_t hv, int32_t is_index) {
    ++item_stats_[is_index].crawler_items_checked;
    if ((eval_item->exptime != 0 &&
         eval_item->exptime < current_time_)
        || IsFlushed(eval_item)) {
        item_stats_[is_index].crawler_reclaimed++;
        if (g_settings.verbose > 1) {
            char *key = ITEM_key(eval_item);
            fprintf(stderr, "LRU crawler found an expired item (flags: %d, slab: %d): ",
                eval_item->it_flags, eval_item->slabs_clsid);
            for (int32_t ii = 0; ii < eval_item->nkey; ++ii) {
                fprintf(stderr, "%c", key[ii]);
            }
            fprintf(stderr, "\n");
        }
        if ((eval_item->it_flags & ITEM_FETCHED) == 0) {
            ++item_stats_[i].expired_unfetched;
        }
        DoItemUnlinkNolock(eval_item, hv);
        DoItemRemove(eval_item);
        assert(eval_item->slabs_clsid == 0);
        /*
         * item maintainer execute evaluate.
         */
        return true;
    }
    return false;
}

void ItemMaintainer::Lock(uint32_t hv) {
    pthread_mutex_lock(&item_locks_[hv & hashmask(power)]);
}

void *ItemMaintainer::TryLock(uint32_t hv) {
    pthread_mutex_t *lock = &item_locks_[hv & hashmask(power)];
    if (pthread_mutex_trylock(lock) == 0) {
        return lock;
    }
    return NULL;
}

void ItemMaintainer::TryLockUnlock(void *arg) {
    pthread_mutex_unlock((pthread_mutex_t *) lock);
}

void ItemMaintainer::Unlock(uint32_t hv) {
    pthread_mutex_unlock(&item_locks_[hv & hashmask(item_lock_hashpower)]);
}

Item *ItemMaintainer::ItemAlloc(char *key, size_t nkey, int flags, rel_time_t exptime, int nbytes) {
    /*
     * do_item_alloc handles its own locks
     */
    return DoItemAlloc(key, nkey, flags, exptime, nbytes, 0);
}

Item *ItemMaintainer::ItemGet(const char *key, const size_t nkey) {
}

Item *ItemMaintainer::ItemTouch(const char *key, const size_t nkey, uint32_t exptime) {
}

int32_t ItemMaintainer::ItemLink(Item *it) {
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

bool ItemMaintainer::IsFlushed(Item* it) {
    rel_time_t oldest_live = g_settings.oldest_live;
    uint64_t cas = ITEM_get_cas(it);
    uint64_t oldest_cas = g_settings.oldest_cas;
    if (0 == oldest_live || oldest_live > current_time_) {
        return false;
    }
    if ((it->time <= oldest_live) ||
        (oldest_cas != 0 && cas != 0 && cas < oldest_cas)) {
        return true;
    }
    return false;
}

size_t ItemMaintainer::ItemMakeHeader(const uint8_t nkey,
                                      const int flags,
                                      const int nbytes,
                                      char *suffix,
                                      uint8_t *nsuffix) {
    /*
     * suffix is defined at 40 chars elsewhere.
     */
    *nsuffix = (uint8_t) snprintf(suffix, 40, " %d %d\r\n", flags, nbytes - 2);
    return sizeof(Item) + nkey + *nsuffix + nbytes;
}

int32_t ItemMaintainer::ItemLRUPullTail(
        const int orig_id,
        const LRUStatus cur_lru,
        const unsigned int total_chunks,
        const bool do_evict,
        const uint32_t cur_hv) {
    if (0 == orig_id) {
        return 0;
    }

    Item *it = NULL;
    LRUStatus lru_status = HOT_LRU;
    int removed = 0;
    int id = orig_id | cur_lru;

    CacheLock(id)
    Item* search = tails_[id];
    ItemStats& cur_itemstats = item_stats_[id];
    /*
     * We walk up *only* for locked items, and if bottom is expired.
     */
    for (int tries = 5, Item* next_it = NULL;
        tries > 0 && search != NULL;
        --tries, search = next_it) {
        void *hold_lock = NULL;
        /*
         * we might relink search mid-loop, so search->prev isn't reliable
         */
        next_it = search->prev;
        if (search->nbytes == 0 && search->nkey == 0 && search->it_flags == 1) {
            /*
             * We are a crawler, ignore it.
             */
            ++tries;
            continue;
        }
        uint32_t hv = hash(ITEM_key(search), search->nkey);
        /*
         * Attempt to hash item lock the "search" item. If locked, no
         * other callers can incr the refcount. Also skip ourselves.
         */
        if (hv == cur_hv || (hold_lock = TryLock(hv)) == NULL) {
            continue;
        }
        /*
         * Now see if the item is refcount locked
         */
        if (RefcountIncr(&search->refcount) != 2) {
            /*
             * Note pathological case with ref'ed items in tail.
             * Can still unlink the item, but it won't be reusable yet
             */
            cur_itemstats.lrutail_reflocked++;
            /*
             * In case of refcount leaks, enable for quick workaround.
             * WARNING: This can cause terrible corruption
             */
            if (g_settings.tail_repair_time &&
                search->time + g_settings.tail_repair_time < current_time_) {
                cur_itemstats.tailrepairs++;
                search->refcount = 1;
                /*
                 * This will call item_remove -> item_free since refcnt is 1
                 */
                DoItemUnlinkNolock(search, hv);
                TryLockUnlock(hold_lock);
                continue;
            }
        }

        /*
         * Expired or flushed
         */
        if ((search->exptime != 0 && search->exptime < current_time_) || IsFlushed(search)) {
            cur_itemstats.reclaimed++;
            if ((search->it_flags & ITEM_FETCHED) == 0) {
                cur_itemstats.expired_unfetched++;
            }
            DoItemUnlinkNolock(search, hv);
            DoItemRemove(search);
            TryLockUnlock(hold_lock);
            ++removed;

            /*
             * If all we're finding are expired, can keep going
             */
            continue;
        }

        /*
         * If we're HOT_LRU or WARM_LRU and over size limit, send to COLD_LRU.
         * If we're COLD_LRU, send to WARM_LRU unless we need to evict
         */
        switch (cur_lru) {
            // case HOT_LRU:
            case WARM_LRU:
                uint64_t limit = total_chunks * g_settings.warm_lru_pct / 100;
                if (item_sizes_[id] > limit) {
                    cur_itemstats.moves_to_cold++;
                    lru_status = COLD_LRU;
                    DoItemUnlinkQ(search);
                    it = search;
                    ++removed;
                    break;
                } else if ((search->it_flags & ITEM_ACTIVE) != 0) {
                    /*
                     * Only allow ACTIVE relinking if we're not too large.
                     */
                    cur_itemstats.moves_within_lru++;
                    search->it_flags &= ~ITEM_ACTIVE;
                    DoItemUpdateNolock(search);
                    DoItemRemove(search);
                    TryLockUnlock(hold_lock);
                } else {
                    /*
                     * Don't want to move to COLD, not active, bail out
                     */
                    it = search;
                }
                break;
            case COLD_LRU:
                it = search; /* No matter what, we're stopping */
                if (do_evict) {
                    if (g_settings.evict_to_free == 0) {
                        break;
                    }
                    cur_itemstats.evicted++;
                    cur_itemstats.evicted_time = current_time_ - search->time;
                    if (search->exptime != 0) {
                        cur_itemstats.evicted_nonzero++;
                    }
                    if ((search->it_flags & ITEM_FETCHED) == 0) {
                        cur_itemstats.evicted_unfetched++;
                    }
                    DoItemUnlinkNolock(search, hv);
                } else if ((search->it_flags & ITEM_ACTIVE) != 0 && g_settings.lru_maintainer_thread) {
                    cur_itemstats.moves_to_warm++;
                    search->it_flags &= ~ITEM_ACTIVE;
                    lru_status = WARM_LRU;
                    DoItemUnlinkQ(search);
                }
                ++removed;
                break;
        }
        if (NULL != it) {
            break;
        }
    }
    CacheUnlock(id);

    if (NULL != it) {
        if (lru_status) {
            it->slabs_clsid = ITEM_clsid(it) | lru_status;
            ItemLinkQ(it);
        }
        DoItemRemove(it);
        TryLockUnlock(hold_lock);
    }

    return removed;
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
    ItemMaintainer::GetInstance().current_time_ = current_time;
}












/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
