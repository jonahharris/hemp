#ifndef HEMP_MODULE_H
#define HEMP_MODULE_H

#include "hemp/types.h"
//#include <ltdl.h>
#include <dlfcn.h>

hemp_bool
    hemp_module_load(
        hemp_t     hemp,
        hemp_string name
    );


#endif /* HEMP_MODULE_H */
