#include <hemp/element.h>


/*--------------------------------------------------------------------------
 * initialisation and cleanup functions
 *--------------------------------------------------------------------------*/

hemp_element
hemp_element_init(
    hemp_element    element,
    hemp_symbol     type, 
    hemp_string     token, 
    hemp_pos        position, 
    hemp_size       length
) {
    HEMP_INSTANCE(element);
        
    element->elements = NULL;
    element->next     = NULL;
    element->type     = type;
    element->token    = token;
    element->position = position;
    element->length   = length;
    element->flags    = type->flags;
// TODO: should we zero this memory?
//  element->value    = NULL;

    return element;
}


void
hemp_element_free(
    hemp_element    element
) {
    hemp_mem_free(element);
}


HEMP_INLINE hemp_grammar
hemp_element_grammar(
    hemp_element    element
) {
    if (! element->type->grammar)
        hemp_fatal(
            "No grammar defined for %s element", 
            element->type->name
        );

    return element->type->grammar;
}


HEMP_INLINE hemp_elements
hemp_element_elements(
    hemp_element    element
) {
    if (! element->elements)
        hemp_fatal(
            "No elements defined for %s element", 
            element->type->name
    );

    return element->elements;
}


HEMP_INLINE hemp_symbol
hemp_element_grammar_symbol(
    hemp_element    element,
    hemp_string     name
) {
    return hemp_grammar_symbol(
        hemp_element_grammar(element),
        name
    );
}


HEMP_INLINE hemp_element
hemp_element_create(
    hemp_element    element,
    hemp_string     typename
) {
    return hemp_elements_append(
        hemp_element_elements(element),
        hemp_element_grammar_symbol(element, typename),
        element->token, element->position, element->length
    );
}


hemp_symbol 
hemp_element_retype(
    hemp_element    element,
    hemp_string     typename
) {
    hemp_symbol  type    = element->type;
    hemp_grammar grammar = type->grammar;

    if (grammar) {
        hemp_debug("found element grammar\n");
    }
    else {
        hemp_fatal("No grammar defined for element type: %s", type->name);
    }

    type = hemp_grammar_symbol(grammar, typename);

    if (! type)
        hemp_fatal(
            "Invalid element type specified (symbol not found in %s grammar): %s",
            grammar->name, typename
        );

    element->type = type;

    return type;
}


/*--------------------------------------------------------------------------
 * front-end parsing pseudo-method
 *--------------------------------------------------------------------------*/

hemp_element
hemp_element_parse(
    hemp_element    element,
    hemp_scope      scope
) {
    hemp_debug_call("hemp_element_parse()\n");
    hemp_element *current = &element;

    hemp_element block = hemp_element_parse_block(
        current,
        scope,
        0, 
        HEMP_FALSE
    );

    hemp_element next_elem = *current;
    
    if (next_elem->type != HempSymbolEOF)
        hemp_fatal("Unexpected token: %s\n", next_elem->type->name);

    // hemp_debug_msg("next element after parse is %s\n", next_elem->type->name);

    return block;
}


/*--------------------------------------------------------------------------
 * generic element parsing functions
 *--------------------------------------------------------------------------*/

HEMP_PREFIX_FUNC(hemp_element_parse_block) {
    hemp_debug_call("hemp_element_parse_block()\n");
    hemp_element element = *elemptr;
    hemp_list list       = hemp_element_parse_exprs(HEMP_PREFIX_ARG_NAMES);
    hemp_element block   = NULL;


    if (list) {
        block = hemp_elements_append(
            hemp_element_elements(element),
            HempSymbolBlock,
            element->token, element->position, element->length
        );
//      hemp_debug_msg("new element block at %p with list at %p and cleaner is %p\n", block, list, list->cleaner);

        // hemp_debug("got list of %d exprs\n", list->length);
//        block = hemp_element_init(
//            NULL,
//            HempSymbolBlock, 
//            element->token,  
//            element->position,
//            element->length
//        );

        hemp_set_block_exprs_list(block, list);
    }

    return block;
}


