#include <hemp/hash.h>


static hemp_size_t 
hemp_primes[] = {
    8 + 3,
    16 + 3,
    32 + 5,
    64 + 3,
    128 + 3,
    256 + 27,
    512 + 9,
    1024 + 9,
    2048 + 5,
    4096 + 3,
    8192 + 27,
    16384 + 43,
    32768 + 3,
    65536 + 45,
    131072 + 29,
    262144 + 3,
    524288 + 21,
    1048576 + 7,
    2097152 + 17,
    4194304 + 15,
    8388608 + 9,
    16777216 + 43,
    33554432 + 35,
    67108864 + 15,
    134217728 + 29,
    268435456 + 3,
    536870912 + 11,
    1073741824 + 85,
};



HEMP_DO_INLINE static hemp_size_t
hemp_hash_wider(
    hemp_size_t n
) {
    int i, max = sizeof(hemp_primes) / sizeof(hemp_primes[0]);
    for (i = 0; i < max; i++) {
        if (hemp_primes[i] > n)
            return hemp_primes[i];
    }
    return hemp_primes[max - 1];
}


hemp_hash_p
hemp_hash_init() {
    hemp_size_t w;
    hemp_hash_p hash = (hemp_hash_p) hemp_mem_alloc(
        sizeof(struct hemp_hash_s)
    );
    
    if (! hash)
        hemp_mem_fail("hash");

    /* hemp_debug_mem("Allocated hash table at %p\n", table); */

    hash->width  = hemp_hash_wider(0);
    hash->size   = 0;
    hash->parent = NULL;
    hash->slots  = (hemp_slot_p *) hemp_mem_alloc(
        hash->width * sizeof(hemp_slot_p)
    );
    if (! hash->slots)
        hemp_mem_fail("hash slots");

    // TODO: hemp_mem_wipe(ptr,size) or perhaps hemp_mem_init(size) to 
    // allocate memory, alloc+ wie

    /* hemp_debug_mem("Allocated hash columns at %p\n", table->columns); */
    for(w = 0; w < hash->width; w++) {
        hash->slots[w] = NULL;
    }

    return hash;
}


void
hemp_hash_free(
    hemp_hash_p hash
) {
    hemp_slot_p slot, next;
    int i;

    /* hemp_debug_mem("Releasing hash at %p\n", table); */

    for(i = 0; i < hash->width; i++) {
        slot = hash->slots[i];

        while (slot) {
            next = slot->next;
            hemp_slot_free(slot);
            slot = next;
        }
    }
    hemp_mem_free(hash->slots);
    hemp_mem_free(hash);
}


hemp_size_t
hemp_hash_resize(
    hemp_hash_p hash
) {
    hemp_size_t width, wider, index, i;
    hemp_slots_p *slots, slots, next;
    
    width = hash->width;
    wider = hemp_hash_wider(width);

    if (width == wider)
        return width;   /* can't go any bigger */

    /* hemp_debug_mem("Resizing hash at %p from %d to %d\n", hash, width, wider); */

    slots = (hemp_slot_p *) hemp_mem_alloc(
        wider * sizeof(hemp_slot_p)
    );

    for (i = 0; i < hash->width; i++) {
        slot = hash->slots[i];
        while (slot) {
            next         = slot->next;
            index        = slot->index % wider;
            slot->next   = slots[index];
            slots[index] = slot;
            slot         = next;
        }
    }

    hemp_mem_free(hash->slots);
    hash->slots = slots;
    hash->width = wider;

    return wider;
}


hemp_slot_p
hemp_hash_store(
    hemp_hash_p     hash,
    hemp_cstr_p     name,
    hemp_value_p    value
) {
    hemp_slot_p slot;
    hemp_size_t index, column;

    if (hash->size / hash->width > HEMP_HASH_DENSITY)
        hemp_hash_resize(hash);

    index  = hemp_hash_function(key);
    column = index % hash->width;
    slot   = hash->slots[column];

    /* look at each entry in the column comparing the numerical hash value 
     * first, and then the more time consuming key string comparison only if 
     * the hash values are identical
     */
    while (slot && (index != slot->index) && strcmp(key, slot->key))
        slot = slot->next;

    if (slot) {
        /* TODO: free the value in the slot */
        slot->value = value;
    }
    else {
        slot = table->columns[column] = hemp_slot_init(
            hash, index, name, value, table->columns[column]
        );
        table->size++;
    }

    return slot;
}


hemp_value_t
hemp_hash_fetch(
    hemp_hash_p hash, 
    hemp_cstr_p name
) {
    hemp_slot_p slot = NULL;
    hemp_size_t index, column;
    index = hemp_hash_function(key);
    
    while (hash && ! slot) {
        column = index % hash->width;
        slot   = hash->slots[column];

        /* look for an entry in this hash table */
        while (slot && (index != slot->index) && strcmp(key, slot->key))
            slot = slot->next;

        /* or try the parent table, if there is one */
        hash = hash->parent;
    }
    
    return slot
        ? slot->value 
        : HempMissing;
}


void 
hemp_hash_attach(
    hemp_hash_p child, 
    hemp_hash_p parent
) {
    child->parent = parent;
}


