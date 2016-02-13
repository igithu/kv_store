/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file slabs.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/12 22:23:54
 * @brief
 *
 **/

#include "slabs.h"


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

void SlabsManager::Run() {
}

void SlabsManager::InitSlabs(const size_t limit, const double factor, const bool prealloc) {
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
