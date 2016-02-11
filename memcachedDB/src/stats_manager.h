/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file stats_manager.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/10 22:38:33
 * @brief
 *
 **/




#ifndef __STATS_MANAGER_H
#define __STATS_MANAGER_H

/** Stats stored per slab (and per thread). */
struct slab_stats {
    uint64_t  set_cmds;
    uint64_t  get_hits;
    uint64_t  touch_hits;
    uint64_t  delete_hits;
    uint64_t  cas_hits;
    uint64_t  cas_badval;
    uint64_t  incr_hits;
    uint64_t  decr_hits;
};



/**
 * Global stats.
 */
struct stats {
    pthread_mutex_t mutex;
    unsigned int  curr_items;
    unsigned int  total_items;
    uint64_t      curr_bytes;
    unsigned int  curr_conns;
    unsigned int  total_conns;
    uint64_t      rejected_conns;
    uint64_t      malloc_fails;
    unsigned int  reserved_fds;
    unsigned int  conn_structs;
    uint64_t      get_cmds;
    uint64_t      set_cmds;
    uint64_t      touch_cmds;
    uint64_t      get_hits;
    uint64_t      get_misses;
    uint64_t      touch_hits;
    uint64_t      touch_misses;
    uint64_t      evictions;
    uint64_t      reclaimed;
    time_t        started;          /* when the process was started */
    bool          accepting_conns;  /* whether we are currently accepting */
    uint64_t      listen_disabled_num;
    unsigned int  hash_power_level; /* Better hope it's not over 9000 */
    uint64_t      hash_bytes;       /* size used for hash tables */
    bool          hash_is_expanding; /* If the hash table is being expanded */
    uint64_t      expired_unfetched; /* items reclaimed but never touched */
    uint64_t      evicted_unfetched; /* items evicted but never touched */
    bool          slab_reassign_running; /* slab reassign in progress */
    uint64_t      slabs_moved;       /* times slabs were moved around */
    uint64_t      lru_crawler_starts; /* Number of item crawlers kicked off */
    bool          lru_crawler_running; /* crawl in progress */
    uint64_t      lru_maintainer_juggles; /* number of LRU bg pokes */
};

extern stats stats;

class StatsManager {
    public:
        void InitPrefix();
        void ClearPrefix(void);
        void GetPrefixRecord(const char *key, const size_t nkey, const bool is_hit);
        void DeletePrefixRecord(const char *key, const size_t nkey);
        void SetPrefixRecord(const char *key, const size_t nkey);
        char *DumpPrefix(int *length);

    private:
};




#endif // __STATS_MANAGER_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