hemp_list
hemp_element_parse_exprs(
    HEMP_PREFIX_ARGS
) {
    hemp_debug_parse("hemp_element_parse_exprs( precedence => %d )\n", precedence);

    hemp_element expr;
    hemp_list    exprs = hemp_list_new();

#if HEMP_DEBUG & HEMP_DEBUG_PARSE
    hemp_debug("\n-- EXPRS --\n");
#endif

    while (1) {
        /* skip whitespace, delimiters (commas) and separators (semi-colons) */
        hemp_skip_separator(elemptr);

        /* ask the next token to return an expression */
        expr = hemp_parse_prefix(elemptr, scope, precedence, HEMP_FALSE);

        /* if it's not an expression (e.g. a terminator) then we're done */
        if (! expr)
            break;

#if HEMP_DEBUG & HEMP_DEBUG_PARSE
        hemp_debug_parse("parsed %s expression:\n", expr->type->name);
        hemp_element_dump(expr);
#endif
        hemp_list_push(exprs, hemp_elem_val(expr));
    }

    /* element should be EOF or we hit a duff token */
    if (hemp_at_eof(elemptr)) {
        hemp_debug_parse("%sReached EOF\n%s\n", HEMP_ANSI_GREEN, HEMP_ANSI_RESET);
    }
    else {
        hemp_debug_parse("%sNot an expression: %s:%s\n", HEMP_ANSI_RED, (*elemptr)->type->name, HEMP_ANSI_RESET);
    }

    // hemp_debug("n expressions: %d\n", exprs->length);
    
    if (! exprs->length && ! force) {
        // hemp_debug("clearing empty list\n");
        hemp_list_free(exprs);
        exprs = NULL;
    }

#if HEMP_DEBUG & HEMP_DEBUG_PARSE
    hemp_debug("-- /EXPRS --\n");
#endif
    
    return exprs;
}


HEMP_PREFIX_FUNC(hemp_element_fixed) {
    hemp_debug_call("hemp_element_fixed()\n");
    hemp_element element = *elemptr;
    hemp_string     string  = hemp_string_extract(
        element->token,
        element->token + element->length
    );

//    hemp_debug("extracted word token string: %s\n", string);
    hemp_set_flag(element, HEMP_BE_FIXED | HEMP_BE_ALLOCATED);
    hemp_set_expr(element, hemp_str_val(string));
    hemp_go_next(elemptr);
    return element;
}


HEMP_PREFIX_FUNC(hemp_element_parse_body) {
    hemp_debug_call("hemp_element_parse_body(%s)\n", (*elemptr)->type->name);

    /* we don't need to skip whitespace here as any skippable whitespace 
     * elements will forward the parse_prefix call onto the next token, but
     * it's slightly more efficient this way.
     */
    hemp_skip_whitespace(elemptr);

    /* See comments for parse_body() in Template::TT3::Element.pm
     * It forces precedence to CMD_PRECEDENCE and FORCE to 1.  I don't 
     * think we need to do this if we just use the values passed as args.
     */
    hemp_element element = hemp_parse_prefix(
        elemptr, scope, precedence, force
    );

    /* A single expression body is fully terminated, so the caller should
     * not look for a corresponding terminator token, e.g. sub inc(a) a + 1
     * has a single expression body, a + 1, so it doesn't require an 'end',
     * unlike: sub inc(a); a + 1; end, which invokes parse_body on the ';'
     * terminator, returning an unterminated block
     */
    if (element)
        hemp_set_flag(element, HEMP_BE_TERMINATED);

    return element;
}


HEMP_PREFIX_FUNC(hemp_element_parse_body_block) {
    hemp_debug_call("hemp_element_parse_body_block(%s)\n", (*elemptr)->type->name);
    return hemp_element_parse_block(elemptr, scope, 0, 1);
}


/*--------------------------------------------------------------------------
 * decline functions
 *--------------------------------------------------------------------------*/

HEMP_PREFIX_FUNC(hemp_element_decline) {
    return NULL;
}


