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
