/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file slabs_rebalancer.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/03/07 00:34:21
 * @brief
 *
 **/

#include "slabs_rebalancer.h"

#include "slabs_manager.h"

static SlabsManager& sm_instance = SlabsManager::GetInstance();

SlabsRebalancer::SlabsRebalancer() : slabs_rebalancer_running_(false) {
}

SlabsRebalancer::~SlabsRebalancer() {
}

/*
 * Slab mover thread.
 * Sits waiting for a condition to jump off and shovel some memory about
 */
void SlabsRebalancer::Run() {
    /*
     * So we first pass into cond_wait with the mutex held
     */
    pthread_mutex_lock(&slabs_rebalance_lock);
    int was_busy = 0;
    while (slabs_rebalancer_running_) {
        if (g_slab_rebalance_signal == 1) {
            if (sm_instance.SlabRebalanceStart() < 0) {
                /* Handle errors with more specifity as required. */
                g_slab_rebalance_signal = 0;
            }
            was_busy = 0;
        } else if (g_slab_rebalance_signal && g_slab_rebal.slab_start != NULL) {
            was_busy = sm_instance.SlabsRebalancerMove();
        }

        if (g_slab_rebal.done) {
            sm_instance.SlabsRebalanceFinish();
        } else if (was_busy) {
            /*
             * Stuck waiting for some items to unlock, so slow down a bit
             * to give them a chance to free up
             */
            usleep(50);
        }

        if (g_slab_rebalance_signal == 0) {
            /*
             * always hold this lock while we're running
             */
            pthread_cond_wait(&sm_instance.slab_rebalance_cond_, &sm_instance.slabs_rebalance_lock_);
        }
    }

}

void SlabsRebalancer::StopSlabsRebalancer() {
    slabs_rebalancer_running_ = false;
}






/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
