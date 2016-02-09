/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file assoc.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/06 21:39:21
 * @brief
 *
 **/




#ifndef __ASSOC_H
#define __ASSOC_H

#include "item.h"

/* Initial power multiplier for the hash table */
#define HASHPOWER_DEFAULT 16

#define hashsize(n) ((ub4)1<<(n))
#define hashmask(n) (hashsize(n)-1)

extern unsigned int hashpower;
extern unsigned int item_lock_hashpower;


typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;   /* unsigned 1-byte quantities */

class AssocMaintainer : public Thread {
    public:
        AssocMaintainer();
        ~AssocMaintainer();

        void InitAssoc(const int hashpower_init);
        item* AssocFind(const char *key, const size_t nkey, const uint32_t hv);
        int32_t AssocInsert();
        void AssocDelete(const char *key, const size_t nkey, const uint32_t hv);

        /*
         * run the AssocMaintainer thread
         */
        virtual void Run();

    private:
        unsigned int hashpower;

        /*
         * Main hash table. This is where we look except during expansion.
         */
        item** primary_hashtable;

        /*
         * Previous hash table. During expansion, we look here for keys that haven't
         * been moved over to the primary yet.
         */
        item** old_hashtable;

        /*
         * Number of items in the hash table.
         */
        unsigned int hash_items;

        /*
         * Flag: Are we in the middle of expanding now?
         */
        bool expanding;
        bool started_expanding;

        unsigned int expand_bucket;

        pthread_cond_t maintenance_cond = PTHREAD_COND_INITIALIZER;
        pthread_mutex_t maintenance_lock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t hash_items_counter_lock = PTHREAD_MUTEX_INITIALIZER;

}


#endif // __ASSOC_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
