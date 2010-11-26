#include <hemp/test.h>


int test_test_language();

int main(
    int argc, char **argv, char **env
) {
//    plan(20);
    return test_test_language();
//    return done();
}


int test_test_language() {
    hemp_hemp          hemp    = hemp_init();
    hemp_context     context = hemp_context_instance(hemp);
    hemp_string      name    = "test1";
    hemp_template tmpl;
    hemp_text     output;

    hemp_string  dir    = hemp_filesystem_join_path(HEMP_TESTDIR, "scripts");
    hemp_string  file   = hemp_filesystem_join_path(dir, name);
    hemp_string  text   = hemp_filesystem_read_file(file);

    hemp_debug("text: %s\n", text);
    hemp_language(hemp, HEMP_TEST);

    hemp_test_plan_p plan = hemp_test_plan(5);

    HEMP_TRY;
        tmpl = hemp_template_instance(
            hemp,
            HEMP_TEST,
            HEMP_TEXT, 
            text
        );
        output = hemp_template_render(tmpl, context);
        ok( output, "%s rendered", name);
        fprintf(stderr, "OUTPUT:\n%s\n/OUTPUT\n", output->string);

        hemp_text_free(output);
        hemp_mem_free(text);
        hemp_mem_free(file);
        hemp_mem_free(dir);
        hemp_context_free(context);
        hemp_free(hemp);
        hemp_mem_trace_ok();

    HEMP_CATCH_ALL;
        fprintf(stderr, "Hemp error: %s", hemp->error->message);

    HEMP_END;

    return hemp_test_done(plan);
//    return 0;
}
