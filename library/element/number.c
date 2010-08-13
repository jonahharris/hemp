#include <math.h>
#include <hemp/element.h>

/* TODO: check for overflow/underflow/divide-by-zero/NaN/Infinity */

#define HEMP_NUMOP_GET_INT(e,c)             \
    hemp_val_int(                           \
        e->type->integer(e, c)              \
    )

#define HEMP_NUMOP_CAST_NUM(v)              \
    (hemp_is_num(v)                         \
        ? hemp_val_num(v)                   \
        : ((hemp_num_t) hemp_val_int(v))    \
    )


/*--------------------------------------------------------------------------
 * static elements to represent literal numbers and integers
 *--------------------------------------------------------------------------*/

static struct hemp_symbol_s
    hemp_symbol_number = { 
        "number",                                   /* name                 */
        NULL,                                       /* start token          */
        NULL,                                       /* end token            */
        HEMP_BE_SOURCE       |                       /* flags                */
        HEMP_BE_STATIC,
        0, 0,                                       /* l/r precedence       */
        NULL,                                       /* scanner callback     */
        NULL,                                       /* cleanup callback     */
        &hemp_element_number_expr,                  /* parse expression     */
        &hemp_element_not_infix,                    /* parse infix expr     */
        &hemp_element_literal_source,               /* source code          */
        &hemp_element_number_text,                  /* output text          */
        &hemp_element_number_number,                /* numeric conversion   */
        &hemp_element_number_integer,               /* integer conversion   */
        &hemp_element_number_boolean,               /* boolean conversion   */
        &hemp_element_number_compare,               /* to comparison        */
    };

hemp_symbol_p HempSymbolNumber = &hemp_symbol_number;


static struct hemp_symbol_s
    hemp_symbol_integer = { 
        "integer",                                  /* name                 */
        NULL,                                       /* start token          */
        NULL,                                       /* end token            */
        HEMP_BE_SOURCE       |                       /* flags                */
        HEMP_BE_STATIC,
        0, 0,                                       /* l/r precedence       */
        NULL,                                       /* scanner callback     */
        NULL,                                       /* cleanup callback     */
        &hemp_element_number_expr,                  /* parse expression     */
        &hemp_element_not_infix,                    /* parse infix expr     */
        &hemp_element_literal_source,               /* source code          */
        &hemp_element_integer_text,                 /* output text          */
        &hemp_element_integer_number,               /* numeric conversion   */
        &hemp_element_integer_integer,              /* integer conversion   */
        &hemp_element_integer_boolean,              /* boolean conversion   */
        &hemp_element_integer_compare,              /* to comparison        */
    };
    
hemp_symbol_p HempSymbolInteger = &hemp_symbol_integer;



