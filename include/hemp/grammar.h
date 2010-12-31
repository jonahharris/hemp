#ifndef HEMP_GRAMMAR_H
#define HEMP_GRAMMAR_H

#include <hemp/core.h>
#include <hemp/action.h>
#include <hemp/element.h>
#include <hemp/factory.h>
#include <hemp/ptree.h>
#include <hemp/namespace.h>
#include <hemp/document.h>
#include <hemp/type/hash.h>


/*--------------------------------------------------------------------------
 * type definitions
 *--------------------------------------------------------------------------*/

typedef hemp_grammar 
(*  hemp_grammar_f) (
    hemp_hemp       hemp,
    hemp_string     name
);

struct hemp_grammar {
    hemp_hemp       hemp;
    hemp_string     name;
    hemp_hash       elements;
    hemp_ptree      operators;
//  hemp_hash       keywords;
};

struct hemp_grammars {
    hemp_string     name;
    hemp_grammar_f  ctor;
};



/*--------------------------------------------------------------------------
 * Factory prototypes
 *--------------------------------------------------------------------------*/

HEMP_FACTORY(hemp_grammar_factory);
HEMP_HASH_ITERATOR(hemp_grammar_cleaner);


/*--------------------------------------------------------------------------
 * function prototypes
 *--------------------------------------------------------------------------*/

hemp_grammar
hemp_grammar_new(
    hemp_hemp       hemp,
    hemp_string     name
);

hemp_element
hemp_grammar_new_element(
    hemp_grammar    grammar,
    hemp_string     element,
    hemp_string     start,
    hemp_string     end
);

hemp_element
hemp_grammar_add_element(
    hemp_grammar    grammar,
    hemp_string     element,
    hemp_string     start,
    hemp_string     end,
    hemp_oprec      lprec,
    hemp_oprec      rprec
);

HEMP_INLINE hemp_element
hemp_grammar_element(
    hemp_grammar    grammar,
    hemp_string     name
);

void
hemp_grammar_free(
    hemp_grammar    grammar
);

hemp_bool
hemp_grammar_free_element(
    hemp_hash       grammars,
    hemp_pos        position,
    hemp_slot       item
);

hemp_bool
hemp_grammar_scanner(
    hemp_grammar    grammar,
    hemp_document   document
);



#endif /* HEMP_GRAMMAR_H */