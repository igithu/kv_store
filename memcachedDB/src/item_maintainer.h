/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file item_maintainer.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/09 23:29:38
 * @brief
 *
 **/




#ifndef __ITEM_MAINTAINER_H
#define __ITEM_MAINTAINER_H



enum StoreItemType {
    NOT_STORED=0, STORED, EXISTS, NOT_FOUND
};

class ItemMaintainer : public Thread {
    public:
        ItemMaintainer();
        ~ItemMaintainer();

        item *DoItemAlloc(
                char *key,
                const size_t nkey,
                const int flags,
                const rel_time_t exptime,
                const int nbytes,
                const uint32_t cur_hv);

        void FreeItem(item *it);
        bool ItemSizeOk(const size_t nkey, const int flags, const int nbytes);

        /*
         * may fail if transgresses limits
         */
        int  DoItemLink(item *it, const uint32_t hv);
        void DoItemUnlink(item *it, const uint32_t hv);
        void DoItemUnlinkNolock(item *it, const uint32_t hv);
        void DoItemRemove(item *it);

        /*
         * update LRU time to current and reposition
         */
        void DoItemUpdate(item *it);
        void DoItemUpdateNolock(item *it);
        int  DoItemReplace(item *it, item *new_it, const uint32_t hv);

        enum StoreItemType DoStoreItem(const uint32_t hv, item* it, int32_t op);
        item *DoItemGet(const char *key, const size_t nkey, const uint32_t hv);
        item *DoItemTouch(const char *key, const size_t nkey, uint32_t exptime, const uint32_t hv);

        char *ItemCacheDump(const unsigned int slabs_clsid, const unsigned int limit, unsigned int *bytes);
        void ItemStats(ADD_STAT add_stats, void *c);
        void ItemStatsTotals(ADD_STAT add_stats, void *c);
        void ItemStatsSizes(ADD_STAT add_stats, void *c);
};





#endif // __ITEM_MAINTAINER_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
