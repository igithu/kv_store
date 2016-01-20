/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file memcached_operator.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/01/20 23:25:02
 * @brief
 *
 **/




#ifndef __MEMCACHED_OPERATOR_H
#define __MEMCACHED_OPERATOR_H


static void process_update(conn *c, char* key, char* value, int comm, bool handle_cas);




#endif // __MEMCACHED_OPERATOR_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
