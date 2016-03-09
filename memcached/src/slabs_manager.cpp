/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file slabs_manager.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/12 22:23:54
 * @brief
 *
 **/

#include "slabs_manager.h"

#include "item_manager.h"
#include "slabs_maintainer.h"
#include "slabs_rebalancer.h"
#include "util.h"

static ItemManager& im_instance = ItemManager::GetInstance();
static SlabsMaintainer& sm_instance = SlabsMaintainer::GetInstance();
static SlabsRebalancer& sr_instance = SlabsRebalancer::GetInstance();


SlabsManager::SlabsManager() :
    mem_limit_(0),
    mem_malloced_(0),
    mem_limit_reached_(false),
    power_largest_(0),
    slab_bulk_check_(1),
    mem_base_(NULL),
    mem_current_(NULL),
    mem_avail_(0),
    slabs_lock_(PTHREAD_MUTEX_INITIALIZER),
    slabs_rebalance_lock_(PTHREAD_MUTEX_INITIALIZER) {
}

SlabsManager::~SlabsManager() {
}

SlabsManager& SlabsManager::GetInstance() {
    static SlabsManager sm_instance;
    return sm_instance;
}

bool SlabsManager::Start() {
    g_slab_rebalance_signal = 0;
    g_slab_rebal.slab_start = NULL;
    char *env = getenv("MEMCACHED_SLAB_BULK_CHECK");
    if (env != NULL) {
        slab_bulk_check_ = atoi(env);
        if (slab_bulk_check_ == 0) {
            slab_bulk_check = 1;
        }
    }

    if (pthread_cond_init(&slab_rebalance_cond_, NULL) != 0) {
        fprintf(stderr, "Can't intiialize rebalance condition\n");
        return false;
    }
    pthread_mutex_init(&slabs_rebalance_lock_, NULL);
    if (!sm_instance.Start()) {
        fprintf(stderr, "Can't create slab maint thread.\n");
        return false;
    }
    if (!sr_instance.Start()) {
        fprintf(stderr, "Can't create rebal thread.\n");
        return false;
    }
    return true;
}

void SlabsManager::Stop() {
    pthread_mutex_lock(&slabs_rebalance_lock_);
    sm_instance.StopSlabsMaintainer();
    sr_instance.StopSlabsRebalancer();
    pthread_mutex_unlock(&slabs_rebalance_lock_);
    pthread_cond_signal(&sm_instance.slab_rebalance_cond_);

    sm_instance.Wait();
    sr_instance.Wait();
}

void SlabsManager::InitSlabs(const size_t limit, const double factor, const bool prealloc) {
    int i = POWER_SMALLEST - 1;
    unsigned int size = sizeof(Item) + g_settings.chunk_size;
    mem_limit_ = limit;

    if (prealloc) {
        /* Allocate everything in a big chunk with malloc */
        mem_base_ = malloc(mem_limit_);
        if (NULL != mem_base) {
            mem_current_ = mem_base_;
            mem_avail_ = mem_limit_;
        } else {
            fprintf(stderr, "Warning: Failed to allocate requested memory in"
                    " one large chunk.\nWill allocate in smaller chunks\n");
        }
    }

    memset(slabclass_, 0, sizeof(SlabClass));

    while (++i < MAX_NUMBER_OF_SLAB_CLASSES - 1 &&
           size <= g_settings.item_size_max / factor) {
        /* Make sure items are always n-byte aligned */
        if (size % CHUNK_ALIGN_BYTES) {
            size += CHUNK_ALIGN_BYTES - (size % CHUNK_ALIGN_BYTES);
        }

        slabclass_[i].size = size;
        slabclass_[i].perslab = g_settings.item_size_max / slabclass_[i].size;
        size *= factor;
        if (g_settings.verbose > 1) {
            fprintf(stderr, "slab class %3d: chunk size %9u perslab %7u\n",
                    i, slabclass_[i].size, slabclass_[i].perslab);
        }
    }

    power_largest = i;
    slabclass_[power_largest].size = g_settings.item_size_max;
    slabclass_[power_largest].perslab = 1;
    if (g_settings.verbose > 1) {
        fprintf(stderr, "slab class %3d: chunk size %9u perslab %7u\n",
                i, slabclass_[i].size, slabclass_[i].perslab);
    }

    /* for the test suite:  faking of how much we've already malloc'd */
    {
        char *t_initial_malloc = getenv("T_MEMD_INITIAL_MALLOC");
        if (t_initial_malloc) {
            mem_malloced = (size_t)atol(t_initial_malloc);
        }

    }

    if (prealloc) {
        slabs_preallocate(power_largest);
    }
}

