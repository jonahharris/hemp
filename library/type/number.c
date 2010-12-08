#include <hemp/value.h>
#include <hemp/context.h>


HEMP_TYPE_FUNC(hemp_type_number) {
    hemp_type type = hemp_type_subtype(HempValue, id, name);
    type->text    = &hemp_type_number_text;    /* convert number to text           */
    type->number  = &hemp_value_self;           /* no-op to return number           */
    type->integer = &hemp_type_number_integer; /* number -> integer conversion     */
    type->boolean = &hemp_type_number_boolean; /* number -> boolean conversion     */
    type->compare = &hemp_type_number_compare; /* number -> comparison conversion  */
    type->defined = &hemp_value_true;           /* number is always defined         */  /* what about NaN / Infinity? */
    return type;
};


HEMP_OUTPUT(hemp_type_number_text) {
    static hemp_char buffer[HEMP_BUFFER_SIZE];
    hemp_text text;

    snprintf((hemp_string) buffer, HEMP_BUFFER_SIZE, HEMP_FMT_NUM, hemp_val_num(value));
    hemp_prepare_text_size(context, output, text, strlen((hemp_string) buffer));
    hemp_text_append_string(text, (hemp_string) buffer);

    return output;
}


HEMP_VALUE(hemp_type_number_integer) {
    return hemp_int_val((hemp_int) hemp_val_num(value));
}


HEMP_VALUE(hemp_type_number_boolean) {
    /* TODO: decide if this is the right thing to do */
    return hemp_val_num(value) == 0.0
        ? HempFalse
        : HempTrue;
}


HEMP_VALUE(hemp_type_number_compare) {
    hemp_num cmp = hemp_val_num(value);
    return  cmp < 0 ? HempBefore
        :   cmp > 0 ? HempAfter
        :             HempEqual; 
}

