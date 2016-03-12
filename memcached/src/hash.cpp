/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file hash.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2016/03/12 17:15:53
 * @brief
 *
 **/

#include "hash.h"

#include "global.h"
#include "jenkins_hash.h"
#include "murmur3_hash.h"


int InitHash(enum HashfuncType type) {
    switch (type) {
        case JENKINS_HASH:
            Hash = jenkins_hash;
            g_settings.hash_algorithm = "jenkins";
            break;
        case MURMUR3_HASH:
            Hash = MurmurHash3_x86_32;
            g_settings.hash_algorithm = "murmur3";
            break;
        default:
            return -1;
    }
    return 0;
}











/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
