#ifndef HEMP_VALUE_H
#define HEMP_VALUE_H

#include <hemp/core.h>
#include <hemp/macros.h>
#include <hemp/type.h>
#include <hemp/type/text.h>
#include <hemp/type/hash.h>


HEMP_DO_INLINE hemp_str_p hemp_identity_name(hemp_value_t value);

/*--------------------------------------------------------------------------
 * Data structures
 *--------------------------------------------------------------------------*/

/* now in hemp/types.h */


/*--------------------------------------------------------------------------
 * global identity (singleton) values
 *--------------------------------------------------------------------------*/

extern const hemp_value_t HempMissing;
extern const hemp_value_t HempNothing;
extern const hemp_value_t HempFalse;
extern const hemp_value_t HempTrue;
extern const hemp_value_t HempBefore;
extern const hemp_value_t HempEqual;
extern const hemp_value_t HempAfter;



/*--------------------------------------------------------------------------
 * Definitions and macros for manipulating NaN-boxed values
 *
 * Hemp values are represented using 64 bit words (one machine word on 64 bit
 * architectures, 2 words on 32 bit) that can store a double precision 
 * floating point number, a full-width 32 bit integer, a short sequence of
 * characters, or a memory pointer.  This technique is known as "NaN-boxing".

 * As per IEEE-754 double precision numbers, the first bit is the sign bit,
 * the next 11 bits are the exponent and the remaining 52 are used to encode
 * the mantissa.  If all of the first 12 (sign + exponent) bits are 1 then 
 * the value is deemed to be outside the range of tangible numbers that can 
 * be represented.  If all the mantissa bits are 0 then it is +/- Infinity.
 * If any other mantissa bit is set then it Not A Number (NaN).  We set the
 * first (highest) mantissa bit to represent the canonical NaN value (this 
 * appears to be common practice, if not mandated by IEEE-754).  All the 
 * remaining bit patterns from 0xFFF0000000000001 to 0xFFF7FFFFFFFFFFFF,
 * and 0xFFF8000000000001 to 0xFFFFFFFFFFFFFFFF are therefore unused to 
 * represent double precision floating point numbers and can be repurposed 
 * to carry other payloads.
 *
 * In those 52 bits we can easily store a 32 bit integer.  We can also store 
 * a memory pointer.  On 32 bit architectures that also requires 32 bits.  On
 * all widely used (at the time of writing) 64-bit platforms, (up to) 47 bits 
 * are used to store memory adddresses (don't worry, that still gives you an
 * addressable memory space of 128Tb - that should be enough for anyone...)
 * That leaves us 5 bits spare which we can use to encode up to 32 different 
 * data type tags.
 *
 * So we use the first 5 (high) mantissa bits to encode a type tag.  This is 
 * necessarily set to 0 (0b00000) for Infinity and 16 (0x80 / 0b10000) for 
 * NaN values, both of which have all the other mantissa bits set to 0.
 * However, this address space is not entirely wasted as we can use either 
 * of these tag types to encode further identity values as long as the 
 * mantissa has at least one bit set.  For example, we can create unique 
 * bit patterns (that can't be generated by any "natural" hemp process) to 
 * represent values like Missing, Nothing (both variants of "undefined"),
 * False, True, Before, Equal, After, Finished, and so on.  These are known
 * as "identity values" and are encoded as the values NaN + n.
 *
 * We currently use the lowest 6 bits to define identity values.  Each bit
 * encodes a flag.
 *
 *   000001  ==  0x01  ==  01       Finite (i.e. not the Infinity value)
 *   000010  ==  0x02  ==  02       Found (i.e. not a missing value)
 *   000100  ==  0x04  ==  04       Defined (i.e. not undef, void, null, etc)
 *   001000  ==  0x08  ==  08       Truth (0=false, 1=true)
 *   010000  ==  0x10  ==  16       Not After (less than or equal to)
 *   100000  ==  0x20  ==  32       Not Before (greater than or equal to)
 *
 * These bits are combined to express specific identity values.  For example,
 * the boolean values True and False each have the Defined, Found and Finite
 * bits set and vary in the setting of the Truth bit.  The comparison values
 * Before, Equal and After are tri-state values used by operators like 'cmp'
 * and '<=>' and are analogous to the -1, 0, +1 values typically returned.
 * Before and After are both considered True (in the sense of comparison, it
 * is true that the two values differ) and thus have the same bits set as for
 * the True identity value, while Equal is considered false. We set the 
 * Not After bit on Equal and Before, and the Not Before bit on Equal and 
 * After.  Thus, Equal can be considered to be neither before nor after.
 * It is admittedly a rather clumsy way of expressing it, but the obvious
 * approach of having "After" and "Before" bits instead of "Not After" and
 * "Not Before" would result in the Equal value having no additional bits
 * set over the False value, and thus there would be no way to tell them
 * apart (which perhaps doesn't really matter in the long run, but this is
 * how it is for now).
 *
 * UPDATE: this next bit isn't true any more, or at the moment....
 *
 * The top two bits gives us a vtable entry as an offset from entry 
 * HEMP_TYPE_MASK (16) in the global vtable list.
 *
 *   00     Undefined values: HempMissing, HempNothing
 *   01     Boolean values: HempTrue, HempFalse
 *   10     Comparison values: HempBefore, HempEqual, HempAfter
 *
 * UPDATE: list of types is growing/changing, so don't trust the next bit.
 * 
 * The other 30 values that can be encoded in those 5 bits represent different
 * builtin data types.
 *
 *    00 00000  Infinity and other identity values
 *    01 00001  32 bit integers
 *    02 00010  untyped C pointer (e.g. a generic void *)
 *    03 00011  heap allocated C string
 *    ...etc...
 *
 * Double:       [sign x 1][exponent x 11][mantissa x 57]
 * Tagged:       [sign x 1][exponent x 11][type x 5][payload x 47]
 * Pointer(64):  [sign x 1][exponent x 11][type x 5][pointer x 47]
 * Pointer(32):  [sign x 1][exponent x 11][type x 5][empty x 15][pointer x 32]
 * Integer:      [sign x 1][exponent x 11][type x 5][empty x 15][integer x 32]
 *--------------------------------------------------------------------------*/

