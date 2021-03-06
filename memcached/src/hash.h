/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file hash.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/06 21:49:12
 * @brief
 *
 **/




#ifndef __HASH_H
#define __HASH_H

#include <stdint.h>
#include <cstddef>

typedef uint32_t (*HashFunc)(const void *key, size_t length);

HashFunc Hash;

enum HashfuncType {
    JENKINS_HASH = 0, MURMUR3_HASH
};

int InitHash(enum HashfuncType type);



#endif // __HASH_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