/*--------------------------------------------------------------------------
 * element to represent a literal number
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_symbol) {
    symbol->expr    = &hemp_element_number_expr,
    symbol->source  = &hemp_element_literal_source;
    symbol->text    = &hemp_element_number_text;
    symbol->number  = &hemp_element_number_number;
    symbol->integer = &hemp_element_number_integer;
    symbol->boolean = &hemp_element_number_boolean;
    symbol->flags   = HEMP_BE_SOURCE | HEMP_BE_STATIC;
    return symbol;
}


HEMP_PARSE_FUNC(hemp_element_number_expr) {
    hemp_debug_call(
        "hemp_element_number_parse_expr() precedence is %d, parg: %d\n", 
        (*elemptr)->type->lprec, precedence
    );
    
    hemp_element_p element = *elemptr;
  
    if (hemp_has_next(elemptr)) {
        hemp_go_next(elemptr);
        hemp_skip_whitespace(elemptr);
        hemp_symbol_p sym = (*elemptr)->type;

        if (sym->infix) {
//          return hemp_parse_infix(HEMP_PARSE_ARGS, element);
            return sym->infix(elemptr, scope, precedence, force, element);
        }
    }

    return element;
}


HEMP_OUTPUT_FUNC(hemp_element_number_text) {
    hemp_debug_call("hemp_element_number_text()\n");

    static hemp_char_t buffer[HEMP_BUFFER_SIZE];
    hemp_text_p text;

    snprintf(
        buffer, HEMP_BUFFER_SIZE, HEMP_FMT_NUM, 
        hemp_val_num(element->args.value)
    );
    hemp_prepare_output(output, text, strlen(buffer));
    hemp_text_append_cstr(text, buffer);

    return output;
}


HEMP_EVAL_FUNC(hemp_element_number_number) {
    hemp_debug_call("hemp_element_number_number()\n");
    return element->args.value;
}


HEMP_EVAL_FUNC(hemp_element_number_integer) {
    hemp_debug_call("hemp_element_number_integer()\n");
    hemp_value_t value = element->args.value;
    return hemp_is_int(value)
        ? value
        : hemp_int_val((hemp_int_t) hemp_val_num(value));
}


HEMP_EVAL_FUNC(hemp_element_number_boolean) {
    hemp_todo("hemp_element_number_boolean() : number element => boolean value\n");
}


HEMP_EVAL_FUNC(hemp_element_number_compare) {
    hemp_todo("hemp_element_number_compare()\n");
    // TODO: coerce number to HempBefore/HempEqual/HempAfter, etc
}



/*--------------------------------------------------------------------------
 * element to represent a literal integer
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_integer_symbol) {
    hemp_element_number_symbol(hemp, symbol);
    symbol->text    = &hemp_element_integer_text;
    symbol->number  = &hemp_element_integer_number;
    symbol->integer = &hemp_element_integer_integer;
    symbol->boolean = &hemp_element_integer_boolean;
    symbol->compare = &hemp_element_not_compare;
    return symbol;
}


HEMP_OUTPUT_FUNC(hemp_element_integer_text) {
    hemp_debug_call("hemp_element_integer_text()\n");

    static hemp_char_t buffer[HEMP_BUFFER_SIZE];
    hemp_text_p text;

    snprintf(
        buffer, HEMP_BUFFER_SIZE, HEMP_FMT_INT,
        hemp_val_int(element->args.value)
    );
    hemp_prepare_output(output, text, strlen(buffer));
    hemp_text_append_cstr(text, buffer);

    return output;
}


HEMP_EVAL_FUNC(hemp_element_integer_number) {
    hemp_debug_call("hemp_element_integer_number()\n");
    hemp_value_t value = element->args.value;
    
    // FIXME: not sure what's best to do here... do we allow number() 
    // functions to return integers?
    return value;
    return hemp_is_num(value)
        ? value
        : hemp_num_val((hemp_num_t) hemp_val_int(value));
}


HEMP_EVAL_FUNC(hemp_element_integer_integer) {
    hemp_debug_call("hemp_element_number_integer()\n");
    return element->args.value;
}


HEMP_EVAL_FUNC(hemp_element_integer_boolean) {
    hemp_todo("hemp_element_integer_boolean() : number element => boolean value\n");
}


HEMP_EVAL_FUNC(hemp_element_integer_compare) {
    hemp_todo("hemp_element_integer_compare()\n");
    // TODO: coerce number to HempBefore/HempEqual/HempAfter, etc
}


/*--------------------------------------------------------------------------
 * number operators
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_numop_symbol) {
    symbol->expr    = &hemp_element_not_expr;
    symbol->infix   = &hemp_element_parse_infix_left;
    symbol->source  = &hemp_element_binary_source;
    symbol->text    = &hemp_element_numop_text;
    symbol->integer = &hemp_element_numop_integer;
    symbol->boolean = &hemp_element_numop_boolean;
    symbol->compare = &hemp_element_not_compare;
    return symbol;
}


HEMP_OUTPUT_FUNC(hemp_element_numop_text) {
    hemp_debug_call("hemp_element_numop_text()\n");

    static hemp_char_t buffer[HEMP_BUFFER_SIZE];

    hemp_text_p text;
    hemp_value_t value = element->type->number(element, context);

    if (hemp_is_int(value))
        snprintf(buffer, HEMP_BUFFER_SIZE, HEMP_FMT_INT, hemp_val_int(value));
    else
        snprintf(buffer, HEMP_BUFFER_SIZE, HEMP_FMT_NUM, hemp_val_num(value));

    hemp_prepare_output(output, text, strlen(buffer));
    hemp_text_append_cstr(text, buffer);

    return output;
}


HEMP_EVAL_FUNC(hemp_element_numop_integer) {
    hemp_debug_call("hemp_element_numop_integer()\n");

    hemp_text_p text;
    hemp_value_t value = element->type->number(element, context);

    return hemp_is_int(value)
        ? value
        : hemp_int_val((int) hemp_val_num(value));
}


HEMP_EVAL_FUNC(hemp_element_numop_boolean) {
    hemp_debug_call("hemp_element_numop_boolean()\n");
    hemp_todo("hemp_element_numop_boolean()");
}


/*--------------------------------------------------------------------------
 * auto-increment
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_autoinc_symbol) {
    hemp_element_numop_symbol(hemp, symbol);
    symbol->expr   = &hemp_element_parse_prefix;
    symbol->infix  = &hemp_element_parse_postfix;
    symbol->number = &hemp_element_number_autoinc_value;
    return symbol;
}


HEMP_EVAL_FUNC(hemp_element_number_autoinc_value) {
    hemp_todo("hemp_element_number_autoinc_value()\n");
}


/*--------------------------------------------------------------------------
 * auto-decrement
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_autodec_symbol) {
    hemp_element_numop_symbol(hemp, symbol);
    symbol->expr   = &hemp_element_parse_prefix;
    symbol->infix  = &hemp_element_parse_postfix;
    symbol->number = &hemp_element_number_autodec_value;
    return symbol;
}


HEMP_EVAL_FUNC(hemp_element_number_autodec_value) {
    hemp_todo("hemp_element_number_autodec_value()\n");
}


/*--------------------------------------------------------------------------
 * number plus, e.g. '+'
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_plus_symbol) {
    hemp_element_numop_symbol(hemp, symbol);
    symbol->expr   = &hemp_element_parse_prefix;
    symbol->number = &hemp_element_number_plus_value;
    return symbol;
}


HEMP_EVAL_FUNC(hemp_element_number_plus_value) {
    hemp_debug_call("hemp_element_number_plus_value()\n");

    /* prefix unary '+' coerces value to a number */
    if (hemp_has_flag(element, HEMP_BE_PREFIX)) {
        hemp_element_p expr = element->args.unary.expr;
        return expr->type->number(expr, context);
    }

    /* otherwise it's an infix addition operator */
    hemp_element_p lhs = element->args.binary.lhs;
    hemp_element_p rhs = element->args.binary.rhs;
    hemp_value_t lval  = lhs->type->number(lhs, context);
    hemp_value_t rval, result;
    
    if (hemp_is_int(lval)) {
        result = hemp_int_val(
            hemp_val_int(lval) 
          + HEMP_NUMOP_GET_INT(rhs, context)
        );
    }
    else {
        rval   = rhs->type->number(rhs, context);
        result = hemp_num_val(
            hemp_val_num(lval) 
          + HEMP_NUMOP_CAST_NUM(rval)
        );
    }

    return result;
}


