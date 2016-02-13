/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file assoc_maintainer.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/11 16:17:56
 * @brief
 *
 **/

#include "assoc_maintainer.h"

#include "item_lru_crawler.h"
#include "lru_maintainer.h"
#include "slabs.h"
#include "stats_manager.h"


AssocMaintainer::AssocMaintainer() :
    primary_hashtable_(NULL),
    old_hashtable_(NULL),
    hash_items_(0),
    expanding_(false),
    started_expanding_(false),
    expand_bucket_(0),
    assoc_running_(true),
    maintenance_cond_(PTHREAD_COND_INITIALIZER),
    maintenance_lock_(PTHREAD_MUTEX_INITIALIZER),
    hash_items_counter_lock_(PTHREAD_MUTEX_INITIALIZER) {
}

AssocMaintainer::~AssocMaintainer() {
}

void AssocMaintainer::InitAssoc(const int hashpower_init) {
    if (hashtable_init) {
        hashpower_ = hashtable_init;
    }
    primary_hashtable_ = calloc(hashsize(hashpower_), sizeof(void *));
    if (NULL != primary_hashtable_) {
        fprintf(stderr, "Failed to init hashtable.\n");
        exit(EXIT_FAILURE);
    }
    STATS_LOCK();
    stats.hash_power_level = hashpower_;
    stats.hash_bytes = hashsize(hashpower_) * sizeof(void *);
    STATS_UNLOCK();
}

Item* AssocMaintainer::AssocFind(const char *key, const size_t nkey, const uint32_t hv) {
    Item *it;
    unsigned int oldbucket;

    if (expanding_ &&
       (oldbucket = (hv & hashmask(hashpower_ - 1))) >= expand_bucket_) {
        it = old_hashtable_[oldbucket];
    } else {
        it = primary_hashtable_[hv & hashmask(hashpower_)];
    }

    Item *ret = NULL;
    int depth = 0;
    while (it) {
        if ((nkey == it->nkey) && (memcmp(key, ITEM_key(it), nkey) == 0)) {
            ret = it;
            break;
        }
        it = it->h_next;
        ++depth;
    }
    return ret;
}

int32_t AssocMaintainer::AssocInsert() {
    unsigned int oldbucket;

    if (expanding_ &&
        (oldbucket = (hv & hashmask(hashpower_ - 1))) >= expand_bucket_)
    {
        it->h_next = old_hashtable_[oldbucket];
        old_hashtable_[oldbucket] = it;
    } else {
        it->h_next = primary_hashtable_[hv & hashmask(hashpower_)];
        primary_hashtable_[hv & hashmask(hashpower_)] = it;
    }

    pthread_mutex_lock(&hash_items_counter_lock_);
    ++hash_items_;
    if (!started_expanding_ && !expanding && hash_items_ > (hashsize(hashpower_) * 3) / 2) {
        started_expanding_ = true;
        pthread_cond_signal(&maintenance_cond_);
    }
    pthread_mutex_unlock(&hash_items_counter_lock_);

    return 1;

}

void AssocMaintainer::AssocDelete(const char *key, const size_t nkey, const uint32_t hv) {
   Item **before = _hashitem_before(key, nkey, hv);

    if (*before) {
        Item *nxt;
        pthread_mutex_lock(&hash_items_counter_lock_);
        --hash_items_;
        pthread_mutex_unlock(&hash_items_counter_lock_);
        nxt = (*before)->h_next;
        /*
         * probably pointless, but whatever.
         */
        (*before)->h_next = 0;
        *before = nxt;
        return;
    }
    /*
     * Note:  we never actually get here.  the callers don't delete things
       they can't find.
     */
    assert(*before != 0);
}