unsigned int SlabsManager::SlabsClsid(const size_t size) {
    if (0 == size) {
        return 0;
    }
    int res = POWER_SMALLEST;
    while (size > slabclass_[res].sizei) {
        if (res++ == power_largest)     /* won't fit in the biggest slab */
            return 0;
    }
    return res;
}

void *SlabsManager::SlabsAllocator(const size_t size, unsigned int id, unsigned int *total_chunks) {
    pthread_mutex_lock(&slabs_lock_);
    void *ret = DoSlabsAllocc(size, id, total_chunks);
    pthread_mutex_unlock(&slabs_lock_);
    return ret;
}

void SlabsManager::FreeSlabs(void *ptr, size_t size, unsigned int id) {
    pthread_mutex_lock(&slabs_lock_);
    DoSlabsFree(ptr, size, id);
    pthread_mutex_unlock(&slabs_lock_);
}

bool SlabsManager::GetSlabStats(const char *stat_type, int nkey, ADD_STAT add_stats, void *c) {
}

void SlabsManager::SlabsStats(ADD_STAT add_stats, void *c) {
    pthread_mutex_lock(&slabs_lock_);
    DoSlabsStats(add_stats, c);
    pthread_mutex_unlock(&slabs_lock_);
}

unsigned int SlabsManager::SlabsAvailableChunks(unsigned int id, bool *mem_flag, unsigned int *total_chunks) {
    pthread_mutex_lock(&slabs_lock_);
    SlabClass* p = &slabclass_[id];
    unsigned int ret = p->sl_curr;
    if (mem_flag != NULL) {
        *mem_flag = mem_limit_reached_;
    }
    if (total_chunks != NULL) {
        *total_chunks = p->slabs * p->perslab;
    }
    pthread_mutex_unlock(&slabs_lock_);
    return ret;

}

enum ReassignResultType SlabsManager::SlabsReassign(int src, int dst) {
    if (pthread_mutex_trylock(&slabs_rebalance_lock_) != 0) {
        return REASSIGN_RUNNING;
    }
    enum ReassignResultType = DoSlabsReassign(src, dst);
    pthread_mutex_unlock(&slabs_rebalance_lock_);
    return ret;
}

void SlabsManager::PauseSlabsRebalancer() {
    pthread_mutex_lock(&slabs_rebalance_lock_);
}

void SlabsManager::ResumeSlabsRebalancer() {
    pthread_mutex_unlock(&slabs_rebalance_lock_);
}

int SlabsManager::NewSlab(const unsigned int id) {
}

void *SlabsManager::MemoryAllocator(size_t size) {
    void *ret;
    if (mem_base == NULL) {
        /*
         * We are not using a preallocated large memory chunk
         */
        ret = malloc(size);
    } else {
        ret = mem_current;

        if (size > mem_avail_) {
            return NULL;
        }

        /*
         * mem_current pointer _must_ be aligned!!!
         */
        if (size % CHUNK_ALIGN_BYTES) {
            size += CHUNK_ALIGN_BYTES - (size % CHUNK_ALIGN_BYTES);
        }

        mem_current_ = ((char*)mem_current_) + size;
        if (size < mem_avail) {
            mem_avail -= size;
        } else {
            mem_avail = 0;
        }
    }

    return ret;

}

void SlabsManager::FreeSlabs(void *ptr, const size_t size, unsigned int id) {
    pthread_mutex_lock(&slabs_lock_);
    DoSlabsFree(ptr, size, id);
    pthread_mutex_unlock(&slabs_lock_);
}

int SlabsManager::DoSlabsNewSlab(const unsigned int id) {
    SlabClass *p = &slabclass_[id];
    int len = g_settings.slab_reassign ? g_settings.item_size_max
        : p->size * p->perslab;

    if ((mem_limit_ && mem_malloced_ + len > mem_limit_ && p->slabs > 0)) {
        mem_limit_reached_ = true;
        return 0;
    }

    char *ptr;
    if ((GrowSlabList(id) == 0) ||
        ((ptr = MemoryAllocator((size_t)len)) == 0)) {
        return 0;
    }
    memset(ptr, 0, (size_t)len);
    FreeSlabPage(ptr, id);

    p->slab_list[p->slabs++] = ptr;
    mem_malloced_ += len;
    return 1;

}

