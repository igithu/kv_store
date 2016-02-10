/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file stats_manager.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/10 22:38:33
 * @brief
 *
 **/




#ifndef __STATS_MANAGER_H
#define __STATS_MANAGER_H

class StatsManager {
    public:
        void InitPrefix();
        void ClearPrefix(void);
        void GetPrefixRecord(const char *key, const size_t nkey, const bool is_hit);
        void DeletePrefixRecord(const char *key, const size_t nkey);
        void SetPrefixRecord(const char *key, const size_t nkey);
        char *DumpPrefix(int *length);

    private:
};




#endif // __STATS_MANAGER_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
