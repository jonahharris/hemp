#include "hemp/dialect.h"
#include "hemp/template.h"
#include "hemp/debug.h"
#include "hemp/types.h"

#define CR 13
#define LF 10


static struct hemp_dialect 
    hemp_dialect_tt3 = { 
        "tt.dialect.tt3", 
        &hemp_dialect_tt3_scan
    };

hemp_dialect_t HempDialectTT3 = &hemp_dialect_tt3;

hemp_tag_t
hemp_scan_tag_start(
    hemp_pnode_t pnode, 
    hemp_text_t  *src
);


// tmp hack until we can build a tagset into a dialect
hemp_tagset_t tmp_hack_make_tagset() {
    hemp_tagset_t tagset = hemp_tagset_init();
    
    hemp_tagset_add_inline_tag(tagset, HempTagInline);
    hemp_tagset_add_inline_tag(tagset, HempTagComment);
    hemp_tagset_add_inline_tag(tagset, HempTagControl);
    hemp_tagset_add_inline_tag(tagset, HempTagVariable);
    hemp_tagset_add_inline_tag(tagset, HempTagEmbed);
    hemp_tagset_add_outline_tag(tagset, HempTagOutline);
    
    return tagset;
}



hemp_bool_t
hemp_dialect_tt3_scan(
    hemp_template_t tmpl
) {
    hemp_tagset_t   tagset   = tmp_hack_make_tagset();
    hemp_text_t     text     = hemp_source_read(tmpl->source),
                    src      = text,
                    from     = text,
                    cmptr, 
                    tag_pos;
    hemp_pos_t      pos      = 0,
                    line     = 0;
    hemp_tag_t      tag;
    hemp_pnode_t    *inhead  = tagset->inline_tags->head,
                    *outhead = tagset->outline_tags->head,
                    pnode;
    hemp_elements_t elements = tmpl->elements;

    debug_magenta("-- source ---\n%s\n------------\n", text);
    
    while (*src) {
        /* at start of line */
        line++;
        debug("\n%d (%02d) : ", line, src - text);

        if ((pnode = outhead[(unsigned char) *src])) {
            debug_yellow("OUTLINE:%c", *src);
            tag_pos = src;
            
            if ((tag = hemp_scan_tag_start(pnode, &src))) {
                if (tag_pos > from) {
                    hemp_elements_append(
                        elements, tagset->text_element,
                        from, pos, tag_pos - from
                    );
                    pos += tag_pos - from;
                    from = tag_pos;
                }
                hemp_elements_append(
                    elements, HempElementTagStart,
                    tag_pos, pos, src - tag_pos
                );
                from = src;
                pos += src - tag_pos;
            }
        }
        
        while (*src) {
            if (*src == LF) {
                src++;
                break;
            }
            else if (*src == CR) {
                src++;
                if (*src == LF)
                    src++;
                break;
            }
            else if ((pnode = inhead[(unsigned char) *src])) {
                debug_yellow("INLINE:%c", *src);

                tag_pos = src;

                if ((tag = hemp_scan_tag_start(pnode, &src))) {
                    if (tag_pos > from) {
                        hemp_elements_append(
                            elements, tagset->text_element,
                            from, pos, tag_pos - from
                        );
                        pos += tag_pos - from;
                        from = tag_pos;
                    }
                    hemp_elements_append(
                        elements, HempElementTagStart,
                        tag_pos, pos, src - tag_pos
                    );
                    from = src;
                    pos += src - tag_pos;
                }
                else {
                    src++;
                }
                /* TODO: call tag handler which consumes everything up to tag
                 * end.  Set from to src position
                 */
            }
            else {
                debug("%c", *src);
                src++;
            }
        }
    }

    if (src > from) {
//      debug("got %d characters of trailing text\n", src - from);
        hemp_elements_append(
            elements, tagset->text_element,
            from, pos, src - from
        );
        pos += src - from;
    }
    
    hemp_elements_eof(elements, src, pos);
    hemp_elements_dump(elements);
    hemp_elements_free(elements);
}


hemp_tag_t
hemp_scan_tag_start(
    hemp_pnode_t pnode, 
    hemp_text_t  *srcptr
) {
//  debug_blue("hemp_scan_tag_start(%p[%s])]", pnode, pnode->key);

    hemp_text_t cmptr  = pnode->key;
    hemp_text_t src    = *srcptr;
    hemp_tag_t  tag    = NULL;

    // nuisance: we do the first comparison twice
    while (1) {
        if (*src == *cmptr) {
            src++;

            if (pnode->equal) {
//              debug_yellow("[%c->equal]", *cmptr);
                tag   = pnode->value;     // payload - but only if pnode is set - don't want to trash previous match
                pnode = pnode->equal;
                cmptr = pnode->key;
            }
            else if (* ++cmptr) {
//              debug_yellow("[%c->more => %c]", *(cmptr - 1), cmptr);
            }
            else {
                tag = pnode->value;
                break;
            }
        }
        else if (*src < *cmptr && pnode->before) {
//          debug_blue("[%c is before %c]", *src, *cmptr);
            pnode = pnode->before;
            cmptr = pnode->key;
        }
        else if (*src > *cmptr && pnode->after) {
//          debug_blue("[%c is after %c]", *src, *cmptr);
            pnode = pnode->after;
            cmptr = pnode->key;
        }
        else {
            break;
        }
    }
  
    if (tag) {
        *srcptr = src;
        debug_green("<%s>", tag->name);
    }
    else {
        debug_red("[MISS]");
    }

    return tag;
}
