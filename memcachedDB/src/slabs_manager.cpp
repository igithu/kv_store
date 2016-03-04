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
    unsigned int size = sizeof(item) + g_settings.chunk_size;
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

    memset(slabclass, 0, sizeof(SlabClass));

    while (++i < MAX_NUMBER_OF_SLAB_CLASSES - 1 &&
           size <= g_settings.item_size_max / factor) {
        /* Make sure items are always n-byte aligned */
        if (size % CHUNK_ALIGN_BYTES) {
            size += CHUNK_ALIGN_BYTES - (size % CHUNK_ALIGN_BYTES);
        }

        slabclass[i].size = size;
        slabclass[i].perslab = g_settings.item_size_max / slabclass[i].size;
        size *= factor;
        if (settings.verbose > 1) {
            fprintf(stderr, "slab class %3d: chunk size %9u perslab %7u\n",
                    i, slabclass[i].size, slabclass[i].perslab);
        }
    }

    power_largest = i;
    slabclass[power_largest].size = settings.item_size_max;
    slabclass[power_largest].perslab = 1;
    if (settings.verbose > 1) {
        fprintf(stderr, "slab class %3d: chunk size %9u perslab %7u\n",
                i, slabclass[i].size, slabclass[i].perslab);
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
}

void *SlabsManager::SlabsAllocator(const size_t size, unsigned int id, unsigned int *total_chunks) {
}

void SlabsManager::FreeSlabs(void *ptr, size_t size, unsigned int id) {
}

bool SlabsManager::GetSlabStats(const char *stat_type, int nkey, ADD_STAT add_stats, void *c) {
}

void SlabsManager::SlabsStats(ADD_STAT add_stats, void *c) {
}

unsigned int SlabsManager::SlabsAvailableChunks(unsigned int id, bool *mem_flag, unsigned int *total_chunks) {
}

void SlabsManager::PauseSlabsRebalancer() {
}

void SlabsManager::ResumeSlabsRebalancer() {
}

int SlabsManager::NewSlab(const unsigned int id) {
}

void *SlabsManager::MemoryAllocator(size_t size) {
}

void SlabsManager::FreeSingleSlabs(void *ptr, const size_t size, unsigned int id) {
}














/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
