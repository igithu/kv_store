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


SlabsManager::SlabsManager() :
    mem_limit_(0),
    mem_malloced_(0),
    mem_limit_reached_(false),
    power_largest_(0),
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
        if (settings.verbose > 1) {
            fprintf(stderr, "slab class %3d: chunk size %9u perslab %7u\n",
                    i, slabclass_[i].size, slabclass_[i].perslab);
        }
    }

    power_largest = i;
    slabclass_[power_largest].size = settings.item_size_max;
    slabclass_[power_largest].perslab = 1;
    if (settings.verbose > 1) {
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
    pthread_mutex_unlock(&slabs_lock);
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

}


int SlabsManager::GrowSlabList(const unsigned int id) {
    SlabClass_t *p = &slabclass_[id];
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














/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
