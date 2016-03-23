/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file lru_maintainer.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/12 17:07:47
 * @brief
 *
 **/

#include "lru_maintainer.h"

#include "item_manager.h"
#include "slabs_manager.h"
#include "lru_crawler.h"
#include "global.h"
#include "util.h"


namespace mdb {

const int MAX_LRU_MAINTAINER_SLEEP = 1000000;
const int MIN_LRU_MAINTAINER_SLEEP = 1000;

static ItemManager& im_instance = ItemManager::GetInstance();
static LRUCrawler& lc_instance = LRUCrawler::GetInstance();
static SlabsManager& sm_instance = SlabsManager::GetInstance();

LRUMaintainer::LRUMaintainer() :
    lru_maintainer_initialized_(false),
    lru_maintainer_running_(false),
    lru_clsid_(0),
    lru_maintainer_lock_(PTHREAD_MUTEX_INITIALIZER) {
}

LRUMaintainer::~LRUMaintainer() {
}

LRUMaintainer& LRUMaintainer::GetInstance() {
    static LRUMaintainer lm_instance;
    return lm_instance;
}

void LRUMaintainer::Run() {
    useconds_t to_sleep = MIN_LRU_MAINTAINER_SLEEP;
    rel_time_t last_crawler_check = 0;

    if (g_settings.verbose > 2) {
        fprintf(stderr, "Starting LRU maintainer background thread\n");
    }
    pthread_mutex_lock(&lru_maintainer_lock_);
    while (lru_maintainer_running_) {
        pthread_mutex_unlock(&lru_maintainer_lock_);
        int did_moves = 0;
        usleep(to_sleep);
        pthread_mutex_lock(&lru_maintainer_lock_);

        StatsLock();
        g_stats.lru_maintainer_juggles++;
        StatsUnlock();
        /* We were asked to immediately wake up and poke a particular slab
         * class due to a low watermark being hit */
        if (!lru_clsid_) {
            did_moves = LRUMaintainerJuggle(lru_clsid_);
            lru_clsid_ = false;
        } else {
            for (int i = POWER_SMALLEST; i < MAX_NUMBER_OF_SLAB_CLASSES; i++) {
                did_moves += LRUMaintainerJuggle(i);
            }
        }
        if (did_moves == 0) {
            if (to_sleep < MAX_LRU_MAINTAINER_SLEEP)
                to_sleep += 1000;
        } else {
            to_sleep /= 2;
            if (to_sleep < MIN_LRU_MAINTAINER_SLEEP)
                to_sleep = MIN_LRU_MAINTAINER_SLEEP;
        }
        /* Once per second at most */
        rel_time_t ct = g_current_time;
        if (g_settings.lru_crawler && last_crawler_check != ct) {
            lc_instance.LRUMaintainerCrawlerCheck();
            last_crawler_check = ct;
        }
    }
    pthread_mutex_unlock(&lru_maintainer_lock_);
    if (g_settings.verbose > 2) {
        fprintf(stderr, "LRU maintainer thread stopping\n");
    }
}

int32_t LRUMaintainer::LRUMaintainerJuggle(const int32_t slabs_clsid) {
    unsigned int total_chunks = 0;
    bool mem_limit_reached = false;
    sm_instance.SlabsAvailableChunks(slabs_clsid, &mem_limit_reached, &total_chunks);
    if (g_settings.expirezero_does_not_evict) {
        total_chunks -= im_instance.NoExpLRUSize(slabs_clsid);
    }

    /*
     * Juggle HOT/WARM up to N times
     */
    int32_t did_moves = 0;
    for (int32_t i = 0; i < 1000; ++i) {
        int do_more = 0;
        if (im_instance.ItemLRUPullTail(slabs_clsid, HOT_LRU, total_chunks, false, 0) ||
            im_instance.ItemLRUPullTail(slabs_clsid, WARM_LRU, total_chunks, false, 0)) {
            ++do_more;
        }
        do_more += im_instance.ItemLRUPullTail(slabs_clsid, COLD_LRU, total_chunks, false, 0);
        if (0 == do_more) {
            break;
        }
        ++did_moves;
    }
    return did_moves;
}

int32_t LRUMaintainer::InitLRUMaintainer() {
    if (false == lru_maintainer_initialized_) {
        pthread_mutex_init(&lru_maintainer_lock_, NULL);
        lru_maintainer_initialized_ = true;
    }
    return 0;
}

bool LRUMaintainer::StartLRUMaintainer() {
    if (g_settings.lru_maintainer_thread) {
        return true;
    }
    pthread_mutex_lock(&lru_maintainer_lock_);
    lru_maintainer_running_ = true;
    g_settings.lru_maintainer_thread = true;
    if (!Start()) {
        fprintf(stderr, "Failed to stop LRU maintainer thread\n");
        return false;
    }
    pthread_mutex_unlock(&lru_maintainer_lock_);
    return true;
}

void LRUMaintainer::StopLRUMaintainer() {
    pthread_mutex_lock(&lru_maintainer_lock_);
    lru_maintainer_running_ = false;
    pthread_mutex_unlock(&lru_maintainer_lock_);
    Wait();
    g_settings.lru_maintainer_thread = false;
}

void LRUMaintainer::PauseLRU() {
    pthread_mutex_lock(&lru_maintainer_lock_);
}

void LRUMaintainer::ResumeLRU() {
    pthread_mutex_unlock(&lru_maintainer_lock_);
}


}  // end of namespace mdb











/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
