/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file slabs_maintainer.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/03/06 22:50:18
 * @brief
 *
 **/




#ifndef __SLABS_MAINTAINER_H
#define __SLABS_MAINTAINER_H

#include "thread.h"

#include <atomic>

#include "disallow_copy_and_assign.h"


namespace mdb {

class SlabsMaintainer : public PUBLIC_UTIL::Thread {
    public:
        ~SlabsMaintainer();

        virtual void Run();
        static SlabsMaintainer& GetInstance();

        void StopSlabsMaintainer();

    private:
        SlabsMaintainer();

        DISALLOW_COPY_AND_ASSIGN(SlabsMaintainer);

    private:
        volatile std::atomic<bool> slabs_maintainer_running_;
};

}  // end of namespace mdb

#endif // __SLABS_MAINTAINER_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