HEMP_PREFIX_FUNC(hemp_element_not_prefix) {
//  hemp_debug("%s is not prefix\n", (*elemptr)->type->name);
    hemp_debug_call("hemp_element_not_prefix()\n");
    return NULL;
}


HEMP_POSTFIX_FUNC(hemp_element_not_postfix) {
    hemp_debug("%s is not postfix\n", (*elemptr)->type->name);
    hemp_debug_call("hemp_element_not_postfix()\n");
    return lhs;
}


HEMP_FIXUP_FUNC(hemp_element_not_lvalue) {
    hemp_fatal(
        "%s element cannot be assigned to\n", 
        element->type->name
    );
    return NULL;
}

HEMP_FIXUP_FUNC(hemp_element_not_proto) {
    hemp_fatal(
        "%s element cannot be a function prototype\n", 
        element->type->name
    );
    return NULL;
}


HEMP_PREFIX_FUNC(hemp_element_not_word) {
    return NULL;
}


HEMP_OUTPUT_FUNC(hemp_element_not_token) {
    hemp_fatal(
        "%s element does not yield token\n", 
        hemp_val_elem(value)->type->name
    );
    return output;
}


HEMP_OUTPUT_FUNC(hemp_element_not_source) {
    hemp_fatal(
        "%s element does not yield source\n", 
        hemp_val_elem(value)->type->name);
    return output;
}


HEMP_OUTPUT_FUNC(hemp_element_not_text) {
    hemp_fatal(
        "%s element does not yield text\n", 
        hemp_val_elem(value)->type->name);
    return output;
}


HEMP_OUTPUT_FUNC(hemp_element_not_values) {
    hemp_fatal(
        "%s element does not yield values\n", 
        hemp_val_elem(value)->type->name
    );
    return HempNothing;
}


HEMP_OUTPUT_FUNC(hemp_element_not_params) {
    hemp_fatal(
        "%s element does not yield params\n", 
        hemp_val_elem(value)->type->name
    );
    return HempNothing;
}


HEMP_VALUE_FUNC(hemp_element_not_value) {
    hemp_fatal(
        "%s element does not yield value\n", 
        hemp_val_elem(value)->type->name
    );
    return HempNothing;
}


HEMP_VALUE_FUNC(hemp_element_not_number) {
    hemp_fatal(
        "%s element does not yield number\n", 
        hemp_val_elem(value)->type->name
    );
    return HempNothing;
}


HEMP_VALUE_FUNC(hemp_element_not_integer) {
    hemp_fatal(
        "%s element does not yield integer\n", 
        hemp_val_elem(value)->type->name
    );
    return HempNothing;
}


HEMP_VALUE_FUNC(hemp_element_not_boolean) {
    hemp_fatal(
        "%s element does not yield boolean\n", 
        hemp_val_elem(value)->type->name
    );
    return HempNothing;
}


HEMP_VALUE_FUNC(hemp_element_not_compare) {
    hemp_fatal(
        "%s element does not yield comparison\n", 
        hemp_val_elem(value)->type->name
    );
    return HempNothing;
}


HEMP_INPUT_FUNC(hemp_element_not_assign) {
    hemp_debug_msg("Throwing not_assign error....\n");
    hemp_fatal(
        "%s element cannot be assigned to\n", 
        hemp_val_elem(value)->type->name
    );
    return HempNothing;
}




/*--------------------------------------------------------------------------
 * delegation functions, forwarding the request to the next element
 *--------------------------------------------------------------------------*/

HEMP_PREFIX_FUNC(hemp_element_next_prefix) {
    hemp_debug_call("hemp_element_next_prefix()\n");

    if (hemp_has_next(elemptr)) {
        hemp_go_next(elemptr);
        return hemp_parse_prefix(elemptr, scope, precedence, force);
    }

    return NULL;
}


