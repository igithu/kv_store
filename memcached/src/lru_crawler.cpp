/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file lru_crawler.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/11 22:32:19
 * @brief
 *
 **/

#include "lru_crawler.h"

#include <stdlib.h>
#include <string.h>

#include "lru_maintainer.h"
#include "global.h"
#include "util.h"
#include "hash.h"

namespace mdb {

static ItemManager& im_instance = ItemManager::GetInstance();

LRUCrawler::LRUCrawler() :
    lru_crawler_initialized_(false),
    lru_crawler_running_(false),
    crawler_count_(0),
    lru_crawler_lock_(PTHREAD_MUTEX_INITIALIZER),
    lru_crawler_cond_(PTHREAD_COND_INITIALIZER),
    lru_crawler_stats_lock_(PTHREAD_MUTEX_INITIALIZER) {

        crawlers_ = (Crawler*)calloc(CRAWL_LARGEST_ID, sizeof(Crawler));
        crawler_stats_ = (CrawlerStats*)calloc(MAX_NUMBER_OF_SLAB_CLASSES, sizeof(CrawlerStats));
}

LRUCrawler::~LRUCrawler() {
    if (NULL != crawlers_) {
        free(crawlers_);
    }
    if (NULL != crawler_stats_) {
        free(crawler_stats_);
    }
}

LRUCrawler& LRUCrawler::GetInstance() {
    static LRUCrawler irc_instance;
    return irc_instance;
}

void LRUCrawler::Run() {
    int crawls_persleep = g_settings.crawls_persleep;

    if (g_settings.verbose > 2) {
        fprintf(stderr, "Starting LRU crawler background thread\n");
    }
    pthread_mutex_lock(&lru_crawler_lock_);
    while (lru_crawler_running_) {
        pthread_cond_wait(&lru_crawler_cond_, &lru_crawler_lock_);
        while (crawler_count_) {
            Item *search = NULL;
            void *hold_lock = NULL;
            for (int i = POWER_SMALLEST; i < LARGEST_ID; ++i) {
                if (crawlers_[i].it_flags != 1) {
                    continue;
                }
                im_instance.CacheLock(i);
                search = im_instance.CrawlerCrawlQ((Item *)&crawlers_[i]);
                if (NULL == search || (crawlers_[i].remaining && --crawlers_[i].remaining < 1)) {
                    if (g_settings.verbose > 2) {
                        fprintf(stderr, "Nothing left to crawl for %d\n", i);
                    }
                    crawlers_[i].it_flags = 0;
                    --crawler_count_;
                    im_instance.DoItemUnlinkQ((Item *)&crawlers_[i], true);
                    im_instance.CacheUnlock(i);
                    pthread_mutex_lock(&lru_crawler_stats_lock_);
                    crawler_stats_[CLEAR_LRU(i)].end_time = g_current_time;
                    crawler_stats_[CLEAR_LRU(i)].run_complete = true;
                    pthread_mutex_unlock(&lru_crawler_stats_lock_);
                    continue;
                }
                uint32_t hv = Hash(ITEM_key(search), search->nkey);
                /* Attempt to hash item lock the "search" item. If locked, no
                 * other callers can incr the refcount
                 */
                if ((hold_lock = im_instance.TryLock(hv)) == NULL) {
                    im_instance.CacheUnlock(i);
                    continue;
                }
                /* Now see if the item is refcount locked */
                if (RefcountIncr(&search->refcount) != 2) {
                    RefcountDecr(&search->refcount);
                    if (hold_lock) {
                        im_instance.TryLockUnlock(hold_lock);
                    }
                    im_instance.CacheUnlock(i);
                    continue;
                }

                /* Frees the item or decrements the refcount. */
                /* Interface for this could improve: do the free/decr here
                 * instead? */
                pthread_mutex_lock(&lru_crawler_stats_lock_);
                ItemCrawlerEvaluate(search, hv, i);
                pthread_mutex_unlock(&lru_crawler_stats_lock_);

                if (hold_lock) {
                    im_instance.TryLockUnlock(hold_lock);
                }
                im_instance.CacheUnlock(i);

                if (crawls_persleep <= 0 && g_settings.lru_crawler_sleep) {
                    usleep(g_settings.lru_crawler_sleep);
                    crawls_persleep = g_settings.crawls_persleep;
                }
            }
        }
        if (g_settings.verbose > 2) {
            fprintf(stderr, "LRU crawler thread sleeping\n");
        }
        StatsLock();
        g_stats.lru_crawler_running = false;
        StatsUnlock();
    }
    pthread_mutex_unlock(&lru_crawler_lock_);
    if (g_settings.verbose > 2) {
        fprintf(stderr, "LRU crawler thread stopping\n");
    }
}

bool LRUCrawler::InitLRUCrawler() {
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

bool LRUCrawler::StartLRUCrawler() {
    if (g_settings.lru_crawler) {
        return true;
    }
    pthread_mutex_lock(&lru_crawler_lock_);
    lru_crawler_running_ = true;
    g_settings.lru_crawler = true;
    if (!Start()) {
        fprintf(stderr, "Failed to stop LRU crawler thread\n");
        return false;
    }
    pthread_mutex_unlock(&lru_crawler_lock_);
}

void LRUCrawler::StopLRUCrawler() {
    pthread_mutex_lock(&lru_crawler_lock_);
    lru_crawler_running_ = false;
    pthread_cond_signal(&lru_crawler_cond_);
    pthread_mutex_unlock(&lru_crawler_lock_);

    Wait();
    g_settings.lru_crawler = false;
}

enum CrawlerResultType LRUCrawler::LRUCrawl(char *slabs) {
    if (pthread_mutex_trylock(&lru_crawler_lock_) != 0) {
        return CRAWLER_RUNNING;
    }

    uint8_t tocrawl[MAX_NUMBER_OF_SLAB_CLASSES];
    /*
     * FIXME: I added this while debugging. Don't think it's needed?
     */
    memset(tocrawl, 0, sizeof(uint8_t) * MAX_NUMBER_OF_SLAB_CLASSES);
    uint32_t sid = 0;
    if (strcmp(slabs, "all") == 0) {
        for (; sid < MAX_NUMBER_OF_SLAB_CLASSES; sid++) {
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
            starts += DoLRUCrawlerStart(sid, g_settings.lru_crawler_tocrawl);
        }
    }
    if (starts) {
        pthread_cond_signal(&lru_crawler_cond_);
        pthread_mutex_unlock(&lru_crawler_lock_);
        return CRAWLER_OK;
    }
    pthread_mutex_unlock(&lru_crawler_lock_);
    return CRAWLER_NOTSTARTED;
}

void LRUCrawler::PauseCrawler() {
    pthread_mutex_lock(&lru_crawler_lock_);
}

void LRUCrawler::ResumeCrawler() {
    pthread_mutex_unlock(&lru_crawler_lock_);
}

int32_t LRUCrawler::LRUCrawlerStart(uint32_t id, uint32_t remaining) {
    if (pthread_mutex_trylock(&lru_crawler_lock_) != 0) {
        return 0;
    }
    int32_t starts = DoLRUCrawlerStart(id, remaining);
    if (starts) {
        pthread_cond_signal(&lru_crawler_cond_);
    }
    pthread_mutex_unlock(&lru_crawler_lock_);
    return starts;
}

void LRUCrawler::LRUMaintainerCrawlerCheck() {
}
int LRUCrawler::DoLRUCrawlerStart(uint32_t id, uint32_t remaining) {
    int starts = 0;

    uint32_t tocrawl[3];
    tocrawl[0] = id | HOT_LRU;
    tocrawl[1] = id | WARM_LRU;
    tocrawl[2] = id | COLD_LRU;

    for (int i = 0; i < 3; ++i) {
        uint32_t sid = tocrawl[i];
        im_instance.CacheLock(sid);
        Item* item = im_instance.GetItemTailByIndex(sid);
        if (NULL != item) {
            if (g_settings.verbose > 2) {
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
            im_instance.DoItemLinkQ((Item *)&crawlers_[sid], true);
            ++crawler_count_;
            ++starts;
        }
        im_instance.CacheUnlock(sid);
    }
    if (starts) {
        StatsLock();
        g_stats.lru_crawler_running = true;
        g_stats.lru_crawler_starts++;
        StatsUnlock();
        pthread_mutex_lock(&lru_crawler_stats_lock_);
        memset(&crawler_stats_[id], 0, sizeof(CrawlerStats));
        crawler_stats_[id].start_time = g_current_time;
        pthread_mutex_unlock(&lru_crawler_stats_lock_);
    }
    return starts;
}

Item *LRUCrawler::CrawlQ(Item *it) {
}

void LRUCrawler::ItemCrawlerEvaluate(Item *search, uint32_t hv, int i) {
    int slab_id = CLEAR_LRU(i);
    CrawlerStats *s = &crawler_stats_[slab_id];
    if (im_instance.ItemEvaluate(search, hv, i)) {
        s->reclaimed++;
    } else {
        s->seen++;
        RefcountIncr(&search->refcount);
        rel_time_t interval_time = search->exptime - g_current_time;
        if (search->exptime == 0) {
            s->noexp++;
        } else if (interval_time > 3599) {
            s->ttl_hourplus++;
        } else {
            int bucket = interval_time / 60;
            s->histo[bucket]++;
        }
    }
}


}  // end of namespace mdb





/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