void AssocMaintainer::Run() {
    pthread_mutex_lock(&maintenance_lock_);
    while(assoc_running_) {
        /*
         * There is only one expansion thread, so no need to global lock.
         */
        for (int32_t i = 0; i < hash_bulk_move && expanding_; ++i) {
            Item *it, *next;
            int bucket;
            void *item_lock = NULL;

            /*
             * bucket = hv & hashmask(hashpower) =>the bucket of hash table
             * is the lowest N bits of the hv, and the bucket of item_locks is
             * also the lowest M bits of hv, and N is greater than M.
             * So we can process expanding with only one item_lock. cool!
             */
            if ((item_lock = TryLock(expand_bucket_))) {
                for (it = old_hashtable_[expand_bucket_]; NULL != it; it = next) {
                    next = it->h_next;
                    bucket = hash(ITEM_key(it), it->nkey) & hashmask(hashpower_);
                    it->h_next = primary_hashtable_[bucket];
                    primary_hashtable_[bucket] = it;
                }
                old_hashtable_[expand_bucket_] = NULL;

                ++expand_bucket_;
                if (expand_bucket_ == hashsize(hashpower_ - 1)) {
                    expanding_ = false;
                    free(old_hashtable_);
                    STATS_LOCK();
                    stats.hash_bytes -= hashsize(hashpower_ - 1) * sizeof(void *);
                    stats.hash_is_expanding = 0;
                    STATS_UNLOCK();
                    if (settings.verbose > 1)
                        fprintf(stderr, "Hash table expansion done\n");
                }
            } else {
                usleep(10*1000);
            }

            if (item_lock) {
                TryLockUnlock(item_lock);
                item_lock = NULL;
            }
        }

        if (!expanding_) {
            /*
             * We are done expanding.. just wait for next invocation
             */
            started_expanding_ = false;
            pthread_cond_wait(&maintenance_cond_, &maintenance_lock_);
            /*
             * assoc_expand() swaps out the hash table entirely, so we need
             * all threads to not hold any references related to the hash
             * table while this happens.
             * This is instead of a more complex, possibly slower algorithm to
             * allow dynamic hash table expansion without causing significant
             * wait times.
             */
            PauseThreads(PAUSE_ALL_THREADS);
            AssocExpand();
            PauseThreads(RESUME_ALL_THREADS);
        }
    }
}

void AssocMaintainer::AssocExpand() {
    old_hashtable_ = primary_hashtable_;

    primary_hashtable_ = calloc(hashsize(hashpower_ + 1), sizeof(void *));
    if (primary_hashtable_) {
        if (settings.verbose > 1)
            fprintf(stderr, "Hash table expansion starting\n");
        ++hashpower_;
        expanding_ = true;
        expand_bucket_ = 0;
        STATS_LOCK();
        stats.hash_power_level = hashpower_;
        stats.hash_bytes += hashsize(hashpower_) * sizeof(void *);
        stats.hash_is_expanding = 1;
        STATS_UNLOCK();
    } else {
        /*
         * Bad news, but we can keep running.
         */
        primary_hashtable = old_hashtable;
    }
}

void AssocMaintainer::PauseThreads(enum PauseThreadTypes type) {
    int i;
    switch (type) {
        case PAUSE_ALL_THREADS:
            SlabsManager::GetInstance().PauseSlabsRebalancer();
            LRUMaintainer::GetInstance().PauseLRU();
            ItemLRUCrawler::GetInstance().PauseCrawler();
            break;
        case RESUME_ALL_THREADS:
            SlabsManager::GetInstance().ResumeSlabsRebalancer();
            LRUMaintainer::GetInstance().ResumeLRU();
            ItemLRUCrawler::GetInstance().ResumeCrawler();
            break;
        default:
            fprintf(stderr, "Unknown lock type: %d\n", type);
            assert(1 == 0);
            break;
    }

    /* Only send a message if we have one. */
    /*
    if (buf[0] == 0) {
        return;
    }

    pthread_mutex_lock(&init_lock);
    init_count = 0;
    for (i = 0; i < settings.num_threads; i++) {
        if (write(threads[i].notify_send_fd, buf, 1) != 1) {
            perror("Failed writing to notify pipe");
        }
    }
    wait_for_thread_registration(settings.num_threads);
    pthread_mutex_unlock(&init_lock);
    */

}






/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
