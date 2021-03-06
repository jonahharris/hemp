#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"
#include <hemp.h>
#include "types.h"

typedef Hemp            Hemp__Hemp;
typedef HempDocument    Hemp__Template;
typedef HempContext     Hemp__Context;
typedef HempText        Hemp__Text;

/*
#--------------------------------------------------------------------------
# Hemp
#--------------------------------------------------------------------------
*/

MODULE = Hemp           PACKAGE = Hemp

PROTOTYPES: enable


char *
hemp_version()
    CODE:
        RETVAL = hemp_version();
    OUTPUT:
        RETVAL

Hemp
new(package)
    char *package
    INIT:
        fprintf(stderr, "Hemp->new()\n");
    CODE:
        RETVAL = hemp_new();
        hemp_perl_init();
    OUTPUT:
        RETVAL


void 
language(hemp, language)
    Hemp   hemp
    char * language
    INIT:
        fprintf(stderr, "Hemp->language('%s')\n", language); 
    PPCODE:
        hemp_language(hemp, language);


Hemp::Template
document(hemp, dialect, scheme, source)
    Hemp   hemp
    char * dialect
    char * scheme
    char * source
    INIT:
        fprintf(stderr, "Hemp->document('%s', '%s', '%s')\n", dialect, scheme, source);
    CODE:
        RETVAL = hemp_document(hemp, dialect, scheme, source);
    OUTPUT:
        RETVAL


Hemp::Context
context(hemp)
    Hemp   hemp
    INIT:
        fprintf(stderr, "Hemp->context()\n");
    CODE:
        RETVAL = hemp_context(hemp);
    OUTPUT:
        RETVAL


void 
DESTROY(hemp)
    Hemp  hemp
    INIT:
        fprintf(stderr, "Hemp->DESTROY()\n"); 
    PPCODE:
        hemp_free(hemp);


#--------------------------------------------------------------------------
# Hemp::Template
#--------------------------------------------------------------------------

MODULE = Hemp           PACKAGE = Hemp::Template

PROTOTYPES: enable


Hemp::Text
render(document, context)
    Hemp::Template document
    Hemp::Context  context
    INIT:
        fprintf(stderr, "Hemp::Template->render()\n");
    CODE:
        Hemp hemp = context->hemp;
        HEMP_TRY;
            RETVAL = hemp_document_render(document, context);
        HEMP_CATCH_ALL;
            croak("Hemp error: %s", hemp->error->message);
        HEMP_END;
    OUTPUT:
        RETVAL


#--------------------------------------------------------------------------
# Hemp::Context
#--------------------------------------------------------------------------

MODULE = Hemp           PACKAGE = Hemp::Context

PROTOTYPES: enable


void 
set(context, name, value)
    Hemp::Context   context
    char *  name
    SV   *  value
    INIT:
        fprintf(stderr, "Hemp::Context->set(%s, %s)\n", name, SvPV_nolen(value));
    PPCODE:
        hemp_context_set(context, name, hemp_perl_value(value));


void
DESTROY(context)
    Hemp::Context context
    INIT:
        fprintf(stderr, "Hemp::Context->DESTROY()\n"); 
    PPCODE:
        hemp_context_free(context);


#--------------------------------------------------------------------------
# Hemp::Text
#--------------------------------------------------------------------------

MODULE = Hemp           PACKAGE = Hemp::Text

PROTOTYPES: enable


char *
text(text)
    Hemp::Text text
    INIT:
        fprintf(stderr, "Hemp::Text->text()\n");
    CODE:
        RETVAL = text->string;
    OUTPUT:
        RETVAL


void
DESTROY(text)
    Hemp::Text text
    INIT:
        fprintf(stderr, "Hemp::Text->DESTROY()\n"); 
    PPCODE:
        hemp_text_free(text);

