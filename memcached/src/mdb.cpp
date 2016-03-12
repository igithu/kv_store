/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file mdb.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/03/09 20:12:04
 * @brief
 *
 **/


#include "mdb.h"

namespace mdb {

using std::string;

MDB::MDB() :
    item_manager_(ItemManager::GetInstance()),
    slabs_manager_(SlabsManager::GetInstance()),
    assoc_maintainer_(AssocMaintainer::GetInstance()),
    lru_maintainer_(LRUMaintainer::GetInstance()),
    slabs_maintainer_(SlabsMaintainer::GetInstance()),
    lru_crawler_(LRUCrawler::GetInstance()),
    slabs_rebalancer_(SlabsRebalancer::GetInstance()) {
}

MDB::~MDB() {
}

bool MDB::InitDB() {
    /*
     * global  init
     */
    g_settings.use_cas = true;
    g_settings.access = 0700;
    g_settings.port = 11211;
    g_settings.udpport = 11211;
    /* By default this string should be NULL for getaddrinfo() */
    g_settings.inter = NULL;
    g_settings.maxbytes = 64 * 1024 * 1024; /* default is 64MB */
    g_settings.maxconns = 1024;         /* to limit connections-related memory to about 5MB */
    g_settings.verbose = 0;
    g_settings.oldest_live = 0;
    g_settings.oldest_cas = 0;          /* supplements accuracy of oldest_live */
    g_settings.evict_to_free = 1;       /* push old items out of cache when memory runs out */
    g_settings.socketpath = NULL;       /* by default, not using a unix socket */
    g_settings.factor = 1.25;
    g_settings.chunk_size = 48;         /* space for a modest key and value */
    g_settings.num_threads = 4;         /* N workers */
    g_settings.num_threads_per_udp = 0;
    g_settings.prefix_delimiter = ':';
    g_settings.detail_enabled = 0;
    g_settings.reqs_per_event = 20;
    g_settings.backlog = 1024;
    g_settings.binding_protocol = negotiating_prot;
    g_settings.item_size_max = 1024 * 1024; /* The famous 1MB upper limit. */
    g_settings.maxconns_fast = false;
    g_settings.lru_crawler = false;
    g_settings.lru_crawler_sleep = 100;
    g_settings.lru_crawler_tocrawl = 0;
    g_settings.lru_maintainer_thread = false;
    g_settings.hot_lru_pct = 32;
    g_settings.warm_lru_pct = 32;
    g_settings.expirezero_does_not_evict = false;
    g_settings.hashpower_init = 0;
    g_settings.slab_reassign = false;
    g_settings.slab_automove = 0;
    g_settings.shutdown_command = false;
    g_settings.tail_repair_time = TAIL_REPAIR_TIME_DEFAULT;
    g_settings.flush_enabled = true;
    g_settings.crawls_persleep = 1000;


    /*
     * global stats init
     */
    g_stats.curr_items = g_stats.total_items = g_stats.curr_conns = g_stats.total_conns = g_stats.conn_structs = 0;
    g_stats.get_cmds = g_stats.set_cmds = g_stats.get_hits = g_stats.get_misses = g_stats.evictions = g_stats.reclaimed = 0;
    g_stats.touch_cmds = g_stats.touch_misses = g_stats.touch_hits = g_stats.rejected_conns = 0;
    g_stats.malloc_fails = 0;
    g_stats.curr_bytes = g_stats.listen_disabled_num = 0;
    g_stats.hash_power_level = g_stats.hash_bytes = g_stats.hash_is_expanding = 0;
    g_stats.expired_unfetched = g_stats.evicted_unfetched = 0;
    g_stats.slabs_moved = 0;
    g_stats.lru_maintainer_juggles = 0;
    g_stats.accepting_conns = true; /* assuming we start in this state. */
    g_stats.slab_reassign_running = false;
    g_stats.lru_crawler_running = false;
    g_stats.lru_crawler_starts = 0;

    lru_crawler_.InitLRUCrawler();
    lru_maintainer_.InitLRUMaintainer();
    assoc_maintainer_.InitAssoc(g_settings.hashpower_init);
    slabs_manager_.InitSlabs(g_settings.maxbytes, g_settings.factor, true);

    return true;
}

bool MDB::StartMDB() {
    if (!assoc_maintainer_.Start()) {
        fprintf(stderr, "Start the assoc_maintainer thread failed!\n");
        return false;
    }
    if (g_settings.lru_crawler && !lru_crawler_.Start()) {
        fprintf(stderr, "Failed to enable LRU crawler thread\n");
        return false;
    }
    if (g_settings.lru_maintainer_thread && !lru_maintainer_.Start()) {
        fprintf(stderr, "Failed to enable LRU maintainer thread\n");
        return false;
    }
    if (g_settings.slab_reassign && !slabs_maintainer_.Start() && !slabs_rebalancer_.Start()) {
        fprintf(stderr,"start slabs thread failed!\n");
        return false;
    }

    ItemManager::ClockHandler(0, 0, 0);
    return true;
}

bool MDB::StopMDB() {
    assoc_maintainer_.Stop();
    lru_crawler_.Stop();
    lru_maintainer_.Stop();
    slabs_maintainer_.Stop();
    slabs_rebalancer_.Stop();

    assoc_maintainer_.Wait();
    lru_crawler_.Wait();
    lru_maintainer_.Wait();
    slabs_maintainer_.Wait();
    slabs_rebalancer_.Wait();
    return true;
}

void MDB::WaitForThreads() {
    assoc_maintainer_.Wait();
    lru_crawler_.Wait();
    lru_maintainer_.Wait();
    slabs_maintainer_.Wait();
    slabs_rebalancer_.Wait();
}

MDB& MDB::GetInstance() {
    static MDB mdb_instance;
    return mdb_instance;
}

bool MDB::Put(const char* key, const char* value) {
}

bool MDB::Get(const char* key, std::string& value) {
    int32_t key_len = strlen(key);
    Item* it = item_manager_.ItemGet(key, key_len);
    if (NULL == it) {
        return  false;
    }
    char* data = ITEM_data(it);
    char* val = (char*)malloc(sizeof(char) * strlen(data));;
    value = val;
    item_manager_.ItemUpdate(it);
    return true;
}

bool MDB::MultiGet(vector<string> key_list, string& value) {
}

bool MDB::Delete(const char* key) {
}




}  // end of namespace mdb









/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