void *SlabsManager::DoSlabsAlloc(const size_t size, unsigned int id, unsigned int *total_chunks) {
    if (id < POWER_SMALLEST || id > power_largest) {
        return NULL;
    }
    void *ret = NULL;

    SlabClass* p = &slabclass_[id];
    assert(p->sl_curr == 0 || ((Item *)p->slots)->slabs_clsid == 0);

    *total_chunks = p->slabs * p->perslab;
    /* fail unless we have space at the end of a recently allocated page,
       we have something on our freelist, or we could allocate a new page */
    if (! (p->sl_curr != 0 || do_slabs_newslab(id) != 0)) {
        /*
         * We don't have more memory available
         */
    } else if (p->sl_curr != 0) {
        /*
         * return off our freelist
         */
        Item* it = (Item *)p->slots;
        p->slots = it->next;
        if (it->next) {
            it->next->prev = 0;
        }
        /*
         * Kill flag and initialize refcount here for lock safety in slab
         * mover's freeness detection.
         */
        it->it_flags &= ~ITEM_SLABBED;
        it->refcount = 1;
        p->sl_curr--;
        ret = (void *)it;
    }

    if (ret) {
        p->requested += size;
    }

    return ret;

}

void SlabsManager::DoSlabsFree(void *ptr, const size_t size, unsigned int id) {
    assert(id >= POWER_SMALLEST && id <= power_largest);
    if (id < POWER_SMALLEST || id > power_largest) {
        return;
    }

    SlabClass* p = &slabclass_[id];

    Item* it = (Item*)ptr;
    it->it_flags |= ITEM_SLABBED;
    it->slabs_clsid = 0;
    it->prev = 0;
    it->next = p->slots;
    if (it->next) {
        it->next->prev = it;
    }
    p->slots = it;

    p->sl_curr++;
    p->requested -= size;
}

void SlabsManager::DoSlabsStats(ADD_STAT add_stats, void *c) {
}

enum ReassignResultType SlabsManager::DoSlabsReassign(int src, int dst) {
    if (g_slab_rebalance_signal != 0) {
        return REASSIGN_RUNNING;
    }
    if (src == dst) {
        return REASSIGN_SRC_DST_SAME;
    }

    /*
     * Special indicator to choose ourselves.
     */
    if (-1 == src) {
        src = SlabsReassignPickany(dst);
        /*
         * TODO: If we end up back at -1, return a new error type
         */
    }

    if (src < POWER_SMALLEST || src > power_largest ||
        dst < POWER_SMALLEST || dst > power_largest) {
        return REASSIGN_BADCLASS;
    }

    if (slabclass_[src].slabs < 2) {
        return REASSIGN_NOSPARE;
    }

    g_slab_rebal.s_clsid = src;
    g_slab_rebal.d_clsid = dst;

    g_slab_rebalance_signal = 1;
    // pthread_cond_signal(&slab_rebalance_cond);

    return REASSIGN_OK;
}


int SlabsManager::GrowSlabList(const unsigned int id) {
    SlabClass *p = &slabclass_[id];
    if (p->slabs == p->list_size) {
        size_t new_size =  (p->list_size != 0) ? p->list_size * 2 : 16;
        void *new_list = realloc(p->slab_list, new_size * sizeof(void *));
        if (new_list == 0) return 0;
        p->list_size = new_size;
        p->slab_list = new_list;
    }
    return 1;
}

void SlabsManager::FreeSlabPage(char *ptr, const unsigned int id) {
    SlabClass *p = &slabclass_[id];
    for (int x = 0; x < p->perslab; ++x) {
        DoSlabsFree(ptr, 0, id);
        ptr += p->size;
    }
}

int SlabsManager::SlabsReassignPickany(int dst) {
    static int cur = POWER_SMALLEST - 1;
    for (int tries = power_largest_ - POWER_SMALLEST + 1; tries > 0; --tries) {
        ++cur;
        if (cur > power_largest_) {
            cur = POWER_SMALLEST;
        }
        if (cur == dst) {
            continue;
        }
        if (slabclass_[cur].slabs > 1) {
            return cur;
        }
    }
    return -1;

}


