/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file item_lru_crawler.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/09 23:23:47
 * @brief
 *
 **/




#ifndef __ITEM_LRU_CRAWLER_H
#define __ITEM_LRU_CRAWLER_H

#include "item_maintainer.h"

#define CRAWL_LARGEST_ID 256

struct Crawler {
    struct _stritem *next;
    struct _stritem *prev;
    struct _stritem *h_next;    /* hash chain next */
    rel_time_t      time;       /* least recent access */
    rel_time_t      exptime;    /* expire time */
    int             nbytes;     /* size of data */
    unsigned short  refcount;
    uint8_t         nsuffix;    /* length of flags-and-length string */
    uint8_t         it_flags;   /* ITEM_* above */
    uint8_t         slabs_clsid;/* which slab class we're in */
    uint8_t         nkey;       /* key length, w/terminating null and padding */
    uint32_t        remaining;  /* Max keys to crawl per slab per invocation */
};

struct CrawlerStats {
    uint64_t histo[60];
    uint64_t ttl_hourplus;
    uint64_t noexp;
    uint64_t reclaimed;
    uint64_t seen;
    rel_time_t start_time;
    rel_time_t end_time;
    bool run_complete;
};

enum CrawlerResultType {
    CRAWLER_OK = 0, CRAWLER_RUNNING, CRAWLER_BADCLASS, CRAWLER_NOTSTARTED
};

class ItemLRUCrawler : public Thread {
    public:
        ~ItemLRUCrawler();

        static ItemLRUCrawler& GetInstance();

        virtual void Run();

        bool InitLRUCrawler();
        enum CrawlerResultType LRUCrawl(char *slabs);
        /*
         * If we hold this lock, crawler can't wake up or move
         */
        void PauseCrawler();
        void ResumeCrawler();

    private:
        ItemLRUCrawler();

        int DoLRUCrawlerStart(uint32_t id, uint32_t remaining);
        /*
         * item is the new tail
         */
        void CrawlerLinkQ(Item *it);

        void CrawlerUnlinkQ(Item *it);

        /*
         * This is too convoluted, but it's a difficult shuffle. Try to rewrite it
         * more clearly.
         */
        Item *CrawlQ(Item *it);

        /*
         * I pulled this out to make the main thread clearer, but it reaches into the
         *  main thread's values too much. Should rethink again.
         */
        void ItemCrawlerEvaluate(Item *search, uint32_t hv, int i);

        DISALLOW_COPY_AND_ASSIGN(ItemLRUCrawler);

    private:
        bool lru_crawler_initialized_;
        int32_t crawler_count_;

        Crawler crawlers_[CRAWL_LARGEST_ID];
        CrawlerStats crawler_stats_[MAX_NUMBER_OF_SLAB_CLASSES];
        pthread_mutex_t lru_crawler_lock_; // = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t  lru_crawler_cond_; // = PTHREAD_COND_INITIALIZER;
        pthread_mutex_t lru_crawler_stats_lock_; // = PTHREAD_MUTEX_INITIALIZER;

};



#endif // __ITEM_LRU_CRAWLER_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