/* tag types */
#define HEMP_NUMBER_ID          0x00        /* 64 bit double                */
#define HEMP_INTEGER_ID         0x01        /* 32 bit integer               */
#define HEMP_POINTER_ID         0x02        /* memory pointer/string        */
#define HEMP_STRING_ID          0x03        /* memory pointer/string        */
#define HEMP_TEXT_ID            0x04        /* hemp_text_p pointer          */
#define HEMP_LIST_ID            0x05        /* hemp_list_p pointer          */
#define HEMP_HASH_ID            0x06        /* hemp_hash_p pointer          */
#define HEMP_OBJECT_ID          0x0F        /* hemp_object_p pointer        */
#define HEMP_IDENTITY_ID        0x10        /* identity values (NaN + n)    */

/* out-of-band values used to mark unused/reserved type slots */
#define HEMP_UBER_ID            0x20        /* type uber type               */
#define HEMP_VALUE_ID           0x21        /* value uber type              */
#define HEMP_RESERVED_ID        0x30        /* reserved for future hemp use */
#define HEMP_UNUSED_ID          0x40        /* available for custom types   */

/* bits to twiddle to encode meaning into identity values */
#define HEMP_FINITE_BIT         0x01        /* not infinity                 */
#define HEMP_FOUND_BIT          0x02        /* not missing                  */
#define HEMP_DEFINED_BIT        0x04        /* not undefined                */
#define HEMP_TRUE_BIT           0x08        /* boolean true/false value     */
#define HEMP_NOT_AFTER_BIT      0x10        /* less than or equal to        */
#define HEMP_NOT_BEFORE_BIT     0x20        /* greater than or equal to     */
#define HEMP_COMPARE_BITS       (HEMP_NOT_BEFORE_BIT | HEMP_NOT_AFTER_BIT)

