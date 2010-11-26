#include <hemp/core.h>
#include <hemp/scanner.h>
//#include <hemp/source.h>


hemp_error
hemp_error_new(
    hemp_errno number
) {
    hemp_error error;
    
    if (number < 0 || number >= HEMP_ERROR_MAX) 
        hemp_fatal("Invalid error number: %d", number);

    HEMP_ALLOCATE(error);

    error->number   = number;
    error->message  = NULL;
    error->parent   = NULL;
    error->scan_pos = NULL;

    return error;
}


hemp_error
hemp_error_init(
    hemp_errno  number,
    hemp_string message
) {
    hemp_error error = hemp_error_new(number);

    /* We use strdup() to deliberately avoid the memory trace that is wrapped 
     * around mem_string_copy() and friend when memory debugging is enabled.
     * This is because the vasprintf() functions called below also avoid our
     * basic memory allocation tracking.  So we call free() not hemp_mem_free()
     * in hemp_error_free() below.
     */
    error->message = strdup(message);

    if (! error->message)
        hemp_mem_fail("error message");
    
    return error;
}


hemp_error
hemp_error_initf(
    hemp_errno  number,
    hemp_string format,
    ...
) {
    hemp_error error = hemp_error_new(number);

    va_list args;
    va_start(args, format);
    vasprintf(&error->message, format, args);
    va_end(args);
    return error;
}


hemp_error
hemp_error_initfv(
    hemp_errno  number,
    hemp_string format,
    va_list     args
) {
    hemp_error error = hemp_error_new(number);
    vasprintf(&error->message, format, args);
    return error;
}


hemp_error
hemp_error_scan_pos(
    hemp_error    error,
    hemp_scan_pos scan_pos
) {
    error->scan_pos = scan_pos;
    return error;
}


void
hemp_error_free(
    hemp_error error
) {
    /* memory allocated by vasprintf() so don't use hemp_mem_free() because 
     * we're not tracking it and it'll blow a fuse when debugging memory
     */
    if (error->message)
        free(error->message);

    if (error->scan_pos)
        hemp_scan_pos_free(error->scan_pos);

    hemp_mem_free(error);
}

