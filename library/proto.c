#include <hemp/proto.h>


HempProto
hemp_proto_init(
    HempProto proto
) {
    HEMP_ALLOCATE(proto);

    proto->name = hemp_hash_new();
    proto->item = hemp_list_new();
    proto->list = NULL;
    proto->hash = NULL;

    return proto;
}


void
hemp_proto_free(
    HempProto proto
) {
    hemp_list_each(proto->item, &hemp_list_each_free);

    if (proto->list)
        hemp_mem_free(proto->list);

    if (proto->hash)
        hemp_mem_free(proto->hash);

    hemp_hash_free(proto->name);
    hemp_list_free(proto->item);
    hemp_mem_free(proto);
}


HempBool
hemp_proto_add_item(
    HempProto   proto,
    HempString      name
) {
    hemp_debug_call("hemp_proto_add_item(%s)\n", name);

    if (hemp_hash_fetch_string(proto->name, name)) {
        hemp_fatal("duplicate parameter: %s", name);
        return HEMP_FALSE;
    }

    hemp_hash_store_string(
        proto->name,
        name, 
        "item"
    );
    hemp_list_push(
        proto->item,
        hemp_str_val(name)
    );

    return HEMP_TRUE;
}


HempBool
hemp_proto_add_list(
    HempProto   proto,
    HempString      name
) {
    hemp_debug_call("hemp_proto_add_list(%s)\n", name);

    if (hemp_hash_fetch_string(proto->name, name)) {
        hemp_fatal("duplicate parameter: %s", name);
        return HEMP_FALSE;
    }

    if (proto->list) {
        hemp_fatal(
            "Params have already got a list item: %s\n", 
            proto->list
        );
        return HEMP_FALSE;
    }

    hemp_hash_store_string(
        proto->name,
        name, 
        "list"
    );

    proto->list = name;
    return HEMP_TRUE;
}


HempBool
hemp_proto_add_hash(
    HempProto   proto,
    HempString      name
) {
    hemp_debug_call("hemp_proto_add_hash(%s)\n", name);

    if (hemp_hash_fetch_string(proto->name, name)) {
        hemp_fatal("duplicate parameter: %s", name);
        return HEMP_FALSE;
    }

    if (proto->hash) {
        hemp_fatal(
            "Params have already got a hash item: %s\n", 
            proto->hash
        );
        return HEMP_FALSE;
    }

    hemp_hash_store_string(
        proto->name,
        name, 
        "hash"
    );

    proto->hash = name;
    return HEMP_TRUE;
}


void 
hemp_proto_dump(
    HempProto proto
) {
    hemp_debug_msg("hemp proto at %p:\n", proto);

    if (proto->list) {
        hemp_debug("list item: %s\n", proto->list);
    }

    if (proto->hash) {
        hemp_debug("hash item: %s\n", proto->hash);
    }
    
    if (proto->item) {
        hemp_debug("%d items:\n", proto->item->length);
        HempText text = hemp_list_dump(proto->item);
        hemp_debug(text->string);
        hemp_text_free(text);
    }
    else {
        hemp_debug("no list\n");
    }

    if (proto->name) {
        hemp_debug("%d named items:\n", proto->name->size);
        HempText text = hemp_hash_dump(proto->name);
        hemp_debug(text->string);
        hemp_text_free(text);
    }
    else {
        hemp_debug("no named items\n");
    }

}