/* twiddle dem bits to define indentity values */
#define HEMP_IDENT_MISSING      (HEMP_FINITE_BIT)                         /* 000001 */
#define HEMP_IDENT_NOTHING      (HEMP_FINITE_BIT | HEMP_FOUND_BIT)        /* 000011 */
#define HEMP_IDENT_DEFINED      (HEMP_FINITE_BIT | HEMP_FOUND_BIT | HEMP_DEFINED_BIT)
/* bollox!   don't have a separate Empty identity value */
#define HEMP_IDENT_FALSE        (HEMP_IDENT_DEFINED)                       /* 000111 */
#define HEMP_IDENT_TRUE         (HEMP_IDENT_FALSE | HEMP_TRUE_BIT)         /* 001111 */
#define HEMP_IDENT_BEFORE       (HEMP_IDENT_TRUE  | HEMP_NOT_AFTER_BIT)    /* 011111 */
#define HEMP_IDENT_AFTER        (HEMP_IDENT_TRUE  | HEMP_NOT_BEFORE_BIT)   /* 101111 */
#define HEMP_IDENT_EQUAL        (HEMP_IDENT_FALSE | HEMP_COMPARE_BITS)     /* 110111 */

/* special identity values */
#define HEMP_INFINITY           0xFFF0000000000000ULL
#define HEMP_NAN                0xFFF8000000000000ULL
#define HEMP_IDENTITY           0xFFF8000000000001ULL

/* The type tag is 5 bits wide, offset 47 bits from the LSB */
#define HEMP_TAG_SHIFT          47
#define HEMP_TAG_MASK           0x1F
#define HEMP_TAG_UP(t)          ((hemp_u64_t) (t & HEMP_TAG_MASK) << HEMP_TAG_SHIFT)
#define HEMP_TAG_DOWN(v)        ((hemp_u8_t)  (v >> HEMP_TAG_SHIFT) & HEMP_TAG_MASK)
#define HEMP_TAG_MAKE(t)        (HEMP_INFINITY | HEMP_TAG_UP(t))
#define HEMP_TAG_TYPE(v)        HEMP_TAG_DOWN(v.bits)
#define HEMP_TAG_VALID(v)       ((hemp_bool_t) ((v.bits & HEMP_INFINITY) == HEMP_INFINITY))

/* internal macros to fetch/test the type identifier in a tagged value */
#define HEMP_TYPE_ID(v)         (HEMP_TAG_VALID(v) ? HEMP_TAG_TYPE(v) : HEMP_NUMBER_ID)
#define HEMP_TYPE_IS(v,t)       (HEMP_TYPE_ID(v) == t)
#define HEMP_TYPE_BELOW(v,t)    (HEMP_TYPE_ID(v) < t)

/* macros for manipulating identity values */
#define HEMP_IDENT_MASK         0xFF
#define HEMP_IDENT_MAKE(t)      ((hemp_value_t) (HEMP_IDENTITY | t))

/* TODO: this isn't right - it doesn't check that the lower 4 bits of the 
 * identity tag are 0.  It works for now because we don't have any type 
 * tags >= 0xFFF8000000000000
 */
#define HEMP_IDENT_VALID(v)     ((hemp_bool_t) ((v.bits & HEMP_IDENTITY) == HEMP_IDENTITY))
#define HEMP_IDENT_VALUE(v)     ((hemp_u8_t) (v.bits & HEMP_IDENT_MASK))
#define HEMP_IDENT_ID(v)        ((hemp_u8_t) HEMP_IDENT_VALID(v) ? HEMP_IDENT_VALUE(v) : 0)
#define HEMP_IDENT_IS(v,i)      ((hemp_bool_t) (v.bits == (HEMP_IDENTITY | i)))
#define HEMP_IDENT_MAX(v,b)     ((hemp_bool_t) (HEMP_IDENT_ID(v) ^ b))
#define HEMP_IDENT_ANY(v,b)     ((hemp_bool_t) (HEMP_IDENT_ID(v) & b))
#define HEMP_IDENT_ALL(v,b)     ((hemp_bool_t) (HEMP_IDENT_ANY(v,b) == b))
#define HEMP_IDENT_NOT(v,b)     ((hemp_bool_t) (HEMP_IDENT_VALID(v) && (HEMP_IDENT_ANY(v,b) == 0)))

