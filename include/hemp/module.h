#ifndef HEMP_MODULE_H
#define HEMP_MODULE_H

#include "hemp/core.h"
//#include <ltdl.h>
#include <dlfcn.h>


/*--------------------------------------------------------------------------
 * type definitions
 *--------------------------------------------------------------------------*/

//enum hemp_module_status {
//    HEMP_MODULE_INIT,
//    HEMP_MODULE_LOADED,
//    HEMP_MODULE_FAILED
//};

struct hemp_module {
    hemp_string         name;
    hemp_string         error;
    hemp_memory         handle;
//    hemp_action         ctor;
//    hemp_module_status  status;
};


/*--------------------------------------------------------------------------
 * function prototypes
 *--------------------------------------------------------------------------*/

hemp_module
hemp_module_new(
    hemp_string     name
);


void
hemp_module_free(
    hemp_module     module
);


hemp_bool
hemp_module_load(
    hemp_module     module
);

hemp_bool
hemp_module_unload(
    hemp_module     module
);


#endif /* HEMP_MODULE_H */