int SlabsManager::SlabAutomoveDecision(int *src, int *dst) {
    static rel_time_t next_run = 0;
    rel_time_t current_time = im_instance.GetCurrentTime();
    /*
     * Run less frequently than the slabmove tester.
     */
    if (current_time >= next_run) {
        next_run = current_time + 10;
    } else {
        return 0;
    }

    uint64_t evicted_new[MAX_NUMBER_OF_SLAB_CLASSES];
    unsigned int total_pages[MAX_NUMBER_OF_SLAB_CLASSES];
    im_instance.ItemStatsEvictions(evicted_new);
    pthread_mutex_lock(&slabs_lock_);
    for (int i = POWER_SMALLEST; i < power_largest; ++i) {
        total_pages[i] = slabclass_[i].slabs;
    }
    pthread_mutex_unlock(&slabs_lock_);

    /*
     * Find a candidate source; something with zero evicts 3+ times
     */
    static uint64_t evicted_old[MAX_NUMBER_OF_SLAB_CLASSES];
    static unsigned int slab_zeroes[MAX_NUMBER_OF_SLAB_CLASSES];
    unsigned int highest_slab = 0;
    uint64_t evicted_diff = 0, evicted_max  = 0;
    int source = 0;
    for (int i = POWER_SMALLEST; i < power_largest; ++i) {
        evicted_diff = evicted_new[i] - evicted_old[i];
        if (evicted_diff == 0 && total_pages[i] > 2) {
            slab_zeroes[i]++;
            if (source == 0 && slab_zeroes[i] >= 3)
                source = i;
        } else {
            slab_zeroes[i] = 0;
            if (evicted_diff > evicted_max) {
                evicted_max = evicted_diff;
                highest_slab = i;
            }
        }
        evicted_old[i] = evicted_new[i];
    }

    /*
     * Pick a valid destination
     */
    static unsigned int slab_winner = 0, slab_wins   = 0;
    int dest = 0;
    if (slab_winner != 0 && slab_winner == highest_slab) {
        ++slab_wins;
        if (slab_wins >= 3) {
            dest = slab_winner;
        }
    } else {
        slab_wins = 1;
        slab_winner = highest_slab;
    }

    if (source && dest) {
        *src = source;
        *dst = dest;
        return 1;
    }
    return 0;
}

void SlabsManager::SlabsAdjustMemRequested(unsigned int id, size_t old, size_t ntotal) {
    pthread_mutex_lock(&slabs_lock_);
    if (id < POWER_SMALLEST || id > power_largest) {
        fprintf(stderr, "Internal error! Invalid slab class\n");
        abort();
    }
    SlabClass* p = &slabclass_[id];
    p->requested = p->requested - old + ntotal;
    pthread_mutex_unlock(&slabs_lock_);
}

/*
 * refcount == 0 is safe since nobody can incr while item_lock is held.
 * refcount != 0 is impossible since flags/etc can be modified in other
 * threads. instead, note we found a busy one and bail. logic in do_item_get
 * will prevent busy items from continuing to be busy
 * NOTE: This is checking it_flags outside of an item lock. I believe this
 * works since it_flags is 8 bits, and we're only ever comparing a single bit
 * regardless. ITEM_SLABBED bit will always be correct since we're holding the
 * lock which modifies that bit. ITEM_LINKED won't exist if we're between an
 * item having ITEM_SLABBED removed, and the key hasn't been added to the item
 * yet. The memory barrier from the slabs lock should order the key write and the
 * flags to the item?
 * If ITEM_LINKED did exist and was just removed, but we still see it, that's
 * still safe since it will have a valid key, which we then lock, and then
 * recheck everything.
 * This may not be safe on all platforms; If not, slabs_alloc() will need to
 * seed the item key while holding slabs_lock.
 */