/* Integers are represented in the lower 32 bits */
#define HEMP_INTEGER_BITS       32
#define HEMP_INTEGER_MASK       0xFFFFFFFFL

/* Pointers use the lower 32 bits on 32 bit machines, 47 bits on 64 bit */
#if HEMP_WORD_LENGTH == 32
    #define HEMP_POINTER_BITS   32
    #define HEMP_POINTER_MASK   0x00000000FFFFFFFFLL
    #define HEMP_POINTER(v)     ((hemp_mem_p)(v.bits >> HEMP_POINTER_BITS))
#elif HEMP_WORD_LENGTH == 64
    #define HEMP_POINTER_BITS   47
    #define HEMP_POINTER_MASK   0x00007FFFFFFFFFFFLL
    #define HEMP_POINTER(v)     ((hemp_mem_p)(v.bits & HEMP_POINTER_MASK))
#else
    #error "Invalid word length"
#endif

/* type tags */
#define HEMP_INTEGER_TAG        HEMP_TAG_MAKE(HEMP_INTEGER_ID)
#define HEMP_POINTER_TAG        HEMP_TAG_MAKE(HEMP_POINTER_ID)
#define HEMP_STRING_TAG         HEMP_TAG_MAKE(HEMP_STRING_ID)
#define HEMP_TEXT_TAG           HEMP_TAG_MAKE(HEMP_TEXT_ID)
#define HEMP_LIST_TAG           HEMP_TAG_MAKE(HEMP_LIST_ID)
#define HEMP_HASH_TAG           HEMP_TAG_MAKE(HEMP_HASH_ID)
#define HEMP_OBJECT_TAG         HEMP_TAG_MAKE(HEMP_OBJECT_ID)
#define HEMP_IDENTITY_TAG       HEMP_TAG_MAKE(HEMP_IDENTITY_ID)


/*--------------------------------------------------------------------------
 * public-facing macros for general use
 *--------------------------------------------------------------------------*/

//#define hemp_identity_id(v)     HEMP_IDENT_ID(c)
#define hemp_is_tagged(v)       HEMP_TAG_VALID(v)
#define hemp_is_number(v)       ((hemp_u64_t) v.bits < HEMP_INFINITY)
#define hemp_is_numeric(v)      HEMP_IDENT_MAX(v, HEMP_INTEGER_ID)
#define hemp_is_integer(v)      HEMP_TYPE_IS(v, HEMP_INTEGER_ID)
#define hemp_is_pointer(v)      HEMP_TYPE_IS(v, HEMP_POINTER_ID)
#define hemp_is_string(v)       HEMP_TYPE_IS(v, HEMP_STRING_ID)
#define hemp_is_text(v)         HEMP_TYPE_IS(v, HEMP_TEXT_ID)
#define hemp_is_list(v)         HEMP_TYPE_IS(v, HEMP_LIST_ID)
#define hemp_is_hash(v)         HEMP_TYPE_IS(v, HEMP_HASH_ID)
#define hemp_is_object(v)       HEMP_TYPE_IS(v, HEMP_OBJECT_ID)
#define hemp_is_identity(v)     HEMP_TYPE_IS(v, HEMP_IDENTITY_ID)
#define hemp_is_missing(v)      HEMP_IDENT_NOT(v, HEMP_FOUND_BIT)
//#define hemp_is_found(v)      HEMP_IDENT_ANY(v, HEMP_FOUND_BIT)
#define hemp_is_found(v)        (! hemp_is_missing(v))
#define hemp_is_nothing(v)      HEMP_IDENT_IS(v, HEMP_IDENT_NOTHING)
#define hemp_is_undefined(v)    HEMP_IDENT_NOT(v, HEMP_DEFINED_BIT)
#define hemp_is_defined(v)      (! hemp_is_undefined(v))        /* HEMP_IDENT_ANY(v, HEMP_DEFINED_BIT) */
#define hemp_is_undef(v)        HEMP_IDENT_NOT(v, HEMP_DEFINED_BIT)
#define hemp_is_boolean(v)      HEMP_IDENT_ANY(v, HEMP_FINITE_BIT)
#define hemp_is_true(v)         HEMP_IDENT_ANY(v, HEMP_TRUE_BIT)
#define hemp_is_false(v)        HEMP_IDENT_NOT(v, HEMP_TRUE_BIT)
#define hemp_is_compare(v)      HEMP_IDENT_ANY(v, HEMP_COMPARE_BITS)
#define hemp_is_before(v)       HEMP_IDENT_IS(v, HEMP_IDENT_BEFORE)
#define hemp_is_after(v)        HEMP_IDENT_IS(v, HEMP_IDENT_AFTER)
#define hemp_is_equal(v)        HEMP_IDENT_IS(v, HEMP_IDENT_EQUAL)


