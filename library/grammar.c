#include <hemp/grammar.h>


hemp_grammar_p
hemp_grammar_init(
    hemp_p      hemp,
    hemp_cstr_p name
) {
    hemp_grammar_p grammar = (hemp_grammar_p) hemp_mem_alloc(
        sizeof(struct hemp_grammar_s)
    );

    if (! grammar)
        hemp_mem_fail("grammar");

    grammar->hemp      = hemp;
    grammar->symbols   = hemp_hash_init();
//  grammar->keywords  = hemp_hash_init();
    grammar->operators = hemp_ptree_init(HEMP_OPERATORS_SIZE);
    grammar->name      = hemp_cstr_copy(name);

    if (! grammar->name)
        hemp_mem_fail("grammar name");

    return grammar;
}

hemp_symbol_p
hemp_grammar_new_symbol(
    hemp_grammar_p grammar,
    hemp_cstr_p    etype,
    hemp_cstr_p    start,
    hemp_cstr_p    end
) {
//  debug_call(
//      "new [%s => %s] symbol\n", 
//      token, etype
//  );

    hemp_action_p constructor = hemp_factory_constructor(
        grammar->hemp->elements, etype
    );
    if (! constructor)
        hemp_throw(grammar->hemp, HEMP_ERROR_INVALID, "element", etype);

    hemp_symbol_p symbol = hemp_symbol_init(
        etype, start, end
    );

    symbol = (hemp_symbol_p) hemp_action_run(
        constructor, symbol 
    );

    if (! symbol)
        hemp_throw(grammar->hemp, HEMP_ERROR_INVALID, "element", etype);

    return symbol;
}


hemp_symbol_p
hemp_grammar_add_symbol(
    hemp_grammar_p grammar,
    hemp_cstr_p    etype,
    hemp_cstr_p    start,
    hemp_cstr_p    end,
    hemp_prec_t    lprec,
    hemp_prec_t    rprec
) {
//  debug_call(
//      "adding [%s => %s] symbol to %s grammar [%d|%d]\n", 
//      token, etype, grammar->name, lprec, rprec
//  );

    if (hemp_hash_fetch(grammar->symbols, start))
        hemp_throw(grammar->hemp, HEMP_ERROR_DUPLICATE, "symbol", start);

    hemp_symbol_p symbol = hemp_grammar_new_symbol(grammar, etype, start, end);

    symbol->lprec = lprec;
    symbol->rprec = rprec;

    /* all symbols get put in the hash table mapping token to symbol */
    hemp_hash_store(grammar->symbols, start, symbol);

    /* non-alphanumeric (starting) symbols go in the operator ptree which
     * allows us to easily match longest tokens so that '++' is interpreted
     * as one single token, not two instances of '+'
     */

    if (! isalpha(*start)) {
//      debug("symbol token is not alphanumeric: %s\n", token);
        hemp_ptree_store(
            grammar->operators, start, (hemp_mem_p) symbol
        );
    }

    return symbol;
}


void
hemp_grammar_free(
    hemp_grammar_p grammar
) {
    hemp_hash_each(grammar->symbols, &hemp_grammar_free_symbol);
    hemp_hash_free(grammar->symbols);
    hemp_ptree_free(grammar->operators);
    hemp_mem_free(grammar->name);
    hemp_mem_free(grammar);
}


hemp_bool_t
hemp_grammar_free_symbol(
    hemp_hash_p         grammars,
    hemp_pos_t          position,
    hemp_hash_item_p    item
) {
    hemp_symbol_free( (hemp_symbol_p) item->value );
    return HEMP_TRUE;
}