int SlabsManager::SlabsRebalancerMove() {
    int was_busy = 0, refcount = 0;
    enum MoveStatus m_status = MOVE_PASS;

    pthread_mutex_lock(&slabs_lock_);
    SlabClass* s_cls = &slabclass[g_slab_rebal.s_clsid];

    for (int x = 0; x < slab_bulk_check; ++x) {
        uint32_t hv = 0;
        void *hold_lock = NULL;
        Item *it = g_slab_rebal.slab_pos;
        m_status = MOVE_PASS;
        if (it->slabs_clsid != 255) {
            /*
             * ITEM_SLABBED can only be added/removed under the slabs_lock
             */
            if (it->it_flags & ITEM_SLABBED) {
                /*
                 * remove from slab freelist
                 */
                if (s_cls->slots == it) {
                    s_cls->slots = it->next;
                }
                if (it->next) {
                    it->next->prev = it->prev;
                }
                if (it->prev) {
                    it->prev->next = it->next;
                }
                s_cls->sl_curr--;
                m_status = MOVE_FROM_SLAB;
            } else if ((it->it_flags & ITEM_LINKED) != 0) {
                /*
                 * If it doesn't have ITEM_SLABBED, the item could be in any
                 * state on its way to being freed or written to. If no
                 * ITEM_SLABBED, but it's had ITEM_LINKED, it must be active
                 * and have the key written to it already.
                 */
                hv = Hash(ITEM_key(it), it->nkey);
                if ((hold_lock = im_instance.TryLock(hv)) == NULL) {
                    m_status = MOVE_LOCKED;
                } else {
                    int refcount = RefcountIncr(&it->refcount);
                    if (refcount == 2) {
                        /* item is linked but not busy */
                        /* Double check ITEM_LINKED flag here, since we're
                         * past a memory barrier from the mutex. */
                        if ((it->it_flags & ITEM_LINKED) != 0) {
                            m_status = MOVE_FROM_LRU;
                        } else {
                            /* refcount == 1 + !ITEM_LINKED means the item is being
                             * uploaded to, or was just unlinked but hasn't been freed
                             * yet. Let it bleed off on its own and try again later */
                            m_status = MOVE_BUSY;
                        }
                    } else {
                        if (g_settings.verbose > 2) {
                            fprintf(stderr, "Slab reassign hit a busy item: refcount: %d (%d -> %d)\n",
                                it->refcount, slab_rebal.s_clsid, slab_rebal.d_clsid);
                        }
                        m_status = MOVE_BUSY;
                    }
                    /*
                     * Item lock must be held while modifying refcount
                     */
                    if (m_status == MOVE_BUSY) {
                        RefcountDecr(&it->refcount);
                        im_instance.TryLockUnlock(hold_lock);
                    }
                }
            }
        }

        switch (m_status) {
            case MOVE_FROM_LRU:
                /*
                 * Lock order is LRU locks -> slabs_lock. unlink uses LRU lock.
                 * We only need to hold the slabs_lock while initially looking
                 * at an item, and at this point we have an exclusive refcount
                 * (2) + the item is locked. Drop slabs lock, drop item to
                 * refcount 1 (just our own, then fall through and wipe it
                 */
                pthread_mutex_unlock(&slabs_lock_);
                im_instance.DoItemUnlink(it, hv);
                im_instance.TryLockUnlock(hold_lock);
                pthread_mutex_lock(&slabs_lock_);
            case MOVE_FROM_SLAB:
                it->refcount = 0;
                it->it_flags = 0;
                it->slabs_clsid = 255;
                break;
            case MOVE_BUSY:
            case MOVE_LOCKED:
                g_slab_rebal.busy_items++;
                ++was_busy;
                break;
            case MOVE_PASS:
                break;
        }

        g_slab_rebal.slab_pos = (char *)g_slab_rebal.slab_pos + s_cls->size;
        if (g_slab_rebal.slab_pos >= g_slab_rebal.slab_end)
            break;
    }

    if (g_slab_rebal.slab_pos >= g_slab_rebal.slab_end) {
        /* Some items were busy, start again from the top */
        if (g_slab_rebal.busy_items) {
            g_slab_rebal.slab_pos = g_slab_rebal.slab_start;
            g_slab_rebal.busy_items = 0;
        } else {
            g_slab_rebal.done++;
        }
    }
    pthread_mutex_unlock(&slabs_lock_);

    return was_busy;
}

int SlabsManager::SlabRebalanceStart() {
}

void SlabsManager::SlabsSlabsRebalanceFinish() {
}



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
