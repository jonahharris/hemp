#include "hemp/template.h"
#include "hemp/dialect.h"
#include "hemp/debug.h"


hemp_template_t 
hemp_template_init(
    hemp_text_t    scheme, 
    hemp_text_t    source,
    hemp_dialect_t dialect
) {
    hemp_template_t tmpl;
    
    if (! dialect)
        dialect = hemp_dialect_tt3();

    if ((tmpl = (hemp_template_t) hemp_mem_init(sizeof(struct hemp_template)))) {
        tmpl->source   = hemp_source(scheme, source);
        tmpl->elements = hemp_elements_init(0);
        tmpl->dialect  = dialect;
        
        if (tmpl->source && tmpl->elements) {
            debug_cyan(
                "Allocated %s %s template at %p\n", 
                dialect->name, scheme, tmpl
            );
        }
        else {
            hemp_template_free(tmpl);
            tmpl = NULL;
        }
    }

    // TODO handle tmpl == NULL
    return tmpl;
}


void
hemp_template_free(
    hemp_template_t tmpl
) {
    if (tmpl->source) {
        hemp_source_free(tmpl->source);
    }
    if (tmpl->elements) {
        hemp_elements_free(tmpl->elements);
    }
    debug_cyan("Releasing %s template at %p\n", tmpl->source->scheme->name, tmpl);
    hemp_mem_free(tmpl);
}

hemp_bool_t
hemp_template_compile(
    hemp_template_t tmpl
) {
    if (tmpl->dialect->scanner(tmpl)) {
        debug_green("scanned OK\n");
    }
    else {
        debug_red("did not scan\n");
    }
}