void 
hemp_hash_detach(
    hemp_hash_p child
) {
    child->parent = NULL;
}


void
hemp_hash_each(
    hemp_hash_p      hash,
    hemp_hash_each_f func
) {
    hemp_size_t i;
    hemp_slot_p slot;
    hemp_pos_t n = 0;


    for (i = 0; i < hash->width; i++) {
        slot = hash->slots[i];

        while (entry) {
            if (! func(hash, n++, slot))
                break;
            slot = slot->next;
        }
    }
}


/*
hemp_cstr_t
hemp_hash_as_text(hemp_hash_t table) {
    hemp_size_t i, s = table->size;
    hemp_hash_entry_t entry;
    HEMP_VALUE  value;
    HEMP_TEXT   text = hemp_text_init(4 + table->size * 32);
    
    hemp_text_append_string(text, "{ ");

    for (i = 0; i < table->width; i++) {
        entry = table->columns[i];
        while (entry) {
            value = entry->value;
            hemp_text_append_string(text, entry->key);
            hemp_text_append_string(text, ":");
            hemp_text_append_text(text, (HEMP_TEXT) value->type->text(value));
            entry = entry->next;
            if (--s)
                hemp_text_append_string(text, ", ");
        }
    }

    hemp_text_append_string(text, " }");
    return text;
}

void 
hemp_hash_entry_print(hemp_hash_entry_t entry) {
    printf("%s => %s\n", entry->key, HEMP_AS_STRING(entry->value));
}
    
void 
hemp_hash_print(hemp_hash_t table) {
    hemp_hash_each(table, &hemp_hash_entry_print);
}

*/


/*---------------------------------------------------------------------
 * Default hash algorithm 
 * Can't remember where I got this from.  It's probably Perl's or 
 * maybe Python's?
 *---------------------------------------------------------------------*/

hemp_size_t
hemp_hash_function_default(
    register hemp_cstr_p s
) {
    register int c;
    register unsigned int val = 0;

    while ((c = *s++) != '\0') {
        val += c;
        val += (val << 10);
        val ^= (val >> 6);
    }
    val += (val << 3);
    val ^= (val >> 11);

    return val + (val << 15);
}


/*---------------------------------------------------------------------
 * Jenkins32 Hash algorithm 
 * See http://burtleburtle.net/bob/hash/evahash.html
 *---------------------------------------------------------------------*/

/* Define an initial seed value and some type aliases */
static hemp_size_t hemp_hash_jenkins32_seed = 42;
typedef  unsigned long int  u4;   
typedef  unsigned     char  u1;

/* The mixing step */
#define hemp_hash_jenkins32_mix(a,b,c) {    \
    a=a-b;  a=a-c;  a=a^(c>>13);            \
    b=b-c;  b=b-a;  b=b^(a<<8);             \
    c=c-a;  c=c-b;  c=c^(b>>13);            \
    a=a-b;  a=a-c;  a=a^(c>>12);            \
    b=b-c;  b=b-a;  b=b^(a<<16);            \
    c=c-a;  c=c-b;  c=c^(b>>5);             \
    a=a-b;  a=a-c;  a=a^(c>>3);             \
    b=b-c;  b=b-a;  b=b^(a<<10);            \
    c=c-a;  c=c-b;  c=c^(b>>15);            \
}

hemp_size_t
hemp_hash_function_jenkins32(
    register hemp_cstr_p key
) {
    register u4 a,b,c;              /* the internal state */
    hemp_size_t length, remain;     /* total length, remaining left to process */

   /* Set up the internal state */
    length = remain = strlen(key);
    a = b = 0x9e3779b9;             /* the golden ratio; an arbitrary value */
    c = hemp_hash_jenkins32_seed;   /* variable initialization of internal state */

    /* handle all but the last 11 bytes */
    while (remain >= 12) {
        a = a+(key[0]+((u4)key[1]<<8)+((u4)key[ 2]<<16)+((u4)key[ 3]<<24));
        b = b+(key[4]+((u4)key[5]<<8)+((u4)key[ 6]<<16)+((u4)key[ 7]<<24));
        c = c+(key[8]+((u4)key[9]<<8)+((u4)key[10]<<16)+((u4)key[11]<<24));
        hemp_hash_jenkins32_mix(a,b,c);
        key    = key + 12; 
        remain = remain - 12;
    }

    /* handle the last 11 bytes */
    c = c + length;
    switch(remain) {
        /* all the case statements fall through */
        case 11: c=c+((u4)key[10]<<24);
        case 10: c=c+((u4)key[9]<<16);
        case 9 : c=c+((u4)key[8]<<8);
        /* the first byte of c is reserved for the length */
        case 8 : b=b+((u4)key[7]<<24);
        case 7 : b=b+((u4)key[6]<<16);
        case 6 : b=b+((u4)key[5]<<8);
        case 5 : b=b+key[4];
        case 4 : a=a+((u4)key[3]<<24);
        case 3 : a=a+((u4)key[2]<<16);
        case 2 : a=a+((u4)key[1]<<8);
        case 1 : a=a+key[0];
        /* case 0: nothing left to add */
    }
    hemp_hash_jenkins32_mix(a,b,c);

    return c;
}
