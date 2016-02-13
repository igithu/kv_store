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

ItemLRUCrawler::ItemLRUCrawler() :
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

void InitLRUCrawler::InitLRUCrawler() {
}

enum CrawlerResultType ItemLRUCrawler::LRUCrawl(char *slabs) {
}

void ItemLRUCrawler::PauseCrawler() {
}

void ItemLRUCrawler::ResumeCrawler() {
}

int ItemLRUCrawler::DoLRUCrawlerStart(uint32_t id, uint32_t remaining) {
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
