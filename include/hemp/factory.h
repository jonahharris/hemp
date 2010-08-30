#ifndef HEMP_FACTORY_H
#define HEMP_FACTORY_H

#include <hemp/action.h>
#include <hemp/hash.h>


/*--------------------------------------------------------------------------
 * data structures
 *--------------------------------------------------------------------------*/

struct hemp_factory_s {
    hemp_hash_p      instances;
    hemp_hash_p      constructors;
    hemp_hash_each_f cleaner;
}; 


/*--------------------------------------------------------------------------
 * function prototypes
 *--------------------------------------------------------------------------*/

hemp_factory_p 
    hemp_factory_init();

void
    hemp_factory_free(
        hemp_factory_p
    );

hemp_bool_t
    hemp_factory_free_constructor(
        hemp_hash_p     dialects,
        hemp_pos_t      position,
        hemp_slot_p     slot
    );

hemp_action_p
    hemp_factory_register(      /* register a constructor action            */
        hemp_factory_p  factory,
        hemp_str_p      name,
        hemp_actor_f    actor,
        hemp_mem_p      script
    );

hemp_action_p
    hemp_factory_constructor(   /* fetch a constructor action               */
        hemp_factory_p  factory,
        hemp_str_p      name
    );

hemp_mem_p
    hemp_factory_instance(      /* fetch/construct a singleton instance     */
        hemp_factory_p  factory,
        hemp_str_p      name
    );



#endif /* HEMP_FACTORY_H */
