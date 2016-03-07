/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file slabs_maintainer.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/03/06 23:50:33
 * @brief
 *
 **/

#include "slabs_maintainer.h"

#include "slabs_manager.h"


static SlabsManager& sm_instance = SlabsManager::GetInstance();

SlabsMaintainer::SlabsMaintainer() : slabs_maintainer_running_(true) {
}

SlabsMaintainer::~SlabsMaintainer() {
}

/*
 * Slab rebalancer thread.
 * Does not use spinlocks since it is not timing sensitive. Burn less CPU and
 * go to sleep if locks are contended
 */
void SlabsMaintainer::Run() {
    while (slabs_maintainer_running_) {
        if (g_settings.slab_automove == 1) {
            int src, dest;
            if (sm_instance.SlabAutomoveDecision(&src, &dest) == 1) {
                /*
                 * Blind to the return codes. It will retry on its own
                 */
                sm_instance.SlabsReassign(src, dest);
            }
            sleep(1);
        } else {
            /*
             * Don't wake as often if we're not enabled.
             * This is lazier than setting up a condition right now.
             */
            sleep(5);
        }
    }
}

void SlabsMaintainer::StopSlabsMaintainer() {
    slabs_maintainer_running_ = false;
}










/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
