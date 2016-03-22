/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file util.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/10 22:07:53
 * @brief
 *
 **/




#ifndef __UTIL_H
#define __UTIL_H

unsigned short RefcountIncr(unsigned short *refcount);
unsigned short RefcountDecr(unsigned short *refcount);
void StatsLock(void);
void StatsUnlock(void);

/*
 * Get the next CAS id for a new item.
 * TODO: refactor some atomics for this.
 */
uint64_t GetCasId();



/*
 * stats interface
 */
// void stats_prefix_init(void);
// void stats_prefix_clear(void);
// void stats_prefix_record_get(const char *key, const size_t nkey, const bool is_hit);
// void stats_prefix_record_delete(const char *key, const size_t nkey);
// void stats_prefix_record_set(const char *key, const size_t nkey);
// /*@null@*/
// char *stats_prefix_dump(int *length);


bool safe_strtoull(const char *str, uint64_t *out);
bool safe_strtoll(const char *str, int64_t *out);
bool safe_strtoul(const char *str, uint32_t *out);
bool safe_strtol(const char *str, int32_t *out);


#ifdef __GCC
# define __gcc_attribute__ __attribute__
#else
# define __gcc_attribute__(x)
#endif

/**
 * Vararg variant of perror that makes for more useful error messages
 * when reporting with parameters.
 *
 * @param fmt a printf format
 */
void vperror(const char *fmt, ...)
    __gcc_attribute__ ((format (printf, 1, 2)));

#endif // __UTIL_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