/* this has been replaced by hemp_element_space_postfix */
HEMP_POSTFIX_FUNC(hemp_element_next_postfix) {
    hemp_debug_call("hemp_element_next_postfix()\n");

    /* TODO: add check for HEMP_BE_XXXX flag to indicate if spaces are allowed
     * or not before postfix operator.  For now, we allow them.  We currently
     * use "infix" to distinguish "spacey" operators from "pure" postfix 
     * operators that bind tight.  But none of this is currently enforced,
     * and won't be until I've had a chance to rethink the terminology and 
     * overall strategy.
     */
    if (hemp_has_next(elemptr)) {
        hemp_go_next(elemptr);
        return hemp_parse_postfix(elemptr, scope, precedence, force, lhs);
    }

    return lhs;
}


HEMP_PREFIX_FUNC(hemp_element_next_body) {
    hemp_debug_call("hemp_element_next_body()\n");

    if (hemp_has_next(elemptr)) {
        hemp_go_next(elemptr);
        return hemp_parse_body(elemptr, scope, precedence, force);
    }

    return NULL;
}



/*--------------------------------------------------------------------------
 * expression parsing methods
 *--------------------------------------------------------------------------*/

HEMP_PREFIX_FUNC(hemp_element_parse_prefix) {
    hemp_element self = *elemptr;
    hemp_symbol  type = self->type;
    hemp_element expr;

    hemp_debug_call("hemp_element_parse_prefix()\n");

    hemp_set_flag(self, HEMP_BE_PREFIX);
    hemp_go_next(elemptr);

    expr = hemp_parse_prefix(elemptr, scope, type->rprec, 1);

    if (! expr)
        hemp_fatal("missing expression on rhs of %s\n", type->start);
    
    hemp_set_expr_element(self, expr);
//  hemp_skip_whitespace(elemptr);

    return hemp_parse_postfix(
        elemptr, scope, precedence, 0,
        self
    );
}


HEMP_POSTFIX_FUNC(hemp_element_parse_postfix) {
    hemp_element self = *elemptr;
    hemp_symbol  type = self->type;

    hemp_debug_call("hemp_element_parse_postfix()\n");

    HEMP_INFIX_LEFT_PRECEDENCE;              // is that right?
    hemp_set_flag(self, HEMP_BE_POSTFIX);

    hemp_set_expr_element(self, lhs);
    hemp_go_next(elemptr);
//  hemp_skip_whitespace(elemptr);

    return hemp_parse_postfix(
        elemptr, scope, precedence, 0,
        self
    );
}


HEMP_POSTFIX_FUNC(hemp_element_parse_infix_left) {
    hemp_element self = *elemptr;
    hemp_symbol  type = self->type;
    hemp_element rhs;

    hemp_debug_call("hemp_element_parse_infix_left()\n");

    HEMP_INFIX_LEFT_PRECEDENCE;
    hemp_set_flag(self, HEMP_BE_INFIX);
    hemp_set_lhs_element(self, lhs);
    hemp_go_next(elemptr);
    rhs = hemp_parse_prefix(elemptr, scope, type->lprec, 1);

    if (! rhs)
        hemp_fatal("missing expression on rhs of %s\n", type->start);

    hemp_set_rhs_element(self, rhs);
//  hemp_skip_whitespace(elemptr);

    hemp_debug_parse(
        "parsed infix [%s] [%s] [%s]\n", 
        lhs->type->name, self->type->start, rhs->type->name
    );

    hemp_debug_parse("next element is %s:\n", (*elemptr)->type->name);

    return hemp_parse_postfix(
        elemptr, scope, precedence, 0,
        self
    );
}


HEMP_POSTFIX_FUNC(hemp_element_parse_infix_right) {
    hemp_element self = *elemptr;
    hemp_symbol  type = self->type;
    hemp_element rhs;

    hemp_debug_call("hemp_element_parse_infix_right()\n");

    HEMP_INFIX_RIGHT_PRECEDENCE;
    hemp_set_flag(self, HEMP_BE_INFIX);

    hemp_set_lhs_element(self, lhs);
    hemp_go_next(elemptr);
    rhs = hemp_parse_prefix(elemptr, scope, type->lprec, 1);

    if (! rhs)
        hemp_fatal("missing expression on rhs of %s\n", type->start);
    
    hemp_set_rhs_element(self, rhs);
//  hemp_skip_whitespace(elemptr);

    return hemp_parse_postfix(
        elemptr, scope, precedence, 0,
        self
    );
}


