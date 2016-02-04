/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file memcache_db.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/01/12 00:18:53
 * @brief
 *
 **/


#include "memcache_db.h"

#include <string.h>
#include <stdlib.h>

#include "assoc.h"
#include "slabs.h"

#define REALTIME_MAXDELTA 60*60*24*30

/*
 * given time value that's either unix time or delta from current unix time, return
 * unix time. Use the fact that delta can't exceed one month (and real time value can't
 * be that low).
 */
static rel_time_t realtime(const time_t exptime) {
    /* no. of seconds in 30 days - largest possible delta exptime */

    if (exptime == 0) return 0; /* 0 means never expire */

    if (exptime > REALTIME_MAXDELTA) {
        /* if item expiration is at/before the server started, give it an
           expiration time of 1 second after the server started.
           (because 0 means don't expire).  without this, we'd
           underflow and wrap around to some large value way in the
           future, effectively making items expiring in the past
           really expiring never */
        if (exptime <= process_started)
            return (rel_time_t)1;
        return (rel_time_t)(exptime - process_started);
    } else {
        return (rel_time_t)(exptime + current_time);
    }
}

MemcacheDB::MemcacheDB() : start_lru_crawler_(false), start_lru_maintainer_(false) {
}

MemcacheDB::~MemcacheDB() {
}

bool MemcacheDB::MemcacheDBInit() {
    InitSettings();

    InitStats();

    init_lru_maintainer();
    assoc_init(settings.hashpower_init);
    slabs_init(settings.maxbytes, settings.factor, /*preallocate*/true);

    if (init_lru_crawler() == -1) {
        exit(EXIT_FAILURE);
    }


    if (start_assoc_maintenance_thread() == -1) {
        exit(EXIT_FAILURE);
    }

    if (start_lru_crawler_ && start_item_crawler_thread() != 0) {
        fprintf(stderr, "Failed to enable LRU crawler thread\n");
        exit(EXIT_FAILURE);
    }

    if (start_lru_maintainer_ && start_lru_maintainer_thread() != 0) {
        fprintf(stderr, "Failed to enable LRU maintainer thread\n");
        return false;
    }

    if (start_slab_maintenance_thread() == -1) {
        exit(EXIT_FAILURE);
    }
    return true;
}

bool MemcacheDB::StopMemcacheDB() {
    stop_slab_maintenance_thread();

    if (stop_lru_maintainer_thread() != 0) {
        fprintf(stderr, "ERROR failed to stop lru thread\n");
    }

    if (stop_item_crawler_thread() != 0) {
        fprintf(stderr, "ERROR failed to stop lru crawler thread\n");
    }

    stop_assoc_maintenance_thread();
    return true;
}

bool MemcacheDB::OpenDB() {
    return true;
}

bool MemcacheDB::Put(WriteOptions& w_options, char* key, const char* value) {
    int32_t req_cas_id = w_options.cas_id;
    time_t exptime = w_options.exptime;

    int32_t nkey = strlen(key);
    int32_t vlen = strlen(value);
    vlen += 2;
    int32_t flags = 0;
    item* it = item_alloc(key, nkey, flags, realtime(exptime), vlen);
    if (NULL == it) {
        it = item_get(key, nkey);
        if (NULL != it) {
            item_unlink(it);
            item_remove(it);
        }
        return false;
    }
    ITEM_set_cas(it, req_cas_id);

    char* data = ITEM_data(it);
    memmove(data, value, vlen);

    protocol prot = w_options.prot;
    if (ascii_prot == prot) {
        store_item(it, NREAD_REPLACE);
        item_remove(it);
    } else if (binary_prot == prot) {
        /* We don't actually receive the trailing two characters in the bin
         * protocol, so we're going to just set them here */
        *(ITEM_data(it) + it->nbytes - 2) = '\r';
        *(ITEM_data(it) + it->nbytes - 1) = '\n';
        store_item(it, NREAD_REPLACE);
    }

    return true;
}

