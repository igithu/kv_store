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

#include "item_maintainer.h"
#include "util.h"

#define MAX_LRU_MAINTAINER_SLEEP 1000000
#define MIN_LRU_MAINTAINER_SLEEP 1000

static ItemMaintainer& im_instance = ItemMaintainer::GetInstance();

LRUMaintainer::LRUMaintainer() :
    lru_maintainer_initialized_(false),
    lru_maintainer_running_(false),
    lru_clsid_checked_(false),
    lru_maintainer_lock_(PTHREAD_MUTEX_INITIALIZER) {
}

LRUMaintainer::~LRUMaintainer() {
}

LRUMaintainer& LRUMaintainer::GetInstance() {
    static LRUMaintainer lm_instance;
    return lm_instance;
}

void LRUMaintainer::Run() {
    int i;
    useconds_t to_sleep = MIN_LRU_MAINTAINER_SLEEP;
    rel_time_t last_crawler_check = 0;

    if (settings.verbose > 2) {
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
        if (!lru_clsid_checked_) {
            did_moves = LRUMaintainerJuggle(lru_clsid_checked_);
            lru_clsid_checked_ = false;
        } else {
            for (i = POWER_SMALLEST; i < MAX_NUMBER_OF_SLAB_CLASSES; i++) {
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
        rel_time_t ct = im_instance.GetCurrentTime()
        if (g_settings.lru_crawler && last_crawler_check != ct) {
            lru_maintainer_crawler_check();
            last_crawler_check = ct;
        }
    }
    pthread_mutex_unlock(&lru_maintainer_lock_);
    if (settings.verbose > 2) {
        fprintf(stderr, "LRU maintainer thread stopping\n");
    }

}

int LRUMaintainer::InitLRUMaintainer() {
    if (false == lru_maintainer_initialized_) {
        pthread_mutex_init(&lru_maintainer_lock_, NULL);
        lru_maintainer_initialized_ = true;
    }
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














/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