/*--------------------------------------------------------------------------
 * evaluation and output functions
 *
 * Elements can provide custom functions for evaluation as a text fragment,
 * number, integer, boolean truth, or tri-state comparison (before, equal,
 * after) value.  In the usual case they "inherit" the default "methods" 
 * below that call the element's generic value() method and then coerce the 
 * returned value to the appropriate type.  That way an element is only 
 * required to implement a single value() method and everything else Just
 * Works[tm].
 *
 * NOTE: don't be tempted to write something like this:
 *   return hemp_to_XXX( element->type->value(element, context) );
 *
 * hemp_to_XXX are macros and the above will cause the element to be 
 * evaluated multiple times.
 *--------------------------------------------------------------------------*/

HEMP_VALUE_FUNC(hemp_element_value) {
    hemp_debug_call("hemp_element_value()\n");
    return hemp_obcall(value, value, context);
}


HEMP_OUTPUT_FUNC(hemp_element_value_text) {
    hemp_debug_call("hemp_element_value_text()\n");
    hemp_value result = hemp_obcall(value, value, context);
    return hemp_onto_text(result, context, output);
}


HEMP_VALUE_FUNC(hemp_element_value_number) {
    hemp_debug_call("hemp_element_value_number()\n");
    hemp_value result = hemp_obcall(value, value, context);
    return hemp_is_numeric(result)
        ? result
        : hemp_call(result, number, context);

// forced coersion is not the way forward
//    return hemp_to_num(v);
}


HEMP_VALUE_FUNC(hemp_element_value_integer) {
    hemp_debug_call("hemp_element_value_integer()\n");
    hemp_value result = hemp_obcall(value, value, context);
    return hemp_to_integer(result, context);
}


HEMP_VALUE_FUNC(hemp_element_value_boolean) {
    hemp_debug_call("hemp_element_value_boolean()\n");
    hemp_value result = hemp_obcall(value, value, context);
    return hemp_to_boolean(result, context);
}


HEMP_VALUE_FUNC(hemp_element_value_compare) {
    hemp_debug_call("hemp_element_value_compare()\n");
    hemp_value result = hemp_obcall(value, value, context);
    return hemp_to_compare(result, context);
}


HEMP_OUTPUT_FUNC(hemp_element_value_values) {
    hemp_debug_call("hemp_element_value_values()\n");
    hemp_value result = hemp_obcall(value, value, context);
    return hemp_values(result, context, output);
}




/*--------------------------------------------------------------------------
 * debugging functions
 *--------------------------------------------------------------------------*/

hemp_bool
hemp_element_dump(
    hemp_element e
) {
    hemp_context context = hemp_context_init(NULL);       // tmp ugly hack

    if (! e->type->text)
        hemp_fatal("%s type does not define a text() method", e->type->name);

    hemp_value output = e->type->token
        ? e->type->token(hemp_elem_val(e), context, HempNothing)
        : e->type->text(hemp_elem_val(e), context, HempNothing);


    hemp_text text  = hemp_val_text(output);
    hemp_string string = text ? text->string : "-- NO OUTPUT --";
    
    hemp_debug(
        "%p %03d:%02d %-20s %s[%s%s%s]%s\n", e,
        (int) e->position, (int) e->length, e->type->name, 
        HEMP_ANSI_BLUE, HEMP_ANSI_YELLOW, string, HEMP_ANSI_BLUE, HEMP_ANSI_RESET
    );

    if (text) {
//        hemp_text_free(text);
    }

    hemp_context_free(context);

    return hemp_string_eq(e->type->name, "EOF")
        ? HEMP_FALSE
        : HEMP_TRUE;
}



