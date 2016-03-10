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

#include <atomic.h>

#include "thread.h"
#include "disallow_copy_and_assign.h"


namespace mdb {

class SlabsRebalancer : public PUBLIC_UTIL::Thread {
    public:
        ~SlabsRebalancer();

        static SlabsRebalancer& GetInstance();

        virtual void Run();
        void StopSlabsRebalancer();

    private:
        SlabsRebalancer();

        DISALLOW_COPY_AND_ASSIGN(SlabsRebalancer);

    private:
        volatile std::atomic<bool> slabs_rebalancer_running_;
};

}  // end of namespace mdb


#endif // __SLABS_REBALANCER_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
