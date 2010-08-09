#ifndef HEMP_VALUE_H
#define HEMP_VALUE_H

#include <hemp/core.h>
#include <hemp/text.h>
#include <hemp/hash.h>
#include <hemp/context.h>


/*--------------------------------------------------------------------------
 * Data structures
 *--------------------------------------------------------------------------*/

typedef union {
    hemp_int_t      integer;
    hemp_bool_t     boolean;
    hemp_char_t     chars[4];
} hemp_payload_t;

typedef struct {
    hemp_payload_t  value;
    hemp_u32_t      type;           /* FIXME: assumes little-endian */
} hemp_tagged_t;

union hemp_value_u {
    hemp_u64_t      bits;
    hemp_num_t      number;
    hemp_tagged_t   tagged;
};



/*--------------------------------------------------------------------------
 * Definitions and macros for manipulating NaN-boxed values
 *
 * As per IEEE-754 double precision numbers, the first bit is the sign bit,
 * the next 11 bits are the exponent and the remaining 52 are used to encode
 * the mantissa.  If all of the first 12 (sign + exponent) bits are 1 then 
 * the value represents +/- Infinity when all of the mantissa bits are 0, or
 * NaN if any of the mantissa bits are 1.  So setting the first 13 bits to 1 
 * will always represent NaN and we can use the remaining 51 bits to encode
 * some other kind of payload.  This widely used technique is commonly known 
 * as "NaN-boxing".
 *
 * In those 51 bits we can store a 32 bit integer.  We can also store a 
 * memory pointer.  On 32 bit architectures that also requires 32 bits.  On
 * all widely used (at the time of writing) 64-bit platforms, (up to) 47 bits 
 * are used to store memory adddresses.  That leaves us 4 bits spare which we
 * can use to encode up to 16 different data types.
 *--------------------------------------------------------------------------*/

#if HEMP_WORD_LENGTH == 32
    #define HEMP_PAYLOAD_MASK   0x00000000FFFFFFFFLL
    #define HEMP_PAYLOAD_BITS   32
#elif HEMP_WORD_LENGTH == 64
    #define HEMP_PAYLOAD_MASK   0x00007FFFFFFFFFFFLL
    #define HEMP_PAYLOAD_BITS   47
#else
    #error "Invalid word length"
#endif

#define HEMP_NAN_MASK           0xFFF8000000000000LL
#define HEMP_PAYLOAD(v)         (v.bits & HEMP_PAYLOAD_MASK)

#define HEMP_TYPE_BITS          4
#define HEMP_TYPE_SHIFT         47
#define HEMP_TYPE_MASK          0x0F
#define HEMP_TYPE_UP(t)         ((hemp_u64_t) (t & HEMP_TYPE_MASK) << HEMP_TYPE_SHIFT)
#define HEMP_TYPE_DOWN(v)       ((hemp_u8_t)  (v >> HEMP_TYPE_SHIFT) & HEMP_TYPE_MASK)
#define HEMP_TYPE_NAN_MASK(t)   (HEMP_NAN_MASK | HEMP_TYPE_UP(t))

#define HEMP_TYPE_NUM_ID        ((hemp_u8_t)  0x00)     /* 64 bit double    */
#define HEMP_TYPE_INT_ID        ((hemp_u8_t)  0x01)     /* 32 bit integer   */
#define HEMP_TYPE_STR_ID        ((hemp_u8_t)  0x02)     /* string pointer   */

#define HEMP_TYPE_NUM_MASK      ((hemp_u64_t) HEMP_TYPE_NUM_ID)
#define HEMP_TYPE_INT_MASK      HEMP_TYPE_NAN_MASK(HEMP_TYPE_INT_ID)
#define HEMP_TYPE_STR_MASK      HEMP_TYPE_NAN_MASK(HEMP_TYPE_STR_ID)

#define HEMP_TYPE(v)            HEMP_TYPE_DOWN(v.bits)
#define HEMP_IS_TYPE(v,t)       ((hemp_bool_t) HEMP_TYPE(v) == t)
#define HEMP_IS_NUM(v)          ((hemp_bool_t) ((hemp_u64_t) v.bits < HEMP_NAN_MASK))
#define HEMP_IS_INT(v)          HEMP_IS_TYPE(v, HEMP_TYPE_INT_ID)
#define HEMP_IS_STR(v)          HEMP_IS_TYPE(v, HEMP_TYPE_STR_ID)
#define HEMP_IS_TAGGED(v)       (! HEMP_IS_NUM(v))

//#define HEMP_TAG_IS_TYPE(v,t)   (! HEMP_TAG_IS_NUM(t))


/*--------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------*/


typedef hemp_text_p     (* hemp_text_vfn)(hemp_value_t, hemp_context_p, hemp_text_p);
typedef hemp_value_t    (* hemp_dot_vfn)(hemp_value_t, hemp_context_p, hemp_cstr_p);
typedef void            (* hemp_init_vfn)(hemp_value_t);
typedef void            (* hemp_wipe_vfn)(hemp_value_t);



