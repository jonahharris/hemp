#ifndef HEMP_TYPES_H
#define HEMP_TYPES_H

#define HEMP_TRUE  1
#define HEMP_FALSE 0


/*--------------------------------------------------------------------------
 * generic types
 *--------------------------------------------------------------------------*/

typedef unsigned char             hemp_bool_t;    /* boolean true/false       */
typedef unsigned char             hemp_char_t;    /* single character         */
typedef long                      hemp_flags_t;   /* bitmask                  */
typedef long                      hemp_offset_t;  /* -ve/+ve offset           */
typedef unsigned long             hemp_size_t;    /* +ve size of list         */
typedef unsigned long             hemp_pos_t;     /* +ve posn. in string/list */
typedef const void              * hemp_mem_t;     /* generic memory pointer   */
typedef void                    * hemp_ptr_t;     /* mutable memory pointer   */
typedef char                    * hemp_cstr_t;    /* C string                 */
typedef const char              * hemp_name_t;    /* immutable C string       */


/*--------------------------------------------------------------------------
 * hemp data structures
 *--------------------------------------------------------------------------*/

typedef struct hemp             * hemp_t;
typedef struct hemp_dialect     * hemp_dialect_t;
typedef struct hemp_etype       * hemp_etype_t;
typedef struct hemp_element     * hemp_element_t;
typedef struct hemp_elements    * hemp_elements_t;
typedef struct hemp_filesystem  * hemp_filesystem_t;
typedef struct hemp_hash        * hemp_hash_t;
typedef struct hemp_hash_entry  * hemp_hash_entry_t;
typedef struct hemp_list        * hemp_list_t;
typedef struct hemp_pool        * hemp_pool_t;
typedef struct hemp_pnode       * hemp_pnode_t;
typedef struct hemp_ptree       * hemp_ptree_t;
typedef struct hemp_scheme      * hemp_scheme_t;
typedef struct hemp_source      * hemp_source_t;
typedef struct hemp_slab        * hemp_slab_t;
typedef struct hemp_tag         * hemp_tag_t;
typedef struct hemp_tagset      * hemp_tagset_t;
typedef struct hemp_template    * hemp_template_t;
typedef struct hemp_type        * hemp_type_t;
typedef struct hemp_value       * hemp_value_t;


/*--------------------------------------------------------------------------
 * function pointers
 *--------------------------------------------------------------------------*/

typedef hemp_size_t  (* hemp_hash_fn)(hemp_cstr_t); 
typedef hemp_bool_t  (* hemp_scan_fn)(hemp_template_t);
typedef hemp_cstr_t  (* hemp_source_text_fn)(hemp_source_t);
typedef hemp_bool_t  (* hemp_source_bool_fn)(hemp_source_t);
typedef hemp_bool_t  (* hemp_build_fn)(hemp_t);
typedef hemp_bool_t  (* hemp_onload_fn)(hemp_t);
typedef hemp_bool_t  (* hemp_list_each_fn)(hemp_list_t, hemp_pos_t, hemp_ptr_t);
typedef void         (* hemp_tag_scan_fn)(hemp_template_t, hemp_tag_t, hemp_cstr_t, hemp_pos_t, hemp_cstr_t *);

typedef hemp_value_t (* hemp_init_fn)();
typedef void         (* hemp_free_fn)(hemp_value_t);
typedef hemp_bool_t  (* hemp_truth_fn)(hemp_value_t);
typedef hemp_value_t (* hemp_text_fn)(hemp_value_t);     // TODO: return more specific text type
typedef hemp_value_t (* hemp_unary_fn)(hemp_value_t);
typedef hemp_value_t (* hemp_binary_fn)(hemp_value_t, hemp_value_t);
typedef hemp_value_t (* hemp_ternary_fn)(hemp_value_t, hemp_value_t, hemp_value_t);



#endif /* HEMP_TYPES_H */