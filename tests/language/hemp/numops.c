#include <hemp/test.h>


int main(
    int argc, char **argv, char **env
) {
    Hemp       hemp    = hemp_new();
    HempContext    context = hemp_context(hemp);
    int             result  = 0;

    hemp_context_set_integer( context, "answer", 42 );
    hemp_context_set_string( context, "n", "400" );

    HEMP_TRY;
        result = hemp_test_expect_script(
            HEMP_TT3, HEMP_TT3, 
            HEMP_TEST_DIR, "numops",
            context
        );

        hemp_context_free(context);
        hemp_free(hemp);
        hemp_mem_trace_ok();

    HEMP_CATCH_ALL;
        fprintf(stderr, "Hemp error: %s", hemp->error->message);

    HEMP_END;


    return result;
}
