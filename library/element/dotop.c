#include <math.h>
#include <hemp/element.h>
#include <hemp/scope.h>


HEMP_SYMBOL_FUNC(hemp_element_dotop_symbol) {
    hemp_element_infix_left_symbol(hemp, symbol);
    symbol->infix   = &hemp_element_dotop_infix;
    symbol->value   = &hemp_element_dotop_value;
    symbol->cleanup = &hemp_element_dotop_clean;

    return symbol;
}


HEMP_POSTFIX_FUNC(hemp_element_dotop_infix) {
    hemp_element_p self = *elemptr;
    hemp_symbol_p  type = self->type;

    hemp_debug_call("hemp_element_dotop_parse_infix()\n");

    HEMP_INFIX_LEFT_PRECEDENCE;
    hemp_set_flag(self, HEMP_BE_INFIX);

    hemp_set_lhs_element(self, lhs);
    hemp_go_next(elemptr);
    hemp_skip_whitespace(elemptr);

    // FIXME: temporary hack to get something working
    hemp_element_p rhs = hemp_parse_fixed(elemptr, scope, type->lprec, 1);

    if (! rhs)
        hemp_fatal("missing expression on rhs of %s\n", type->start);

    hemp_set_flag(self, HEMP_BE_FIXED);
    
    // make rhs yield static token name
    hemp_set_rhs(
        self,
        /* fuckola!  we need to call token() on words, but value() on numbers */
        /* TODO: make the word be a plain word and only upgrade to variable
         * when used in prefix/infix position.  If used as fixed then it 
         * returns the token text (which is always a text copy...)
         */
        rhs->type->value(
            rhs, scope->context
        )
    );

    hemp_skip_whitespace(elemptr);
    
    return hemp_parse_infix(
        elemptr, scope, precedence, 0,
        self
    );
}


HEMP_EVAL_FUNC(hemp_element_dotop_value) {
    hemp_debug_call("hemp_element_dotop_value()\n");

    /* Note the temporary hack: rhs is always a pre-evaluated value */
    hemp_element_p  lhs  = hemp_lhs_element(element);
    hemp_value_t    lval = lhs->type->value(lhs, context);
    hemp_value_t    rval = hemp_rhs(element);

    if (hemp_is_undefined(lval)) {
        hemp_fatal("TODO: cannot call dotop on undefined value");
    }
    
    if (hemp_is_undefined(rval)) {
        hemp_fatal("TODO: cannot call dotop with undefined RHS value");
    }

    return hemp_dot(lval, context, rval);
}


void
hemp_element_dotop_clean(
    hemp_element_p element
) {
    hemp_debug_call("hemp_element_dotop_clean(%p)\n", element);

    if (hemp_has_flag(element, HEMP_BE_FIXED)) {
//      hemp_debug("cleaning static dotop element...");
        // TODO: more comprehensive check, probably via type->cleanup

        hemp_value_t rhs = hemp_rhs(element);
        if (hemp_is_text(rhs)) {
//          hemp_debug("rhs is text, freeing\n");
            hemp_text_free( hemp_val_text(rhs) );
        }
        else {
//          hemp_debug("not freeing %s RHS\n", hemp_type_name(rhs));
        }
    }
}

