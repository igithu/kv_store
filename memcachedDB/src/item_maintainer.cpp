/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file item_maintainer.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/11 23:40:59
 * @brief
 *
 **/


#include "item_maintainer.h"

ItemMaintainer::ItemMaintainer() {
}

ItemMaintainer::~ItemMaintainer() {
}


Item *ItemMaintainer::DoItemAlloc(
        char *key,
        const size_t nkey,
        const int flags,
        const rel_time_t exptime,
        const int nbytes,
        const uint32_t cur_hv) {
}

void ItemMaintainer::FreeItem(Item *it) {
}

bool ItemMaintainer::ItemSizeOk(const size_t nkey, const int flags, const int nbytes) {
}

int  ItemMaintainer::DoItemLink(Item *it, const uint32_t hv) {
}

void ItemMaintainer::DoItemUnlink(Item *it, const uint32_t hv) {
}

void ItemMaintainer::DoItemUnlinkNolock(Item *it, const uint32_t hv) {
}

void ItemMaintainer::DoItemRemove(Item *it) {
}

void ItemMaintainer::DoItemUpdate(Item *it) {
}

void ItemMaintainer::DoItemUpdateNolock(Item *it) {
}

int  ItemMaintainer::DoItemReplace(Item *it, Item *new_it, const uint32_t hv) {
}

enum StoreItemType ItemMaintainer::DoStoreItem(const uint32_t hv, Item* it, int32_t op) {
}

Item *ItemMaintainer::DoItemGet(const char *key, const size_t nkey, const uint32_t hv) {
}

Item *ItemMaintainer::DoItemTouch(const char *key, const size_t nkey, uint32_t exptime, const uint32_t hv) {
}

char *ItemMaintainer::ItemCacheDump(const unsigned int slabs_clsid, const unsigned int limit, unsigned int *bytes) {
}

void ItemMaintainer::ItemStats(ADD_STAT add_stats, void *c) {
}

void ItemMaintainer::ItemStatsTotals(ADD_STAT add_stats, void *c) {
}

void ItemMaintainer::ItemStatsSizes(ADD_STAT add_stats, void *c) {
}













/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
