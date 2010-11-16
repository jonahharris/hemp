#ifndef HEMP_GRAMMAR_H
#define HEMP_GRAMMAR_H

#include <hemp/core.h>
#include <hemp/ptree.h>
#include <hemp/element.h>
#include <hemp/action.h>
#include <hemp/factory.h>
#include <hemp/symbol.h>
#include <hemp/type/hash.h>


/*
    commonly used elements:
        - text, space, comment, padding
        - tag_start, tag_end
        - number, integer
        - word, squote, dquote
*/
/*--------------------------------------------------------------------------
 * data structures
 *--------------------------------------------------------------------------*/

struct hemp_grammar_s {
    hemp_p          hemp;
    hemp_str_p      name;
    hemp_hash_p     symbols;
    hemp_hash_p     keywords;
    hemp_ptree_p    operators;
};


hemp_grammar_p
    hemp_grammar_init(
        hemp_p          hemp,
        hemp_str_p      name
    );

hemp_symbol_p
    hemp_grammar_new_symbol(
        hemp_grammar_p  grammar,
        hemp_str_p      element,
        hemp_str_p      start,
        hemp_str_p      end
    );

hemp_symbol_p
    hemp_grammar_add_symbol(
        hemp_grammar_p  grammar,
        hemp_str_p      element,
        hemp_str_p      start,
        hemp_str_p      end,
        hemp_prec_t     lprec,
        hemp_prec_t     rprec
    );

//hemp_symbol_p
//    hemp_grammar_symbol(
//        hemp_grammar_p  grammar,
//        hemp_str_p      
//

void
    hemp_grammar_free(
        hemp_grammar_p  grammar
    );

hemp_bool_t
    hemp_grammar_free_symbol(
        hemp_hash_p     grammars,
        hemp_pos_t      position,
        hemp_slot_p     item
    );


#endif /* HEMP_GRAMMAR_H */