//#define hemp_type(v)            (hemp_global_types[HEMP_TYPE_ID(v)])
#define hemp_htype(v)           (hemp_global_types[HEMP_TYPE_ID(v)])
#define hemp_otype(v)           (((hemp_object_p) HEMP_POINTER(v))->type)
#define hemp_type(v)            (hemp_is_object(v) ? hemp_otype(v) : hemp_htype(v))
#define hemp_tfunc(v,n)         (hemp_type(v)->n)
#define hemp_call(v,n,c)        (hemp_tfunc(v,n)(v,c))
#define hemp_dot(v,c,k)         (hemp_tfunc(v,dot)(v,c,k))
#define hemp_text(v,c,o)        (hemp_tfunc(v,text)(v,c,o))
#define hemp_type_name(v)       (hemp_type(v)->name)
#define hemp_type_method(t,m)   ((hemp_value_f) hemp_hash_fetch_pointer(t->methods, m))
#define hemp_type_extend(t,m,f) hemp_hash_store_pointer(t->methods, m, f)
#define hemp_object_method(o,m) hemp_type_method(hemp_type(o), m)
#define hemp_send(o,m,c) ({                             \
    hemp_value_f method = hemp_object_method(o, m);     \
    method                                              \
        ? method(o,c)                                   \
        : HempMissing;                                  \
})


//#define hemp_ident_name(v)      (hemp_identity_name(HEMP_IDENT_ID(v)))
//#define hemp_type_class(v)      (hemp_type(v).name)

#define hemp_to_number(v,c)     (hemp_is_number(v)  ? v : hemp_call(v,number,c))
#define hemp_to_integer(v,c)    (hemp_is_integer(v) ? v : hemp_call(v,integer,c))
#define hemp_to_boolean(v,c)    (hemp_is_boolean(v) ? v : hemp_call(v,boolean,c))
#define hemp_to_compare(v,c)    (hemp_is_compare(v) ? v : hemp_call(v,compare,c))
#define hemp_to_text(v,c)       (hemp_is_text(v)    ? v : hemp_text(v,c,HempNothing))
#define hemp_onto_text(v,c,o)   hemp_text(v,c,o)


//#define hemp_is_text(v)         HEMP_TYPE_IS(v, HEMP_TYPE_TEXT_ID)

/*--------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------*/

typedef hemp_text_p     (* hemp_text_vfn)(hemp_value_t, hemp_context_p, hemp_text_p);
typedef hemp_value_t    (* hemp_dot_vfn)(hemp_value_t, hemp_context_p, hemp_str_p);
typedef void            (* hemp_init_vfn)(hemp_value_t);
typedef void            (* hemp_wipe_vfn)(hemp_value_t);



/*--------------------------------------------------------------------------
 * inline functions to encode native values as tagged values
 *--------------------------------------------------------------------------*/

