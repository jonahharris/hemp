#include <hemp/test.h>


void test_elements();

int main(
    int argc, char **argv, char **env
) {
    plan(4);
    test_elements();
    return done();
}

void test_elements() {
    hemp_hemp hemp = hemp_new();
    ok( hemp, "created hemp" );

    hemp_elements elements = hemp_elements_new(hemp, 0);
    ok( elements, "created elements" );
    
    hemp_element eof = hemp_elements_eof(elements, 0);
    ok( eof, "got EOF element" );
    ok( hemp_has_flag(eof, HEMP_BE_EOF), "HEMP_BE_EOF is set" );

    hemp_elements_free(elements);
    hemp_free(hemp);
}