/*--------------------------------------------------------------------------
 * number minus, e.g. '-'
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_minus_symbol) {
    hemp_element_numop_symbol(hemp, symbol);
    symbol->expr   = &hemp_element_parse_prefix;
    symbol->number = &hemp_element_number_minus_value;
    return symbol;
}


HEMP_EVAL_FUNC(hemp_element_number_minus_value) {
    hemp_debug_call("hemp_element_number_minus_value()\n");

    /* prefix unary '-' coerces value to a number and negates it */
    if (hemp_has_flag(element, HEMP_BE_PREFIX)) {
        hemp_element_p expr = element->args.unary.expr;
        hemp_value_t   val  = expr->type->number(expr, context);

        return hemp_is_int(val)
            ? hemp_int_val(- hemp_val_int(val) )
            : hemp_num_val(- hemp_val_num(val) );
    }

    /* otherwise it's an infix subtraction operator */
    hemp_element_p lhs = element->args.binary.lhs;
    hemp_element_p rhs = element->args.binary.rhs;
    hemp_value_t lval  = lhs->type->number(lhs, context);
    hemp_value_t rval, result;

    if (hemp_is_int(lval)) {
        result = hemp_int_val(
            hemp_val_int(lval) 
          - HEMP_NUMOP_GET_INT(rhs, context)
        );
    }
    else {
        rval   = rhs->type->number(rhs, context);
        result = hemp_num_val(
            hemp_val_num(lval) 
          - HEMP_NUMOP_CAST_NUM(rval)
        );
    }

    return result;
}


