#include <hemp/test.h>


int main(
    int argc, char **argv, char **env
) {
    hemp_hemp      hemp    = hemp_new();
    hemp_list list    = hemp_list_new();
    hemp_hash hash    = hemp_hash_new();
    hemp_context context = hemp_context_instance(hemp);
    int         result;

    hemp_text message = hemp_text_from_string("Hello World!");
    
    hemp_context_set_number(  context, "e",         2.718    );
    hemp_context_set_number(  context, "pi",        3.14159  );
    hemp_context_set_integer( context, "answer",    42       );
    hemp_context_set_string(  context, "name",      "Badger" );
    hemp_context_set_text(    context, "message",   message );
    hemp_context_set_hash(    context, "stuff",     hash );
    hemp_context_set_list(    context, "items",     list );

    hemp_hash_store_number(hash, "foo", 69);
    hemp_hash_store_number(hash, "bar", 420);
    hemp_hash_store_string(hash, "woo", "hoo!");
    
    hemp_list_push_integer(list, 99);
    hemp_list_push_number(list, 100.0);
    hemp_list_push_string(list, "one-oh-one");

    result = hemp_test_expect_script(
        HEMP_TT3, HEMP_TT3, 
        HEMP_TEST_DIR, "words", 
        context
    );

    hemp_text_free(message);
    hemp_list_free(list);
    hemp_hash_free(hash);
    hemp_context_free(context);
    hemp_free(hemp);

    hemp_mem_trace_ok();

    return result;
}
