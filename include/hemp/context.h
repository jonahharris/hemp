#ifndef HEMP_CONTEXT_H
#define HEMP_CONTEXT_H

#include <hemp/core.h>
#include <hemp/pool.h>
#include <hemp/element.h>
#include <hemp/frame.h>
#include <hemp/type/hash.h>
#include <hemp/type/code.h>
#include <hemp/type/params.h>


/*--------------------------------------------------------------------------
 * type definitions
 *--------------------------------------------------------------------------*/

struct hemp_context {
    hemp_hemp       hemp;
    hemp_hash       vars;
    hemp_frame      frame;
    hemp_pool       text_pool;          /* tmp hack */
    hemp_pool       list_pool;          /* tmp hack */
    hemp_pool       hash_pool;          /* tmp hack */
    hemp_pool       code_pool;          /* tmp hack */
    hemp_pool       params_pool;        /* tmp hack */
};


/*--------------------------------------------------------------------------
 * function prototypes
 *--------------------------------------------------------------------------*/

hemp_context
hemp_context_new(
    hemp_hemp       hemp
);

void
hemp_context_free(
    hemp_context    context
);

HEMP_INLINE hemp_text
hemp_context_tmp_text(
    hemp_context    context
);

HEMP_INLINE hemp_text
hemp_context_tmp_text_size(
    hemp_context    context,
    hemp_size       size
);

HEMP_INLINE hemp_list
hemp_context_tmp_list(
    hemp_context    context
);

HEMP_INLINE hemp_hash
hemp_context_tmp_hash(
    hemp_context    context
);

HEMP_INLINE hemp_code
hemp_context_tmp_code(
    hemp_context    context
);

HEMP_INLINE hemp_params
hemp_context_tmp_params(
    hemp_context    context
);

hemp_bool
hemp_context_text_pool_cleaner(
    hemp_memory     item
);

hemp_bool
hemp_context_list_pool_cleaner(
    hemp_memory     item
);

hemp_bool
hemp_context_hash_pool_cleaner(
    hemp_memory     item
);

hemp_bool
hemp_context_code_pool_cleaner(
    hemp_memory     item
);

hemp_bool
hemp_context_params_pool_cleaner(
    hemp_memory     item
);

HEMP_INLINE hemp_frame
hemp_context_frame(
    hemp_context    context
);

HEMP_INLINE hemp_frame
hemp_context_enter(
    hemp_context    context,
    hemp_element    element
);

HEMP_INLINE hemp_element
hemp_context_leave(
    hemp_context    context
);

HEMP_INLINE void
hemp_context_focus_params(
    hemp_context    context
);

HEMP_INLINE void
hemp_context_blur_params(
    hemp_context    context
);


/*--------------------------------------------------------------------------
 * macros
 *--------------------------------------------------------------------------*/

#define hemp_context_set(c, k, v)           \
    hemp_hash_store(c->vars, k, v)

#define hemp_context_set_number(c, k, n)    \
    hemp_hash_store_number(c->vars, k, n)

#define hemp_context_set_integer(c, k, i)   \
    hemp_hash_store_integer(c->vars, k, i)

#define hemp_context_set_pointer(c, k, p)   \
    hemp_hash_store_pointer(c->vars, k, p)

#define hemp_context_set_string(c, k, s)    \
    hemp_hash_store_string(c->vars, k, s)

#define hemp_context_set_text(c, k, t)      \
    hemp_hash_store_text(c->vars, k, t)

#define hemp_context_set_list(c, k, l)      \
    hemp_hash_store_list(c->vars, k, l)

#define hemp_context_set_hash(c, k, h)      \
    hemp_hash_store_hash(c->vars, k, h)

#define hemp_context_get(c, k)              \
    hemp_hash_fetch(c->vars, k)

#define hemp_context_get_number(c, k)       \
    hemp_hash_fetch_number(c->vars, k)

#define hemp_context_get_integer(c, k)      \
    hemp_hash_fetch_integer(c->vars, k)

#define hemp_context_get_pointer(c, k)      \
    hemp_hash_fetch_pointer(c->vars, k)

#define hemp_context_get_string(c, k)       \
    hemp_hash_fetch_string(c->vars, k)


#endif /* HEMP_CONTEXT_H */
