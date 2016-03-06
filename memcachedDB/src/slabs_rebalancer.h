/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file slabs_rebalancer.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/03/06 22:06:55
 * @brief
 *
 **/




#ifndef __SLABS_REBALANCER_H
#define __SLABS_REBALANCER_H

#include <pthread.h>

#include "thread.h"

class SlabsRebalancer : public Thread {
    public:
        SlabsRebalancer();
        ~SlabsRebalancer();

        virtual void Run();

    private:
        pthread_cond_t slab_rebalance_cond_;

};



#endif // __SLABS_REBALANCER_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
