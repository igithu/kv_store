/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file item_lru_crawler.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/11 22:32:19
 * @brief
 *
 **/

#include "item_lru_crawler.h"

#include "lru_maintainer.h"

ItemLRUCrawler::ItemLRUCrawler() :
    lru_crawler_initialized_(false),
    crawler_count_(0),
    lru_crawler_lock_(PTHREAD_MUTEX_INITIALIZER),
    lru_crawler_cond_(PTHREAD_COND_INITIALIZER),
    lru_crawler_stats_lock_(PTHREAD_MUTEX_INITIALIZER) {
}

ItemLRUCrawler::~ItemLRUCrawler() {
}

ItemLRUCrawler& ItemLRUCrawler::GetInstance() {
    static ItemLRUCrawler irc_instance;
    return irc_instance;
}

void ItemLRUCrawler::Run() {
}

bool InitLRUCrawler::InitLRUCrawler() {
    if (lru_crawler_initialized_) {
        return true;
    }
    memset(&crawler_stats_, 0, sizeof(CrawlerStats) * MAX_NUMBER_OF_SLAB_CLASSES);
    if (pthread_cond_init(&lru_crawler_cond_, NULL) != 0) {
        fprintf(stderr, "Can't initialize lru crawler condition\n");
        return false;
    }
    pthread_mutex_init(&lru_crawler_lock_, NULL);
    lru_crawler_initialized_ = true;
    return true;
}

enum CrawlerResultType ItemLRUCrawler::LRUCrawl(char *slabs) {
    if (pthread_mutex_trylock(&lru_crawler_lock_) != 0) {
        return CRAWLER_RUNNING;
    }

    uint8_t tocrawl[MAX_NUMBER_OF_SLAB_CLASSES];
    /*
     * FIXME: I added this while debugging. Don't think it's needed?
     */
    memset(tocrawl, 0, sizeof(uint8_t) * MAX_NUMBER_OF_SLAB_CLASSES);
    if (strcmp(slabs, "all") == 0) {
        for (uint32_t sid = 0; sid < MAX_NUMBER_OF_SLAB_CLASSES; sid++) {
            tocrawl[sid] = 1;
        }
    } else {
        char *b = NULL;
        for (char *p = strtok_r(slabs, ",", &b);
             p != NULL;
             p = strtok_r(NULL, ",", &b)) {
            if (!safe_strtoul(p, &sid) || sid < POWER_SMALLEST || sid >= MAX_NUMBER_OF_SLAB_CLASSES-1) {
                pthread_mutex_unlock(&lru_crawler_lock_);
                return CRAWLER_BADCLASS;
            }
            tocrawl[sid] = 1;
        }
    }

    int starts = 0;
    for (uint32_t sid = POWER_SMALLEST; sid < MAX_NUMBER_OF_SLAB_CLASSES; ++sid) {
        if (tocrawl[sid]) {
            starts += DoLRUCrawlerStart(sid, settings.lru_crawler_tocrawl);
        }
    }
    if (starts) {
        pthread_cond_signal(&lru_crawler_cond);
        pthread_mutex_unlock(&lru_crawler_lock_);
        return CRAWLER_OK;
    }
    pthread_mutex_unlock(&lru_crawler_lock_);
    return CRAWLER_NOTSTARTED;
}

void ItemLRUCrawler::PauseCrawler() {
}

void ItemLRUCrawler::ResumeCrawler() {
}

int ItemLRUCrawler::DoLRUCrawlerStart(uint32_t id, uint32_t remaining) {
    int starts = 0;

    uint32_t tocrawl[3];
    tocrawl[0] = id | HOT_LRU;
    tocrawl[1] = id | WARM_LRU;
    tocrawl[2] = id | COLD_LRU;

    for (int i = 0; i < 3; ++i) {
        uint32_t sid = tocrawl[i];
        if (ItemMaintainer::GetInstance().GetItemSizeByIndex(sid) != NULL ) {
            if (settings.verbose > 2) {
                fprintf(stderr, "Kicking LRU crawler off for LRU %d\n", sid);
            }
            crawlers_[sid].nbytes = 0;
            crawlers_[sid].nkey = 0;
            crawlers_[sid].it_flags = 1; /* For a crawler, this means enabled. */
            crawlers_[sid].next = 0;
            crawlers_[sid].prev = 0;
            crawlers_[sid].time = 0;
            crawlers_[sid].remaining = remaining;
            crawlers_[sid].slabs_clsid = sid;
            CrawlerLinkQ((Item *)&crawlers_[sid]);
            ++crawler_count_;
            starts++;
        }
    }
    if (starts) {
        STATS_LOCK();
        stats.lru_crawler_running = true;
        stats.lru_crawler_starts++;
        STATS_UNLOCK();
        pthread_mutex_lock(&lru_crawler_stats_lock_);
        memset(&crawler_stats_[id], 0, sizeof(CrawlerStats));
        crawler_stats_[id].start_time = ItemMaintainer::GetInstance().GetCurrentTime();
        pthread_mutex_unlock(&lru_crawler_stats_lock_);
    }
    return starts;
}

void ItemLRUCrawler::CrawlerLinkQ(Item *it) {
}

void ItemLRUCrawler::CrawlerUnlinkQ(Item *it) {
}

Item *ItemLRUCrawler::CrawlQ(Item *it) {
}

void ItemLRUCrawler::ItemCrawlerEvaluate(Item *search, uint32_t hv, int i) {
}








/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
