#include <hemp/test.h>

void test_unplugged();


int
main(
    int  argc, 
    char **argv, 
    char **env
) {
    plan(8);
    test_unplugged();
    return done();
}


void test_unplugged() {
    Hemp       hemp = hemp_new();
    HempDocument   document;
    HempContext    context;
    HempValue      output;
    ok( hemp, "created hemp object" );

    HEMP_TRY;
        hemp_language(hemp, "json");
        pass("got json langauge"); 

        document = hemp_document(
            hemp, "json", HEMP_TEXT, "[10, 20, {w:10,x:null,y:false,z:true}]"
        );
        ok( document , "created document" );

        context = hemp_context_new(hemp);
        output  = hemp_document_data(document, context);

        ok( hemp_is_list(output), "got data list returned" );
        HempList list = hemp_val_list(output);
        eq( list->length, 3, "three items in json list" );
        
        HempValue  item = hemp_list_item(list, 2);
        ok( hemp_is_hash(item), "third item is hash" );
        HempHash   hash = hemp_val_hash(item);

        eq( hash->size, 4, "four items in hash in json list" );
        HempValue y = hemp_hash_fetch(hash, "y");
        ok( hemp_is_false(y), "y is false" );
    
        hemp_context_free(context);

    HEMP_CATCH_ALL;
        fail( "Caught error: %s", hemp->error->message );

    HEMP_END;

    hemp_free(hemp);
}


