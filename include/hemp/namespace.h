#ifndef HEMP_NAMESPACE_H
#define HEMP_NAMESPACE_H

#include <hemp/core.h>
#include <hemp/type/hash.h>


struct hemp_namespace {
    hemp_u16        id;
    hemp_string     path;
    hemp_string     name;
    hemp_namespace  parent;
    hemp_hash       children;
};

//struct hemp_namespaces {
//    hemp_u16        next_id;
//    hemp_string     name;
//    hemp_hash       namespaces;
//};


/*--------------------------------------------------------------------------
 * function prototypes
 *--------------------------------------------------------------------------*/

hemp_namespace
hemp_namespace_init(
    hemp_u16        id,
    hemp_string     name,
    hemp_namespace  parent
);

hemp_namespace
hemp_namespace_child(
    hemp_namespace  namespace,
    hemp_string     name
);

hemp_namespace
hemp_resolve_namespace(
    hemp_string     path
);

void
hemp_namespace_free(
    hemp_namespace  namespace
);

hemp_bool
hemp_namespace_free_child(
    hemp_hash       namespaces,
    hemp_pos        position,
    hemp_slot       item
);




/*--------------------------------------------------------------------------
 * macros
 *--------------------------------------------------------------------------*/

#define hemp_namespace_next_id()                                \
    (++HempGlobal.namespace_id)


#define hemp_namespace_root(name)                               \
    hemp_namespace_child(                                       \
        HempGlobal.namespace, name                              \
    )

#define hemp_namespace_instance(name) (                         \
    strchr(name, HEMP_DOT)                                      \
        ? hemp_resolve_namespace(name)                          \
        : hemp_namespace_root(name)                             \
)

#define hemp_namespace_id(name) (                               \
    hemp_namespace_instance(name)->id                           \
)


#endif /* HEMP_NAMESPACE_H */