/*--------------------------------------------------------------------------
 * number raised to power, e.g. '**', '^', or something similar
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_power_symbol) {
    hemp_element_numop_symbol(hemp, symbol);
    symbol->infix  = &hemp_element_parse_infix_right;
    symbol->number = &hemp_element_number_power_value;
    return symbol;
}


HEMP_EVAL_FUNC(hemp_element_number_power_value) {
    hemp_debug_call("hemp_element_number_power_value()\n");

    hemp_element_p lhs = element->args.binary.lhs;
    hemp_element_p rhs = element->args.binary.rhs;
    hemp_value_t lval  = lhs->type->number(lhs, context);
    hemp_value_t rval  = rhs->type->number(rhs, context);
    hemp_value_t result;

    if (hemp_is_int(lval)) {
        result = hemp_int_val(
            (hemp_int_t)
            pow( 
                (hemp_num_t) hemp_val_int(lval),
                HEMP_NUMOP_CAST_NUM(rval)
            )
        );
    }
    else {
        result = hemp_num_val(
            pow( 
                hemp_val_num(lval),
                HEMP_NUMOP_CAST_NUM(rval)
            )
        );
    }

    return result;
}


/*--------------------------------------------------------------------------
 * number multiplication, e.g. '*' or 'x'
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_multiply_symbol) {
    hemp_element_numop_symbol(hemp, symbol);
    symbol->number = &hemp_element_number_multiply_value;
    return symbol;
}


HEMP_EVAL_FUNC(hemp_element_number_multiply_value) {
    hemp_debug_call("hemp_element_number_multiply_value()\n");

    hemp_element_p lhs = element->args.binary.lhs;
    hemp_element_p rhs = element->args.binary.rhs;
    hemp_value_t lval  = lhs->type->number(lhs, context);
    hemp_value_t rval, result;

    if (hemp_is_int(lval)) {
        result = hemp_int_val(
            hemp_val_int(lval) 
          * HEMP_NUMOP_GET_INT(rhs, context)
        );
    }
    else {
        rval   = rhs->type->number(rhs, context);
        result = hemp_num_val(
            hemp_val_num(lval) 
          * HEMP_NUMOP_CAST_NUM(rval)
        );
    }

    return result;
}


/*--------------------------------------------------------------------------
 * number division, e.g. '/'
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_divide_symbol) {
    hemp_element_numop_symbol(hemp, symbol);
    symbol->number = &hemp_element_number_divide_value;
    return symbol;
}


HEMP_EVAL_FUNC(hemp_element_number_divide_value) {
    hemp_debug_call("hemp_element_number_divide_value()\n");

    hemp_element_p lhs = element->args.binary.lhs;
    hemp_element_p rhs = element->args.binary.rhs;
    hemp_value_t lval  = lhs->type->number(lhs, context);
    hemp_value_t rval, result;

    if (hemp_is_int(lval)) {
        result = hemp_int_val(
            hemp_val_int(lval) 
          / HEMP_NUMOP_GET_INT(rhs, context)
        );
    }
    else {
        rval   = rhs->type->number(rhs, context);
        result = hemp_num_val(
            hemp_val_num(lval) 
          / HEMP_NUMOP_CAST_NUM(rval)
        );
    }

    return result;
}


/*--------------------------------------------------------------------------
 * integer division forces both sides to integers, e.g. 'div'
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_divint_symbol) {
    hemp_element_numop_symbol(hemp, symbol);
    symbol->number  = &hemp_element_number_divint_value;
    symbol->integer = &hemp_element_number_divint_value;
    return symbol;
}


HEMP_EVAL_FUNC(hemp_element_number_divint_value) {
    hemp_debug_call("hemp_element_number_divint_value()\n");

    hemp_element_p lhs  = element->args.binary.lhs;
    hemp_element_p rhs  = element->args.binary.rhs;
    
    return hemp_int_val(
        (hemp_int_t)
        hemp_val_int( lhs->type->integer(lhs, context) )
      / hemp_val_int( rhs->type->integer(rhs, context) )
    );
}


/*--------------------------------------------------------------------------
 * integer modulus, e.g. '%', 'mod', etc.
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_modulus_symbol) {
    hemp_element_numop_symbol(hemp, symbol);
    symbol->number  = &hemp_element_number_modulus_value;
    symbol->integer = &hemp_element_number_modulus_value;
    return symbol;
}


HEMP_EVAL_FUNC(hemp_element_number_modulus_value) {
    hemp_debug_call("hemp_element_number_modulus_value()\n");

    hemp_element_p lhs  = element->args.binary.lhs;
    hemp_element_p rhs  = element->args.binary.rhs;
    
    return hemp_int_val(
        (hemp_int_t)
        hemp_val_int( lhs->type->integer(lhs, context) )
      % hemp_val_int( rhs->type->integer(rhs, context) )
    );
}


/*--------------------------------------------------------------------------
 * number comparison, e.g. '<=>'
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_compare_symbol) {
    hemp_element_compare_symbol(hemp, symbol);
    symbol->compare = &hemp_element_number_compare_value;
    return symbol;
}


HEMP_EVAL_FUNC(hemp_element_number_compare_value) {
    hemp_debug_call("hemp_element_number_compare_value()\n");

    hemp_element_p lhs = element->args.binary.lhs;
    hemp_element_p rhs = element->args.binary.rhs;
    hemp_value_t lval  = lhs->type->number(lhs, context);
    hemp_value_t rval  = rhs->type->number(rhs, context);
    hemp_num_t   lnum  = HEMP_NUMOP_CAST_NUM(lval);
    hemp_num_t   rnum  = HEMP_NUMOP_CAST_NUM(rval);
    
    return  lnum < rnum ? HempBefore
        :   lnum > rnum ? HempAfter
        :                 HempEqual;
}


/*--------------------------------------------------------------------------
 * number equality, e.g. '=='
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_equal_symbol) {
    hemp_element_compare_equal_symbol(hemp, symbol);
    symbol->compare = &hemp_element_number_compare_value;
    return symbol;
}


/*--------------------------------------------------------------------------
 * number inequality, e.g. '!=', '<>', etc.
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_not_equal_symbol) {
    hemp_element_compare_not_equal_symbol(hemp, symbol);
    symbol->compare = &hemp_element_number_compare_value;
    return symbol;
}


/*--------------------------------------------------------------------------
 * number less than, e.g. '<'
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_before_symbol) {
    hemp_element_compare_before_symbol(hemp, symbol);
    symbol->compare = &hemp_element_number_compare_value;
    return symbol;
}


/*--------------------------------------------------------------------------
 * number less than or equal to, e.g. '<=', '!>', etc
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_not_after_symbol) {
    hemp_element_compare_not_after_symbol(hemp, symbol);
    symbol->compare = &hemp_element_number_compare_value;
    return symbol;
}


/*--------------------------------------------------------------------------
 * number more than, e.g. '>'
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_after_symbol) {
    hemp_element_compare_after_symbol(hemp, symbol);
    symbol->compare = &hemp_element_number_compare_value;
    return symbol;
}


/*--------------------------------------------------------------------------
 * number more than more than or equal to, e.g. '>=', '!<', etc.
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_number_not_before_symbol) {
    hemp_element_compare_not_before_symbol(hemp, symbol);
    symbol->compare = &hemp_element_number_compare_value;
    return symbol;
}



