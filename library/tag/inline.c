#include <limits.h>
#include <errno.h>
#include <hemp/core.h>
#include <hemp/ptree.h>
#include <hemp/element.h>
#include <hemp/tag.h>
#include <hemp/scanner.h>
#include <hemp/element.h>
#include <hemp/grammar.h>



HEMP_TAG(hemp_tag_inline) {
//  hemp_debug_msg(
//      "hemp_tag_inline(%p, %s, %s, %s, %s, %s)\n",
//      hemp, type, name, start, end ? end : "", 
//      grammar ? grammar->name : "no grammar"
//  );

    hemp_tag tag = hemp_tag_new(
        type, name, start, end, grammar
    );
    tag->style  = HEMP_INLINE_TAG;
    tag->scan   = &hemp_tag_inline_scan;
    tag->to_eol = &hemp_tag_inline_to_eol;

    return tag;
}


void 
hemp_tag_inline_scan(
    hemp_template   tmpl,
    hemp_tag        tag,
    hemp_string     tagtok,
    hemp_pos        pos,
    hemp_string     *srcptr
) {
    hemp_string     src     = *srcptr,
                    from    = src;
    hemp_string     tagend  = tag->end;
    hemp_size       endlen  = strlen(tagend);
    hemp_num        num_val = 0;
    hemp_int        int_val = 0;
    hemp_element    element;
    hemp_bool       is_int, is_word;
    hemp_pnode      pnode;
    hemp_symbol     symbol;

    hemp_debug_call("hemp_scan_inline_tag()\n");

    // add the tag start token
    hemp_fragments_add_fragment(
        tmpl->fragments, HempElementTagStart,
        tagtok, pos, src - tagtok
    );
    pos += src - tagtok;
    
    is_word = HEMP_FALSE;

    while (*src) {
        if (isspace(*src)) {
            /* whitespace */
            hemp_scan_while(src, isspace);
            hemp_debug_token("SPACE", from, src-from);
            hemp_fragments_add_fragment(
                tmpl->fragments, HempElementSpace,
                from, pos, src - from
            );
        }
        else if (hemp_stringn_eq(src, tagend, endlen)) {      // TODO: end flags
            /* tag end */
            hemp_debug_token("TAG END", from, endlen);
            hemp_fragments_add_fragment(
                tmpl->fragments, HempElementTagEnd,
                from, pos, endlen
            );
            src += endlen;
            break;                                          /* ESCAPE ROUTE */
        }
        else if (isdigit(*src)) {
            /* number - try integer first */
            errno   = 0;
            int_val = strtol(src, &src, 0);
            is_int  = HEMP_TRUE;

            /* If there's a decimal point and a digit following then it's a 
             * floating point number.  We also look out for e/E which also
             * indicate fp numbers in scientific notation, e.g. 1.23e6.
             * Note that we don't accept the decimal point if the next 
             * character is not a digit.  This is required to support methods
             * called against numeric constants, e.g. 12345.hex 
             */
            if ( ( *src == '.' && isdigit(*(src + 1)) )
              || ( *src == 'e' || *src == 'E' )
            )  {
                is_int  = HEMP_FALSE;
                num_val = strtod(from, &src);
            }

            if (errno == ERANGE) {
                /* TODO: trim next token out of text */
                hemp_throw(tmpl->dialect->hemp, HEMP_ERROR_OVERFLOW, from);
            }
            else if (errno) {
                /* should never happen (famous last words) as we pre-check 
                 * that there is at least one valid digit available to be 
                 * parsed, but we check anyway
                 */
                hemp_fatal("Unknown number parsing error: %d", errno);
            }
            else if (is_int) {
                hemp_debug_token("INTEGER", from, src-from);
                element = hemp_fragments_add_fragment(
                    tmpl->fragments, HempElementInteger,
                    from, pos, src - from
                );
                element->args.value = hemp_int_val(int_val);
            }
            else {
                hemp_debug_token("NUMBER", from, src-from);
                element = hemp_fragments_add_fragment(
                    tmpl->fragments, HempElementNumber,
                    from, pos, src - from
                );
                element->args.value = hemp_num_val(num_val);
            }
        }
        else if (
            (pnode  = hemp_ptree_root(tag->grammar->operators, src))
        &&  (symbol = (hemp_symbol) hemp_pnode_match_more(pnode, &src))
        ) {
            hemp_debug_token("OPERATOR", from, src-from);

            /* We have to be check that we haven't matched the first part of
             * a longer word, e.g. matching 'le' at the start of 'length'.
             * However, we also have to account for the fact that symbols
             * may contain a mixture of word and non-word characters, e.g.
             * 'C<', 'q/' and so on.  It's OK to have a non-word followed by
             * a word, e.g. 'q/a' ('q/' is the operator, 'a' the next word),
             * or a word followed by a non-word, e.g. '+foo'.
             *
             * If we find that we are at a word/word boundary then we pretend
             * this bit never happened by jumping down to the next check...
             * don't look if you're squeamish about the use of "goto"
             */
            if (isalpha(*src) && isalpha(*(src - 1)))
                goto bareword;

            if (symbol->scanner) {
//              hemp_debug("symbol has dedicated scanner\n");
                symbol->scanner(tmpl, tag, from, pos, &src, symbol);
            }
            else {
                hemp_fragments_add_fragment(
                    tmpl->fragments, symbol,
                    from, pos, src - from
                );
            }
        }
        else if (isalpha(*src)) {
bareword:
            /* word */
            hemp_scan_while(src, isalnum);
            // TODO: check for ':' following after, e.g. file:/blah/blah
            hemp_debug_token("WORD", from, src-from);
            hemp_fragments_add_fragment(
                tmpl->fragments, HempElementWord,
                from, pos, src - from
            );
        }
        else {
            hemp_throw(tmpl->dialect->hemp, HEMP_ERROR_TOKEN, src);
            break;
        }

        pos += src - from;
        from = src;
    }
    
    *srcptr = src;
}


hemp_string
hemp_tag_inline_to_eol(
    HEMP_TAG_SKIP_ARGS
) {
    hemp_debug_call("hemp_tag_inline_to_eol()\n");
    hemp_string  tag_end = tag->end;
    hemp_size tag_len = strlen(tag->end);

    /* walk to the end of line or end of tag */
    while ( *src
        &&  hemp_not_newline(src) 
        &&  hemp_not_tag_end(src, tag_end, tag_len)
    ) {
        src++;
    }
    
    return src;
}
