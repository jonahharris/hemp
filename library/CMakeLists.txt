file(
    GLOB hemp_library_SOURCES 
    *.c 
    element/*.c
    element/command/*.c
    language/*.c
    scheme/*.c
    tag/*.c
    type/*.c
    viewer/*.c
)

add_library(
    hemp_library
    SHARED
    ${hemp_library_SOURCES}
)

set_target_properties(
    hemp_library
    PROPERTIES 
    OUTPUT_NAME hemp
)

install (
    TARGETS     hemp_library
    DESTINATION lib
)


