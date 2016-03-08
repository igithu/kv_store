/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file assoc_maintainer.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/10 22:02:58
 * @brief
 *
 **/




#ifndef __ASSOC_MAINTAINER_H
#define __ASSOC_MAINTAINER_H


#include "item_manager.h"

#define hashsize(n) ((ub4)1<<(n))
#define hashmask(n) (hashsize(n)-1)

typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;   /* unsigned 1-byte quantities */

enum PauseThreadTypes {
    PAUSE_WORKER_THREADS = 0,
    PAUSE_ALL_THREADS,
    RESUME_ALL_THREADS,
    RESUME_WORKER_THREADS
};

class AssocMaintainer : public Thread {
    public:
        AssocMaintainer();
        ~AssocMaintainer();

        void InitAssoc(const int hashpower_init = 0);
        Item* AssocFind(const char *key, const size_t nkey, const uint32_t hv);
        int32_t AssocInsert(Item *it, const uint32_t hv);
        void AssocDelete(const char *key, const size_t nkey, const uint32_t hv);
        void AssocStartExpand();

        /*
         * run the AssocMaintainer thread
         */
        virtual void Run();
        void StopAssocMaintainer();

    private:
        /*
         * grows the hashtable to the next power of 2.
         */
        void AssocExpand();
        void PauseThreads(enum PauseThreadTypes type);

    private:
        unsigned int hashpower_;

        /*
         * Main hash table. This is where we look except during expansion.
         */
        Item** primary_hashtable_;

        /*
         * Previous hash table. During expansion, we look here for keys that haven't
         * been moved over to the primary yet.
         */
        Item** old_hashtable_;

        /*
         * Number of items in the hash table.
         */
        unsigned int hash_items_;

        /*
         * Flag: Are we in the middle of expanding now?
         */
        bool expanding_;
        bool started_expanding_;

        unsigned int expand_bucket_;
        int32_t hash_bulk_move_;

        volatile bool assoc_running_;

        pthread_cond_t maintenance_cond_; // = PTHREAD_COND_INITIALIZER;
        pthread_mutex_t maintenance_lock_; //  = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t hash_items_counter_lock_; //  = PTHREAD_MUTEX_INITIALIZER;

}






#endif // __ASSOC_MAINTAINER_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
