/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file items_op.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/01/23 22:37:30
 * @brief
 *
 **/


#include "items_op.h"

/*
 * Stores an item in the cache according to the semantics of one of the set
 * commands. In threaded mode, this is protected by the cache lock.
 *
 * Returns the state of storage.
 */
enum store_item_type do_store_item(const uint32_t hv, item* it, int32_t op) {
    char *key = ITEM_key(it);
    item *old_it = do_item_get(key, it->nkey, hv);
    enum store_item_type stored = NOT_STORED;

    item *new_it = NULL;
    int flags;

    if (old_it != NULL && NREAD_ADD == op) {
        /* add only adds a nonexistent item, but promote to head of LRU */
        do_item_update(old_it);
    } else if (!old_it && (NREAD_REPLACE == op || NREAD_APPEND == op || NREAD_PREPEND == op)) {
        /* replace only replaces an existing value; don't store */
    } else if (NREAD_CAS == op) {
        /* validate cas operation */
        if(old_it == NULL) {
            // LRU expired
            stored = NOT_FOUND;
        } else if (ITEM_get_cas(it) == ITEM_get_cas(old_it)) {
            // cas validates
            // it and old_it may belong to different classes.
            // I'm updating the stats for the one that's getting pushed out

            item_replace(old_it, it, hv);
            stored = STORED;
        } else {
            stored = EXISTS;
        }
    } else {
        /*
         * Append - combine new and old record into single one. Here it's
         * atomic and thread-safe.
         */
        if (NREAD_APPEND == op || NREAD_PREPEND == op) {
            /*
             * Validate CAS
             */
            if (ITEM_get_cas(it) != 0) {
                // CAS much be equal
                if (ITEM_get_cas(it) != ITEM_get_cas(old_it)) {
                    stored = EXISTS;
                }
            }

            if (NOT_STORED == stored) {
                /* we have it and old_it here - alloc memory to hold both */
                /* flags was already lost - so recover them from ITEM_suffix(it) */

                flags = (int) strtol(ITEM_suffix(old_it), (char **) NULL, 10);
                new_it = do_item_alloc(key, it->nkey, flags, old_it->exptime, it->nbytes + old_it->nbytes - 2 /* CRLF */, hv);

                if (new_it == NULL) {
                    /* SERVER_ERROR out of memory */
                    if (old_it != NULL)
                        do_item_remove(old_it);

                    return NOT_STORED;
                }

                /* copy data from it and old_it to new_it */

                if (NREAD_APPEND == op) {
                    memcpy(ITEM_data(new_it), ITEM_data(old_it), old_it->nbytes);
                    memcpy(ITEM_data(new_it) + old_it->nbytes - 2 /* CRLF */, ITEM_data(it), it->nbytes);
                } else {
                    /* NREAD_PREPEND */
                    memcpy(ITEM_data(new_it), ITEM_data(it), it->nbytes);
                    memcpy(ITEM_data(new_it) + it->nbytes - 2 /* CRLF */, ITEM_data(old_it), old_it->nbytes);
                }

                it = new_it;
            }
        }

        if (NOT_STORED == stored) {
            if (old_it != NULL)
                item_replace(old_it, it, hv);
            else
                do_item_link(it, hv);
            stored = STORED;
        }
    }

    if (old_it != NULL)
        do_item_remove(old_it);         /* release our reference */
    if (new_it != NULL)
        do_item_remove(new_it);
    return stored;
}









/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
