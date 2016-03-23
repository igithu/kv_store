/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file lru_maintainer.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/09 23:27:45
 * @brief
 *
 **/




#ifndef __LRU_MAINTAINER_H
#define __LRU_MAINTAINER_H

#include "thread.h"

#include <stdint.h>

#include "disallow_copy_and_assign.h"

namespace mdb {

class LRUMaintainer : public PUBLIC_UTIL::Thread {
    public:
        ~LRUMaintainer();

        static LRUMaintainer& GetInstance();

        virtual void Run();

        int32_t InitLRUMaintainer();
        bool StartLRUMaintainer();
        void StopLRUMaintainer();
        void PauseLRU();
        void ResumeLRU();

    private:
        LRUMaintainer();

        int32_t LRUMaintainerJuggle(const int32_t slabs_clsid);

        DISALLOW_COPY_AND_ASSIGN(LRUMaintainer);


    private:
        bool lru_maintainer_initialized_;
        volatile bool lru_maintainer_running_;
        bool lru_clsid_;

        pthread_mutex_t lru_maintainer_lock_;

};

}  // end of namespace mdb



#endif // __LRU_MAINTAINER_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
