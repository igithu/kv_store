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

/*
 * actual cap is 255
 */
#define POWER_LARGEST  256

extern pthread_mutex_t lru_locks[POWER_LARGEST];

class LRUMaintainer : public Thread {
    public:
        ~LRUMaintainer();

        static LRUMaintainer& GetInstance();

        virtual void Run();

        int InitLRUMaintainer();
        void PauseLRU();
        void ResumeLRU();

    private:
        LRUMaintainer();

        DISALLOW_COPY_AND_ASSIGN(LRUMaintainer);


    private:
        pthread_mutex_t lru_maintainer_lock_;

};








#endif // __LRU_MAINTAINER_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
