#include <hemp/tag.h>


hemp_tag_p
hemp_tag_init(
    hemp_str_p       name,
    hemp_tag_style_t style,
    hemp_str_p       start,
    hemp_str_p       end,
    hemp_tag_scan_f  scan,
    hemp_grammar_p   grammar
) {
    hemp_tag_p tag = (hemp_tag_p) hemp_mem_alloc(
        sizeof(struct hemp_tag_s)
    );

    if (! tag)
        hemp_mem_fail("tag");

    tag->style   = style;
    tag->name    = hemp_string_copy(name);
    tag->start   = hemp_string_copy(start);
    tag->end     = end ? hemp_string_copy(end) : NULL;
    tag->scan    = scan;
    tag->grammar = grammar;

    if (! tag->name || ! tag->start || (end && ! tag->end))
        hemp_mem_fail("tag");

    return tag;
}


hemp_tag_p
hemp_tag_copy(
    hemp_tag_p tag
) {
    return hemp_tag_init(
        tag->name,
        tag->style,
        tag->start,
        tag->end,
        tag->scan,
        tag->grammar
    );
}


void
hemp_tag_set_name(
    hemp_tag_p tag,
    hemp_str_p name
) {
    hemp_mem_free(tag->name);

    tag->name = hemp_string_copy(name);

    if (! tag->name)
        hemp_mem_fail("tag name");
}


void
hemp_tag_set_start(
    hemp_tag_p tag,
    hemp_str_p start
) {
    hemp_mem_free(tag->start);

    tag->start = hemp_string_copy(start);

    if (! tag->start)
        hemp_mem_fail("tag start");
}


void
hemp_tag_set_end(
    hemp_tag_p tag,
    hemp_str_p end
) {
    if (tag->end)
        hemp_mem_free(tag->end);

    if (end) {
        tag->end = hemp_string_copy(end);

        if (! tag->end)
            hemp_mem_fail("tag end");
    }
    else {
        /* end can be set NULL to indicate no explicit end tag */
        tag->end = NULL;
    }
}


void
hemp_tag_set_start_end(
    hemp_tag_p tag,
    hemp_str_p start,
    hemp_str_p end
) {
    hemp_tag_set_start(tag, start);
    hemp_tag_set_end(tag, end);
}


void
hemp_tag_free(
    hemp_tag_p tag
) {
    hemp_mem_free(tag->name);
    hemp_mem_free(tag->start);

    if (tag->end)
        hemp_mem_free(tag->end);
    
    hemp_mem_free(tag);
}

