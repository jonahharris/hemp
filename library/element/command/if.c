#include <hemp/element.h>


HEMP_ELEMENT(hemp_element_command_if);
HEMP_PREFIX(hemp_element_command_if_prefix);
HEMP_VALUE(hemp_element_command_if_value);
HEMP_OUTPUT(hemp_element_command_if_text);
HEMP_OUTPUT(hemp_element_command_if_values);
HEMP_CLEANUP(hemp_element_command_if_cleanup);

HEMP_ELEMENT(hemp_element_command_else);
HEMP_POSTFIX(hemp_element_command_else_branch);

HEMP_ELEMENT(hemp_element_command_elsif);
HEMP_POSTFIX(hemp_element_command_elsif_branch);


/*--------------------------------------------------------------------------
 * if
 *--------------------------------------------------------------------------*/

HEMP_ELEMENT(hemp_element_command_if) {
    hemp_debug_call("hemp_element_command_if()\n");
    hemp_element_literal(hemp, element);
    element->parse_prefix    = &hemp_element_command_if_prefix;
    element->cleanup         = &hemp_element_command_if_cleanup;
    element->values          = &hemp_element_command_if_values;
    element->value           = &hemp_element_command_if_value;
    element->text            = &hemp_element_command_if_text;
    element->number          = &hemp_element_value_number;
    element->integer         = &hemp_element_value_integer;
    element->boolean         = &hemp_element_value_boolean;
    element->compare         = &hemp_element_value_compare;
    element->flags           = HEMP_BE_SOURCE;
    return element;
}


HEMP_PREFIX(hemp_element_command_if_prefix) {
    hemp_fragment fragment = *fragptr;
    hemp_element  element  = fragment->type;

    hemp_debug_msg("hemp_element_command_if_prefix()\n");

    HEMP_PREFIX_PRECEDENCE;

    /* skip past the keyword */
    hemp_advance(fragptr);
    hemp_set_flag(fragment, HEMP_BE_PREFIX);

    // TODO: look for option #fragment

    hemp_parse_lhs_expr(fragment);
    hemp_debug_msg("looking for block: %s\n", (*fragptr)->type->name);
    hemp_fragment block  = hemp_parse_rhs_body(fragment);
    hemp_debug_msg("looking for branch: %s\n", (*fragptr)->type->name);
    hemp_fragment branch = hemp_parse_branch(
        fragptr, scope, 0, 0, 
        fragment
    );

    if (branch) {
        hemp_debug_msg(
            "got a branch: %s (%s terminated)\n", 
            branch->type->name,
            hemp_has_flag(branch, HEMP_BE_TERMINATED) ? "is" : "not"
        );
        fragment->branch = branch;
        /* set the block to reference the branch for the test below */
        block = branch;
    }

    if (hemp_not_flag(block, HEMP_BE_TERMINATED)) {
        hemp_debug_msg("looking for terminator: %s\n", element->end);
        if (hemp_element_terminator_matches(*fragptr, element->end)) {
            hemp_debug_msg("found matching terminator for %s => %s\n", element->start, element->end);
            hemp_advance(fragptr);
        }
        else {
            HEMP_THROW_NOEND(fragment);
        }
    }
    else {
        hemp_debug_msg("branch/block is terminated\n");
    }

    return hemp_parse_postfix(
        fragptr, scope, precedence, 0,
        fragment
    );
}


HEMP_VALUE(hemp_element_command_if_value) {
    hemp_debug_call("hemp_element_command_if_value()\n");

    hemp_fragment   fragment = hemp_val_frag(value);
    hemp_value      test     = hemp_lhs(fragment);
    hemp_value      block    = hemp_rhs(fragment);
    hemp_value      truth    = hemp_obcall(test, boolean, context);
    hemp_value      result   = HempNothing;

    if (hemp_is_true(truth)) {
        result = hemp_obcall(block, value, context);
    }
    else if (fragment->branch) {
        // branch should probably be a value
        block  = hemp_frag_val(fragment->branch);
        result = hemp_obcall(block, value, context);
    }

    return result;
}


