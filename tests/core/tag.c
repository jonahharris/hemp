#include <hemp/test.h>

void test_tag();
void test_hemp_tags();


int main(
    int argc, char **argv, char **env
) {
    plan(12);
    test_tag();
    return done();
}


void test1_scan(
    hemp_tag_p  tag,
    hemp_str_p  start, 
    hemp_pos_t  pos, 
    hemp_str_p *src
) {
    hemp_debug("custom scanner");
}


void test_tag() {
    hemp_tag_p tag, copy;

    tag = hemp_tag_init(
        "test1", HEMP_INLINE_TAG, "[%", "%]", 
        NULL, NULL
    );

    ok( tag, "created tag object" );
    ok( hemp_string_eq(tag->name, "test1"), "name is test1" );
    ok( hemp_string_eq(tag->start, "[%"), "start is [%" );
    ok( hemp_string_eq(tag->end, "%]"), "end is %]" );

    hemp_tag_set_name(tag, "test2");
    hemp_tag_set_start(tag, "[*");
    hemp_tag_set_end(tag, "*]");

    ok( hemp_string_eq(tag->name, "test2"), "name is test2" );
    ok( hemp_string_eq(tag->start, "[*"), "start is [*" );
    ok( hemp_string_eq(tag->end, "*]"), "end is *]" );

    hemp_tag_set_start_end(tag, "<-", "->");
    ok( hemp_string_eq(tag->start, "<-"), "start is <-" );
    ok( hemp_string_eq(tag->end, "->"), "end is ->" );

    copy = hemp_tag_copy(tag);
    hemp_tag_set_name(copy, "test3");

    ok( hemp_string_eq(copy->name, "test3"), "name is test3" );
    ok( hemp_string_eq(copy->start, "<-"), "copy start is <-" );
    ok( hemp_string_eq(copy->end, "->"), "copy end is ->" );

    hemp_tag_free(tag);
    hemp_tag_free(copy);
}


//void test_hemp_tags() {
//    hemp_p hemp = hemp_init();
//    ok( hemp, "got hemp object" );
//
//    hemp_tag_p comment_tag = hemp_tag(hemp, "comment");
//    ok( comment_tag, "got comment tag" );
//    
//    hemp_free(hemp);
//}

