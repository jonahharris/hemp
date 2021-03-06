#include <hemp/parser.h>


HempList
hemp_parse_exprs(
    HEMP_PREFIX_ARGS
) {
    hemp_debug_call("hemp_parse_exprs( precedence => %d )\n", precedence);

    HempFragment   expr;
    HempList       exprs = hemp_list_new();
//  hemp_debug_msg("hemp_parse_exprs() LIST: %p\n", exprs);

    while (1) {
        /* skip whitespace, delimiters (commas) and separators (semi-colons) */
        hemp_skip_separator(fragptr);

        /* ask the next token to return an expression */
        hemp_debug_parse("%s parse_prefix: %p\n", (*fragptr)->type->name, (*fragptr)->type->parse_prefix);
        expr = hemp_parse_prefix(fragptr, scope, precedence, HEMP_FALSE);

        /* if it's not an expression (e.g. a terminator) then we're done */
        if (! expr)
            break;

        hemp_debug_parse("expr: %s\n", expr->type->name);
        hemp_list_push(exprs, hemp_frag_val(expr));
    }

    /* element should be EOF or we hit a duff token */
    if (hemp_at_eof(fragptr)) {
        hemp_debug_parse("%sReached EOF\n%s\n", HEMP_ANSI_GREEN, HEMP_ANSI_RESET);
    }
    else {
        hemp_debug_parse("%sNot an expression: %s:%s\n", HEMP_ANSI_RED, (*fragptr)->type->name, HEMP_ANSI_RESET);
    }

    // hemp_debug("n expressions: %d\n", exprs->length);
    
    if (! exprs->length && ! force) {
        hemp_list_free(exprs);
        exprs = NULL;
    }

#if HEMP_DEBUG & HEMP_DEBUG_PARSE
    hemp_fragment_dump_exprs(exprs);
#endif
    
    return exprs;
}


HEMP_PREFIX(hemp_parse_block) {
    hemp_debug_call("hemp_parse_block()\n");
    HempFragment   fragment    = *fragptr;
    HempList       list        = hemp_parse_exprs(HEMP_PREFIX_ARG_NAMES);
    HempFragment   block       = NULL;

    if (list) {
        block = hemp_fragments_add_fragment(
            hemp_fragment_fragments(fragment),
            HempElementBlock,
            fragment->token, fragment->position, 0
        );
        hemp_set_block_exprs_list(block, list);
    }

    return block;
}


HempList
hemp_parse_pairs(
    HEMP_PREFIX_ARGS
) {
    hemp_debug_call("hemp_parse_pairs()\n");

    HempFragment   expr;
    HempList       exprs = hemp_list_new();

    while (1) {
        /* skip whitespace and delimiters (commas) */
        hemp_skip_delimiter(fragptr);

//      hemp_debug_msg("%s parse_prefix (prec: %d): %p\n", save->type->name, precedence, save->type->parse_pair);
        expr = hemp_parse_pair(fragptr, scope, precedence, HEMP_FALSE);

        /* if it's not a pairable expression then we're done */
        if (! expr) {
//          hemp_debug_msg("expression does not yield pairs: %s (%p vs %p)\n", save->type->name, save, *fragptr);
            break;
        }
            
//      hemp_debug_msg("expression yields pairs: %s\n", expr->type->name);
        hemp_list_push(exprs, hemp_frag_val(expr));
    }

    if (! exprs->length && ! force) {
        hemp_list_free(exprs);
        exprs = NULL;
    }

    return exprs;
}

