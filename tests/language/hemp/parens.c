#include <hemp/test.h>


int main(
    int argc, char **argv, char **env
) {
    Hemp       hemp    = hemp_new();
    HempContext    context = hemp_context(hemp);
    int             result  = 0;

    hemp_context_set_number(context, "a", 2);
    hemp_context_set_number(context, "b", 4);

    HEMP_TRY;
        result = hemp_test_expect_script(
            HEMP_TT3, HEMP_TT3, 
            HEMP_TEST_DIR, "parens", 
            context
            );
    HEMP_CATCH_ALL;
        printf("FAILED: %s\n", hemp_error_text(hemp->error)->string);
    HEMP_END;

    hemp_context_free(context);
    hemp_free(hemp);
    hemp_mem_trace_ok();

    return result;
}