bool MemcacheDB::Get(ReadOptions& r_options, const char* key, std::string& value) {
    //int32_t req_cas_id = w_options.cas_id;
    bool need_cas = r_options.need_cas;
    int32_t nkey = strlen(key);

    item* it = item_get(key, nkey);
    if (NULL == it) {
        return false;
    }

    if (need_cas) {
        value.assign(ITEM_data(it), it->nbytes);
    } else {
        value.assign(ITEM_suffix(it), it->nsuffix + it->nbytes);
    }
    item_remove(it);
    item_update(it);
    return true;
}

bool MemcacheDB::Delete(WriteOptions& w_options, const char* key) {
    int32_t nkey = strlen(key);

    item* it = item_get(key, nkey);
    if (NULL == it) {
        return false;
    }
    item_unlink(it);
    item_remove(it);      /* release our reference */
    return true;
}

void MemcacheDB::InitSettings(void) {
    settings.use_cas = true;
    settings.access = 0700;
    settings.port = 11211;
    settings.udpport = 11211;
    /* By default this string should be NULL for getaddrinfo() */
    settings.inter = NULL;
    settings.maxbytes = 64 * 1024 * 1024; /* default is 64MB */
    settings.maxconns = 1024;         /* to limit connections-related memory to about 5MB */
    settings.verbose = 0;
    settings.oldest_live = 0;
    settings.oldest_cas = 0;          /* supplements accuracy of oldest_live */
    settings.evict_to_free = 1;       /* push old items out of cache when memory runs out */
    settings.socketpath = NULL;       /* by default, not using a unix socket */
    settings.factor = 1.25;
    settings.chunk_size = 48;         /* space for a modest key and value */
    settings.num_threads = 4;         /* N workers */
    settings.num_threads_per_udp = 0;
    settings.prefix_delimiter = ':';
    settings.detail_enabled = 0;
    settings.reqs_per_event = 20;
    settings.backlog = 1024;
    settings.binding_protocol = negotiating_prot;
    settings.item_size_max = 1024 * 1024; /* The famous 1MB upper limit. */
    settings.maxconns_fast = false;
    settings.lru_crawler = false;
    settings.lru_crawler_sleep = 100;
    settings.lru_crawler_tocrawl = 0;
    settings.lru_maintainer_thread = false;
    settings.hot_lru_pct = 32;
    settings.warm_lru_pct = 32;
    settings.expirezero_does_not_evict = false;
    settings.hashpower_init = 0;
    settings.slab_reassign = false;
    settings.slab_automove = 0;
    settings.shutdown_command = false;
    settings.tail_repair_time = TAIL_REPAIR_TIME_DEFAULT;
    settings.flush_enabled = true;
    settings.crawls_persleep = 1000;
}

void MemcacheDB::InitStats() {
    stats.curr_items = stats.total_items = stats.curr_conns = stats.total_conns = stats.conn_structs = 0;
    stats.get_cmds = stats.set_cmds = stats.get_hits = stats.get_misses = stats.evictions = stats.reclaimed = 0;
    stats.touch_cmds = stats.touch_misses = stats.touch_hits = stats.rejected_conns = 0;
    stats.malloc_fails = 0;
    stats.curr_bytes = stats.listen_disabled_num = 0;
    stats.hash_power_level = stats.hash_bytes = stats.hash_is_expanding = 0;
    stats.expired_unfetched = stats.evicted_unfetched = 0;
    stats.slabs_moved = 0;
    stats.lru_maintainer_juggles = 0;
    stats.accepting_conns = true; /* assuming we start in this state. */
    stats.slab_reassign_running = false;
    stats.lru_crawler_running = false;
    stats.lru_crawler_starts = 0;

    /* make the time we started always be 2 seconds before we really
       did, so time(0) - time.started is never zero.  if so, things
       like 'settings.oldest_live' which act as booleans as well as
       values are now false in boolean context... */
    process_started = time(0) - ITEM_UPDATE_INTERVAL - 2;
    stats_prefix_init();
}


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
