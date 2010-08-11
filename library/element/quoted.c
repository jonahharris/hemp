#include <hemp/element.h>


/*--------------------------------------------------------------------------
 * single quoted strings
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_squote_symbol) {
    symbol->flags      = HEMP_BE_STATIC;
    symbol->scanner    = &hemp_element_squote_scanner;
    symbol->cleanup    = &hemp_element_text_clean;
    symbol->expr       = &hemp_element_literal_expr,
    symbol->source     = &hemp_element_literal_source;
    symbol->text       = &hemp_element_quoted_text;
    symbol->number     = &hemp_element_literal_number;
    symbol->integer    = &hemp_element_literal_integer;
    symbol->boolean    = &hemp_element_literal_boolean;
    return symbol;
}


HEMP_SCAN_FUNC(hemp_element_squote_scanner) {
    hemp_cstr_p     src       = *srcptr;
    hemp_bool_t     is_source = HEMP_TRUE;
    hemp_cstr_p     end       = symbol->end;
    hemp_size_t     endlen    = strlen(end);
    hemp_element_p  element;

    debug_call("hemp_element_squote_scanner()\n");

    if (! end)
        end = start;

    /* walk to the end */
    while (1) {
        if (*src == *end
        && (endlen == 1 || hemp_cstrn_eq(src, end, endlen)))
            break;
        if (*src == HEMP_BACKSLASH) {
            src++;
            is_source = HEMP_FALSE;
        }
        if (! *++src)
            HEMP_SCAN_ERROR(BADQUOTE,0);
    }

    /* skip over the terminating character(s) */
    end  = src;
    src += endlen;

    debug_token("SQUOTE", start, src-start);

    element = hemp_elements_append(
        tmpl->elements, symbol,
        start, pos, src - start
    );

    if (is_source) {
        /* we can generate the output text from the source token */
        hemp_set_flag(element, HEMP_BE_SOURCE);
    }
    else {
        /* we need to create a new string with escapes resolved */
        hemp_cstr_p sqfrom  = *srcptr;
        hemp_cstr_p squote  = (hemp_cstr_p) hemp_mem_alloc(end - sqfrom + 1);
        element->args.value = HEMP_STR_VAL(squote);
                    
        while (sqfrom < end) {
            /* skip past the '\' if we've got "\\" or "\'" */
            if (*sqfrom == HEMP_BACKSLASH 
            && ( *(sqfrom + 1) == HEMP_BACKSLASH 
            ||   *(sqfrom + 1) == *end ))
                sqfrom++;

            *squote++ = *sqfrom++;
        }
        *squote = HEMP_NUL;
    }

    *srcptr = src;
    return element;
}


/*--------------------------------------------------------------------------
 * double quoted strings
 *--------------------------------------------------------------------------*/

HEMP_SYMBOL_FUNC(hemp_element_dquote_symbol) {
    symbol->scanner    = &hemp_element_dquote_scanner;
    symbol->cleanup    = &hemp_element_text_clean;
    symbol->expr       = &hemp_element_literal_expr,
    symbol->source     = &hemp_element_literal_source;
    symbol->text       = &hemp_element_quoted_text;
    symbol->number     = &hemp_element_literal_number;
    symbol->integer    = &hemp_element_literal_integer;
    symbol->boolean    = &hemp_element_literal_boolean;
    return symbol;
}


HEMP_SCAN_FUNC(hemp_element_dquote_scanner) {
    hemp_cstr_p     src       = *srcptr;
    hemp_bool_t     is_source = HEMP_TRUE;
    hemp_cstr_p     end       = symbol->end;
    hemp_size_t     endlen    = strlen(end);
    hemp_element_p  element;

    debug_call("hemp_element_dquote_scanner()\n");

    if (! end)
        end = start;

//  debug("end token: %s  length: %d\n", end, endlen);

    /* walk to the end */
    while (1) {
        if (*src == *end
        && (endlen == 1 || hemp_cstrn_eq(src, end, endlen)))
            break;
        if (*src == HEMP_BACKSLASH) {
            src++;
            is_source = HEMP_FALSE;
        }
        if (! *++src)
            HEMP_SCAN_ERROR(BADQUOTE,0);
    }

    /* skip over the terminating character(s) */
    end  = src;
    src += endlen;

    debug_token("SQUOTE", start, src-start);

    element = hemp_elements_append(
        tmpl->elements, symbol,
        start, pos, src - start
    );

    if (is_source) {
        /* we can generate the output text from the source token */
        hemp_set_flag(element, HEMP_BE_SOURCE);
    }
    else {
        /* we need to create a new string with escapes resolved */
        hemp_cstr_p dqfrom  = *srcptr;
        hemp_cstr_p dquote  = (hemp_cstr_p) hemp_mem_alloc(end - dqfrom + 1);        // CHECK ME
        element->args.value = HEMP_STR_VAL(dquote);
                    
        while (dqfrom < end) {
            if (*dqfrom == HEMP_BACKSLASH) {
                if (*(dqfrom + 1) == *end) {
                    /* end token is escaped, e.g. "18\" Stonehenge monument */
                    dqfrom++;
                    continue;
                }
                switch (*(dqfrom + 1)) {
                    case HEMP_BACKSLASH:
                        /* \" or \\  =>  " or \ */
                        dqfrom++;
                        break;

                    case 'n':
                        /* \n => newline (currently just LF) */
                        *dquote++ = HEMP_NL;
                        dqfrom += 2;
                        break;

                    case 't':
                        /* \t => tab */
                        *dquote++ = HEMP_TAB;
                        dqfrom += 2;
                        break;

                    default:
                        /* \X => X */
                        *dquote++ = *dqfrom++;
                }
            }
            else {
                *dquote++ = *dqfrom++;
            }
        }
        *dquote = HEMP_NUL;
    }

    *srcptr = src;
    return element;
}


/*--------------------------------------------------------------------------
 * generic text output function for quoted strings
 *--------------------------------------------------------------------------*/

HEMP_OUTPUT_FUNC(hemp_element_quoted_text) {
    debug_call("hemp_element_quoted_text(%p) [%s]\n", element, element->type->name);

    hemp_text_p text;
    hemp_prepare_output(output, text, element->length);

    if (hemp_has_flag(element, HEMP_BE_SOURCE)) {
        /* quoted string can be regenerated from source */
        hemp_symbol_p symbol = element->type;
        hemp_size_t slen     = symbol->start ? strlen(symbol->start) : 0;
        hemp_size_t elen     = symbol->end   ? strlen(symbol->end)   : 0;

        /* "pinch" in the ends of the token range to avoid the quotes */
        hemp_text_append_cstrn(
            text, 
            element->token  + slen,         /* skip past start              */
            element->length - slen - elen   /* shorten by start/end lengths */
        );
    }
    else {
        /* TODO: check that it's OK to assume we always have a value */
        hemp_text_append_cstr(text, HEMP_VAL_STR(element->args.value));
    }

    return output;
}


