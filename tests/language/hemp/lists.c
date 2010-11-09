#include <hemp/test.h>


int main(
    int argc, char **argv, char **env
) {
    hemp_p      hemp    = hemp_init();
    hemp_cntx_p context = hemp_context(hemp);
    int         result;

    hemp_context_set_number(context, "a", 2);
    hemp_context_set_number(context, "b", 4);

    result = hemp_test_expect_script(
        HEMP_TT3, HEMP_TT3, 
        HEMP_TESTDIR, "lists", 
        context
    );

    hemp_context_free(context);
    hemp_free(hemp);
    hemp_mem_trace_ok();

    return result;
}