extern HEMP_INLINE hemp_value_t     hemp_num_val(hemp_num_t n);
extern HEMP_INLINE hemp_value_t     hemp_int_val(hemp_int_t i);
extern HEMP_INLINE hemp_value_t     hemp_ptr_val(hemp_mem_p p);
extern HEMP_INLINE hemp_value_t     hemp_str_val(hemp_str_p s);
extern HEMP_INLINE hemp_value_t     hemp_text_val(hemp_text_p t);
extern HEMP_INLINE hemp_value_t     hemp_list_val(hemp_list_p l);
extern HEMP_INLINE hemp_value_t     hemp_hash_val(hemp_hash_p l);
extern HEMP_INLINE hemp_value_t     hemp_obj_val(hemp_object_p o);
extern HEMP_INLINE hemp_value_t     hemp_bool_val(hemp_bool_t b);
extern HEMP_INLINE hemp_value_t     hemp_ident_val(hemp_u8_t i);


/*--------------------------------------------------------------------------
 * inline functions to decode tagged values to native values
 *--------------------------------------------------------------------------*/

extern HEMP_INLINE hemp_num_t       hemp_val_num(hemp_value_t v);
extern HEMP_INLINE hemp_int_t       hemp_val_int(hemp_value_t v);
extern HEMP_INLINE hemp_mem_p       hemp_val_ptr(hemp_value_t v);
extern HEMP_INLINE hemp_str_p       hemp_val_str(hemp_value_t v);
extern HEMP_INLINE hemp_text_p      hemp_val_text(hemp_value_t v);
extern HEMP_INLINE hemp_list_p      hemp_val_list(hemp_value_t v);
extern HEMP_INLINE hemp_hash_p      hemp_val_hash(hemp_value_t v);
extern HEMP_INLINE hemp_object_p    hemp_val_obj(hemp_value_t v);
extern HEMP_INLINE hemp_bool_t      hemp_val_bool(hemp_value_t v);


/*--------------------------------------------------------------------------
 * function prototypes
 *--------------------------------------------------------------------------*/

/* generic value evalaution */
HEMP_VALUE_FUNC(hemp_value_no_op);
HEMP_VALUE_FUNC(hemp_value_self);
HEMP_VALUE_FUNC(hemp_value_true);
HEMP_VALUE_FUNC(hemp_value_false);
HEMP_FETCH_FUNC(hemp_value_dot);

/* default "cannot convert to X" function */
HEMP_VTEXT_FUNC(hemp_value_not_text);
HEMP_VALUE_FUNC(hemp_value_not_number);
HEMP_VALUE_FUNC(hemp_value_not_integer);
HEMP_VALUE_FUNC(hemp_value_not_boolean);
HEMP_VALUE_FUNC(hemp_value_not_compare);
HEMP_VALUE_FUNC(hemp_value_not_defined);
HEMP_FETCH_FUNC(hemp_value_not_fetch);
HEMP_STORE_FUNC(hemp_value_not_store);
HEMP_FETCH_FUNC(hemp_value_not_dot);

/* number -> xxx conversion */
HEMP_VTEXT_FUNC(hemp_value_number_text);
HEMP_VALUE_FUNC(hemp_value_number_integer);
HEMP_VALUE_FUNC(hemp_value_number_boolean);
HEMP_VALUE_FUNC(hemp_value_number_compare);

/* integer -> xxx conversion */
HEMP_VTEXT_FUNC(hemp_value_integer_text);
HEMP_VALUE_FUNC(hemp_value_integer_number);
HEMP_VALUE_FUNC(hemp_value_integer_boolean);
HEMP_VALUE_FUNC(hemp_value_integer_compare);

/* identity -> xxx conversions */
HEMP_VTEXT_FUNC(hemp_value_identity_text);
HEMP_VALUE_FUNC(hemp_value_identity_number);
HEMP_VALUE_FUNC(hemp_value_identity_integer);
HEMP_VALUE_FUNC(hemp_value_identity_defined);
HEMP_VALUE_FUNC(hemp_value_identity_boolean);
HEMP_VALUE_FUNC(hemp_value_identity_compare);

/* prototypes for text, list and hash are in the respective type/XXX.h files */

/* debugging functions */
void hemp_dump_u64(hemp_u64_t value);
void hemp_dump_64(hemp_u64_t value);
void hemp_dump_32(hemp_u32_t value);
void hemp_dump_value(hemp_value_t value);



#endif /* HEMP_VALUE_H */
