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

LRUMaintainer::LRUMaintainer() {
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
}

void LRUMaintainer::StopLRUMaintainer() {
}

void LRUMaintainer::PauseLRU() {
}

void LRUMaintainer::ResumeLRU() {
}














/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
