/* A pool performs block memory allocation for multiple records of a fixed 
 * size.  It stores the head of a linked list of memory slabs and bits of 
 * housekeeping data including the records size, number of records currently 
 * in use and the record capacity for which memory has been pre-allocated.
 */

#include "hemp/pool.h"
#include "hemp/debug.h"


hemp_pool_t 
hemp_pool_init(
    hemp_size_t size, 
    hemp_size_t capacity
) {
    hemp_pool_t pool;

    if ((pool = (hemp_pool_t) hemp_mem_init(sizeof(struct hemp_pool)))) {
        if ((pool->slab    = hemp_slab_init(size * capacity))) {
            pool->next     = pool->slab->data;
            pool->size     = size;
            pool->capacity = capacity;
            pool->used     = 0;
            debug_cyan(
                "Allocated pool buffer of %d x %d = %d bytes\n", 
                size, capacity, size * capacity
            );
        }
        else {
            hemp_pool_free(pool);
            pool = NULL;
        }
    }
    
    // TODO: handle pool == NULL
    
    return pool;
}


hemp_ptr_t
hemp_pool_take(hemp_pool_t pool) {
    hemp_ptr_t value;

    if (pool->used == pool->capacity)
        hemp_pool_grow(pool);

    debug_yellow("Returning pool item %d\n", pool->used);

    value = pool->next;
    pool->next += pool->size;
    pool->used++;
    return value;
}


void 
hemp_pool_grow(
    hemp_pool_t pool
) {
    // create a new slab as large as our current capacity (so the pool size
    // doubles on each growth) and insert it at the head of the slab list
    hemp_slab_t slab = hemp_slab_init(pool->size * pool->capacity);
    slab->next = pool->slab;
    pool->slab = slab;
    pool->next = slab->data;
    pool->capacity *= 2;
    debug_yellow("Increased pool capacity to %d\n", pool->capacity);
}


void
hemp_pool_free(
    hemp_pool_t pool
) {
    if (pool->slab) {
        debug_cyan("Releasing pool slab\n");
        hemp_slab_free(pool->slab);
    }
    hemp_mem_free(pool);
}