/* A hemp_variable represents a typed data value being used in some expression.
 * It contains a reference to its value type, the parent value (i.e. container)
 * from where it originated, the local (variable) name for the value, and a 
 * hemp_data union containing the raw data value.  For example, foo.bar would
 * create a 'foo' value originated from the root variables stash, and a 'bar'
 * value with the parent pointer set to the 'foo' value.  The type of 'foo'
 * would presumably be a hash or object, and bar could be any value type.
 *
 * NOTE: this is being changed and/or phased out
 */
 
struct hemp_variable_s {
    hemp_vtype_p    vtype;
    hemp_cstr_p     name;             // TODO: symbol?  dup?  element?  text?
    hemp_value_t    value;
    hemp_variable_p parent;
//  TODO: flags?, next?
};


/* A hemp_vtype defines a lookup table (aka vtable) of functions (aka methods)
 * which perform the basic data operations on values of different types.
 * e.g. the 'dot' method for accessing a dotted part of a value, e.g. foo.bar.
 * It also contains a value type name and a reference to the hemp_types 
 * collection of which it is a part.  e.g. the "hemp" hemp_vtypes collection
 * contains hemp_vtype definitions for values using the builtin hemp_text,
 * hemp_list, hemp_hash, etc., data structures (aka objects).  A some point 
 * in the future a "perl" hemp_vtypes collection can provide hemp_vtype 
 * definitions for accessing Perl scalars, hash array and arrays.
 */

struct hemp_vtype_s {
    hemp_vtypes_p   vtypes;
    hemp_cstr_p     name;
    hemp_init_vfn   init;
    hemp_wipe_vfn   wipe;
    hemp_text_vfn   text;
    hemp_dot_vfn    dot;
/*
    set
    dot
    text
    value
    pairs
    dot_set   ??
    apply     # function application
    name fullname
*/
};




/* The hemp_vtypes data structure defines a collection of hemp_vtype value
 * types.  It is essentially a factory for creating value instances of 
 * different types.  The builtin data types (text, list, hash, etc) have
 * hard-coded entries in the data structure that can be accessed directly 
 * for the sake of efficiency.  Any other custom value types can be defined
 * in the types hash table which offers unlimited extensibility at the cost
 * of slower lookup time.
 */

struct hemp_vtypes_s {
    hemp_cstr_p     name;
    /* builtin value types */
    /* TODO: make this an array of 16 vtables for core types */
    hemp_vtype_p    text;
    hemp_vtype_p    list;
    hemp_vtype_p    hash;
    /* hash table for additional types */
    hemp_hash_p     types;
};




/* each value has a pointer to its type and the value iteself */
//#define HEMP_VALUE_BASE     \
//    hemp_type_t  type;      \
//    hemp_flags_t flags;     \
//    hemp_value_t next;
    
/* From TT3:

    # variable slots
    META    => '=0',
    CONTEXT => '=1',
    NAME    => '=2',
    VALUE   => '=3',
    PARENT  => '=4',

    # variable metadata slots
    CONFIG  => '=0',
    VARS    => '=1',
    METHODS => '=2',

    We want to unify CONTEXT/PARENT to be one and the same.
    
    foo       ===> [.] <-- [foo]             # parent/context is the root stash 
    bar.baz   ===> [.] <-- [bar] <-- [baz]
*/


/*--------------------------------------------------------------------------
 * function prototypes
 *--------------------------------------------------------------------------*/

extern hemp_value_t HEMP_NUM_VAL(hemp_num_t n);
extern hemp_value_t HEMP_INT_VAL(hemp_int_t n);
extern hemp_value_t HEMP_STR_VAL(hemp_cstr_p n);

extern hemp_num_t   HEMP_VAL_NUM(hemp_value_t v);
extern hemp_int_t   HEMP_VAL_INT(hemp_value_t v);
extern hemp_cstr_p  HEMP_VAL_STR(hemp_value_t v);


void hemp_dump_u64(hemp_u64_t value);
void hemp_dump_64(hemp_u64_t value);
void hemp_dump_32(hemp_u32_t value);
void hemp_dump_value(hemp_value_t value);


hemp_vtypes_p   hemp_vtypes_init();
void            hemp_vtypes_free(hemp_vtypes_p);
hemp_vtype_p    hemp_vtypes_new_type(hemp_vtypes_p, hemp_cstr_p);

/*
void            hemp_vtypes_free_vtype(hemp_hash_entry_p entry);
hemp_vtype_p    hemp_vtypes_type(hemp_vtypes_p, hemp_cstr_p);

hemp_vtype_p    hemp_vtype_init(hemp_vtypes_p, hemp_cstr_p);
void            hemp_vtype_free();
*/
/*hemp_value_t    hemp_value_init(hemp_vtype_p, hemp_cstr_p, hemp_data_t, hemp_value_t);
void            hemp_value_free(hemp_value_t); */


#endif /* HEMP_VALUE_H */