HEMP_OUTPUT(hemp_element_command_if_values) {
    hemp_debug_call("hemp_element_command_if_values()\n");

    hemp_fragment   fragment = hemp_val_frag(value);
    hemp_value      test     = hemp_lhs(fragment);
    hemp_value      block    = hemp_rhs(fragment);
    hemp_value      truth    = hemp_call(test, boolean, context);
    hemp_list       values;

    hemp_prepare_values(context, output, values);

    if (hemp_is_true(truth)) {
        hemp_obcall(block, values, context, output);
    }
    else if (fragment->branch) {
        block = hemp_frag_val(fragment->branch);
        hemp_obcall(block, values, context, output);
    }
    
    return output;
}


HEMP_OUTPUT(hemp_element_command_if_text) {
    hemp_debug_call("hemp_element_command_if_text()\n");

    hemp_fragment   fragment = hemp_val_frag(value);
    hemp_value      test     = hemp_lhs(fragment);
    hemp_value      block    = hemp_rhs(fragment);
    hemp_value      truth    = hemp_call(test, boolean, context);
    hemp_text       text;

    hemp_prepare_text(context, output, text);

    if (hemp_is_true(truth)) {
        hemp_obcall(block, text, context, output);
    }
    else if (fragment->branch) {
        block = hemp_frag_val(fragment->branch);
        hemp_obcall(block, text, context, output);
    }
    return output;
}


HEMP_CLEANUP(hemp_element_command_if_cleanup) {
    hemp_debug_call("hemp_element_command_if_cleanup(%p)\n", fragment);

}


/*--------------------------------------------------------------------------
 * elsif
 *--------------------------------------------------------------------------*/

HEMP_ELEMENT(hemp_element_command_elsif) {
    hemp_debug_call("hemp_element_command_elsif()\n");
    hemp_element_literal(hemp, element);
    element->parse_prefix   = NULL;
    element->parse_postfix  = NULL;
    element->parse_body     = NULL;
    element->parse_branch   = &hemp_element_command_else_branch;
    element->values         = &hemp_element_command_if_values;
    element->value          = &hemp_element_command_if_value;
    element->text           = &hemp_element_command_if_text;
    element->flags          = HEMP_BE_SOURCE;
    return element;
}


HEMP_POSTFIX(hemp_element_command_elsif_branch) {
    hemp_fragment fragment = *fragptr;
    hemp_fragment block, branch;

    hemp_debug_msg("hemp_element_command_elsif_branch()\n");

    /* skip the 'elsif' keyword     */
    hemp_advance(fragptr);

    /* parse an expression          */
    hemp_parse_lhs_expr(fragment);
    
    /* parse the body expr/block    */
    block  = hemp_parse_rhs_body(fragment);

    /* look for any other dangling branches */
    branch = hemp_parse_branch(fragptr, scope, 0, 0, fragment);

    if (branch) {
        fragment->branch = branch;
        /* set the block to reference the branch for the test below */
        block = branch;
    }

    /* Copy the block/branch's terminated flag into the current fragment 
     * so that the head of the branch (if) knows to look for a terminating
     * 'end' token or not
     */ 
    if (hemp_has_flag(block, HEMP_BE_TERMINATED))
        hemp_set_flag(fragment, HEMP_BE_TERMINATED);

    return fragment;
}


/*--------------------------------------------------------------------------
 * else
 *--------------------------------------------------------------------------*/

HEMP_ELEMENT(hemp_element_command_else) {
    hemp_debug_call("hemp_element_command_else()\n");
    hemp_element_literal(hemp, element);
    element->parse_prefix   = NULL;
    element->parse_postfix  = NULL;
    element->parse_body     = NULL;
    element->parse_branch   = &hemp_element_command_else_branch;
    element->flags          = HEMP_BE_SOURCE;
    return element;
}


HEMP_POSTFIX(hemp_element_command_else_branch) {
    hemp_fragment fragment = *fragptr;

    hemp_debug_call("hemp_element_command_else_branch()\n");

    /* Skip past the keyword then return the next expression or block */
    hemp_advance(fragptr);
    hemp_fragment body = hemp_parse_body(fragptr, scope, 0, 1);

    if (! body)
        HEMP_THROW_NOBODY(fragment);

    return body;
}