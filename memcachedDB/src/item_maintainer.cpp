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

ItemMaintainer& ItemMaintainer::GetInstance() {
    static ItemMaintainer im_instance;
    return im_instance;
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

Item *ItemMaintainer::GetItemSizeByIndex(int32_t index) {
}

void ItemMaintainer::ItemSizeIncrement(int32_t index) {
}

void ItemMaintainer::ItemSizeDecrement(int32_t index) {
}

Item *ItemMaintainer::ItemAlloc(char *key, size_t nkey, int flags, rel_time_t exptime, int nbytes) {
}

Item *ItemMaintainer::ItemGet(const char *key, const size_t nkey) {
}

Item *ItemMaintainer::ItemTouch(const char *key, const size_t nkey, uint32_t exptime) {
}

int ItemMaintainer::ItemLink(Item *it) {
}

void ItemMaintainer::ItemRemove(Item *it) {
}

int ItemMaintainer::ItemReplace(Item *it, Item *new_it, const uint32_t hv) {
}

void ItemMaintainer::ItemUnlink(Item *it) {
}

void ItemMaintainer::ItemUpdate(Item *it) {
}

enum StoreItemType ItemMaintainer::StoreItem(Item *item, int op) {
}

rel_time_t ItemMaintainer::GetCurrentTime() {
    return current_time_;
}

void ItemMaintainer::ClockHandler(struct ev_loop *loop,ev_timer *timer_w,int e) {
}












/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
