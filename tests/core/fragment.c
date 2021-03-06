#include <hemp/test.h>

void test_fragment();
void test_fragment_factory();

int main(
    int argc, char **argv, char **env
) {
    plan(5);

    test_fragment();
    test_fragment_factory();

    return done();
}


void test_fragment() {
    Hemp hemp = hemp_new();
    HempFragment fragment = hemp_fragment_new(
        HempElementSpace, NULL, 0, 0
    );
    ok( fragment, "created fragment" );
    hemp_fragment_free(fragment);
    hemp_free(hemp);
}


void test_fragment_factory() {
    HempElement element;

    Hemp hemp = hemp_new();
    ok( hemp, "created hemp" );

    HEMP_TRY;
        element = hemp_element(hemp, "hemp.number.plus", "+", NULL);
        ok( element, "got %s element", element->name );
        hemp_element_free(element);

        element = hemp_element(hemp, "hemp.number.plus", "+", NULL);
        ok( element, "got it again" );
        hemp_element_free(element);
    HEMP_CATCH_ALL;
        fail("unexpected error: %s", hemp->error->message);
    HEMP_END;

    HEMP_TRY;
        element = hemp_element(hemp, "blah.blah", "dud", NULL);
    HEMP_CATCH(HEMP_ERROR_INVALID);
        pass("error as expected: %s", hemp->error->message);
    HEMP_CATCH_ALL;
        fail("unexpected error: %s", hemp->error->message);
    HEMP_END;
    
    hemp_free(hemp);
}
