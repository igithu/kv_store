/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file mdb.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/03/09 20:00:50
 * @brief
 *
 **/




#ifndef __MDB_H
#define __MDB_H

#include <string>

#include "item_manager.h"
#include "lru_crawler.h"
#include "lru_maintainer.h"
#include "slabs_manager.h"
#include "slabs_rebalancer.h"
#include "slabs_maintainer.h"
#include "assoc_maintainer.h"
#include "disallow_copy_and_assign.h"



namespace mdb {

class MDB {
    public:
        ~MDB();

        bool InitDB();
        bool StartMDB();
        bool StopMDB();
        void WaitForThreads();

        static MDB& GetInstance();

        bool Put(const char* key, const char* value);
        bool Get(const char* key, std::string& value);
        bool Delete(const char* key);

    private:
        MDB();

        DISALLOW_COPY_AND_ASSIGN(MDB);

    private:
        ItemManager& item_manager_;
        SlabsManager& slabs_manager_;

        AssocMaintainer& assoc_maintainer_;
        LRUMaintainer& lru_maintainer_;
        SlabsMaintainer& slabs_maintainer_;

        LRUCrawler& lru_crawler_;
        SlabsRebalancer& slabs_rebalancer_;
};


}  // end of namespace mdb






#endif // __MDB_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
