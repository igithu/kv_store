/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file slabs_manager.h
 * @author aishuyu(asy5178@163.com)
 * @date 2016/02/08 22:36:52
 * @brief
 *
 **/




#ifndef __SLABS_H
#define __SLABS_H

#define MAX_NUMBER_OF_SLAB_CLASSES (63 + 1)

struct Slab {
    void *slab_start;
    void *slab_end;
    void *slab_pos;
    int s_clsid;
    int d_clsid;
    int busy_items;
    uint8_t done;
};

struct SlabClass {
    unsigned int size;      /* sizes of items */
    unsigned int perslab;   /* how many items per slab */

    void *slots;           /* list of item ptrs */
    unsigned int sl_curr;   /* total free items in list */

    unsigned int slabs;     /* how many slabs were allocated for this class */

    void **slab_list;       /* array of slab pointers */
    unsigned int list_size; /* size of prev array */
};

enum ReassignResultType {
    REASSIGN_OK=0,
    REASSIGN_RUNNING,
    REASSIGN_BADCLASS,
    REASSIGN_NOSPARE,
    REASSIGN_SRC_DST_SAME
};

enum MoveStatus {
    MOVE_PASS=0,
    MOVE_FROM_SLAB,
    MOVE_FROM_LRU,
    MOVE_BUSY,
    MOVE_LOCKED
};

extern Slab g_slab_rebal;
extern volatile int g_slab_rebalance_signal;

class SlabsManager {
    public:
        ~SlabsManager();

        static SlabsManager& GetInstance();

        bool Start();

        /*
         * Init the subsystem. 1st argument is the limit on no. of bytes to allocate,
         * 0 if no limit. 2nd argument is the growth factor; each slab will use a chunk
         * size equal to the previous slab's chunk size times this factor.
         * 3rd argument specifies if the slab allocator should allocate all memory
         * up front (if true), or allocate memory in chunks as it is needed (if false)
         */
        void InitSlabs(const size_t limit, const double factor, const bool prealloc);

        /*
         * Given object size, return id to use when allocating/freeing memory for object
         * 0 means error: can't store such a large object
         */
        unsigned int SlabsClsid(const size_t size);

        /*
         * Allocate object of given length. 0 on error
         * */ /*@null@*/
        void *SlabsAllocator(const size_t size, unsigned int id, unsigned int *total_chunks);

        /*
         * Free previously allocated object
         */
        void FreeSlabs(void *ptr, size_t size, unsigned int id);

        /*
         * Return a datum for stats in binary protocol
         */
        bool GetSlabStats(const char *stat_type, int nkey, ADD_STAT add_stats, void *c);

        /*
         * Fill buffer with stats
         */ /*@null@*/
        void SlabsStats(ADD_STAT add_stats, void *c);

        /*
         * Hints as to freespace in slab class
         */
        unsigned int SlabsAvailableChunks(unsigned int id, bool *mem_flag, unsigned int *total_chunks);
        enum ReassignResultType SlabsReassign(int src, int dst);
         /*
          * If we hold this lock, rebalancer can't wake up or move
          */
        void PauseSlabsRebalancer();
        void ResumeSlabsRebalancer();

    private:
        SlabsManager();

        int NewSlab(const unsigned int id);
        void *MemoryAllocator(size_t size);
        void FreeSlabs(void *ptr, const size_t size, unsigned int id);

        int DoSlabsNewSlab(const unsigned int id);
        void *DoSlabsAlloc(const size_t size, unsigned int id, unsigned int *total_chunks);
        void DoSlabsFree(void *ptr, const size_t size, unsigned int id);
        void DoSlabsStats(ADD_STAT add_stats, void *c);
        enum ReassignResultType DoSlabsReassign(int src, int dst);

        int GrowSlabList(const unsigned int id);
        void FreeSlabPage(char *ptr, const unsigned int id);
        /*
         * Iterate at most once through the slab classes and pick a "random" source.
         * I like this better than calling rand() since rand() is slow enough that we
         * can just check all of the classes once instead.
         */
        int SlabsReassignPickany(int dst);

        DISALLOW_COPY_AND_ASSIGN(SlabsManager);

    protected:

        /* Return 1 means a decision was reached.
         * Move to its own thread (created/destroyed as needed) once automover is mor
         * complex.
         */
        int SlabAutomoveDecision(int *src, int *dst);
        void SlabsAdjustMemRequested(unsigned int id, size_t old, size_t ntotal);
        int SlabRbalanceMove();


    private:
        SlabClass slabclass[MAX_NUMBER_OF_SLAB_CLASSES];

        size_t mem_limit_;
        size_t mem_malloced_;

        /*
         * If the memory limit has been hit once. Used as a hint to decide when to
         * early-wake the LRU maintenance thread
         */
        bool mem_limit_reached_;
        int power_largest_;

        void *mem_base_;
        void *mem_current_;
        size_t mem_avail_;

        /*
         * Access to the slab allocator is protected by this lock
         */
        pthread_mutex_t slabs_lock_;
        pthread_mutex_t slabs_rebalance_lock_;

};


#endif // __SLABS_H



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
