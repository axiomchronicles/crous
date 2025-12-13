#include "../include/crous_value.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
   CONSTRUCTORS
   ============================================================================ */

crous_value* crous_value_new_null(void) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_NULL;
    return v;
}

crous_value* crous_value_new_bool(int b) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_BOOL;
    v->data.b = b ? 1 : 0;
    return v;
}

crous_value* crous_value_new_int(int64_t v_val) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_INT;
    v->data.i = v_val;
    return v;
}

crous_value* crous_value_new_float(double d) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_FLOAT;
    v->data.f = d;
    return v;
}

crous_value* crous_value_new_string(const char *data, size_t len) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_STRING;
    v->data.s.data = malloc(len);
    if (!v->data.s.data) {
        free(v);
        return NULL;
    }
    if (len > 0) memcpy(v->data.s.data, data, len);
    v->data.s.len = len;
    return v;
}

crous_value* crous_value_new_bytes(const uint8_t *data, size_t len) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_BYTES;
    v->data.bytes.data = malloc(len);
    if (!v->data.bytes.data) {
        free(v);
        return NULL;
    }
    if (len > 0) memcpy(v->data.bytes.data, data, len);
    v->data.bytes.len = len;
    return v;
}

crous_value* crous_value_new_list(size_t capacity) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_LIST;
    v->data.list.items = capacity > 0 ? malloc(capacity * sizeof(crous_value *)) : NULL;
    if (capacity > 0 && !v->data.list.items) {
        free(v);
        return NULL;
    }
    v->data.list.len = 0;
    v->data.list.cap = capacity;
    return v;
}

crous_value* crous_value_new_tuple(size_t capacity) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_TUPLE;
    v->data.list.items = capacity > 0 ? malloc(capacity * sizeof(crous_value *)) : NULL;
    if (capacity > 0 && !v->data.list.items) {
        free(v);
        return NULL;
    }
    v->data.list.len = 0;
    v->data.list.cap = capacity;
    return v;
}

crous_value* crous_value_new_dict(size_t capacity) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_DICT;
    v->data.dict.entries = capacity > 0 ? malloc(capacity * sizeof(crous_dict_entry)) : NULL;
    if (capacity > 0 && !v->data.dict.entries) {
        free(v);
        return NULL;
    }
    v->data.dict.len = 0;
    v->data.dict.cap = capacity;
    return v;
}

crous_value* crous_value_new_tagged(uint32_t tag, crous_value *inner) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_TAGGED;
    v->data.tagged.tag = tag;
    v->data.tagged.value = inner;
    return v;
}

/* ============================================================================
   GETTERS
   ============================================================================ */

crous_type_t crous_value_get_type(const crous_value *v) {
    return v ? v->type : CROUS_TYPE_NULL;
}

int crous_value_get_bool(const crous_value *v) {
    return v && v->type == CROUS_TYPE_BOOL ? v->data.b : 0;
}

int64_t crous_value_get_int(const crous_value *v) {
    return v && v->type == CROUS_TYPE_INT ? v->data.i : 0;
}

double crous_value_get_float(const crous_value *v) {
    return v && v->type == CROUS_TYPE_FLOAT ? v->data.f : 0.0;
}

const char* crous_value_get_string(const crous_value *v, size_t *out_len) {
    if (v && v->type == CROUS_TYPE_STRING) {
        if (out_len) *out_len = v->data.s.len;
        return (const char *)v->data.s.data;
    }
    if (out_len) *out_len = 0;
    return NULL;
}

const uint8_t* crous_value_get_bytes(const crous_value *v, size_t *out_len) {
    if (v && v->type == CROUS_TYPE_BYTES) {
        if (out_len) *out_len = v->data.bytes.len;
        return v->data.bytes.data;
    }
    if (out_len) *out_len = 0;
    return NULL;
}

uint32_t crous_value_get_tag(const crous_value *v) {
    if (v && v->type == CROUS_TYPE_TAGGED) return v->data.tagged.tag;
    return 0;
}

const crous_value* crous_value_get_tagged_inner(const crous_value *v) {
    if (v && v->type == CROUS_TYPE_TAGGED) return v->data.tagged.value;
    return NULL;
}

/* ============================================================================
   LIST/TUPLE OPERATIONS
   ============================================================================ */

size_t crous_value_list_size(const crous_value *v) {
    return (v && (v->type == CROUS_TYPE_LIST || v->type == CROUS_TYPE_TUPLE)) ? v->data.list.len : 0;
}

crous_value* crous_value_list_get(const crous_value *v, size_t index) {
    if (!v || (v->type != CROUS_TYPE_LIST && v->type != CROUS_TYPE_TUPLE)) return NULL;
    if (index >= v->data.list.len) return NULL;
    return v->data.list.items[index];
}

crous_err_t crous_value_list_set(crous_value *v, size_t index, crous_value *item) {
    if (!v || (v->type != CROUS_TYPE_LIST && v->type != CROUS_TYPE_TUPLE))
        return CROUS_ERR_INVALID_TYPE;
    if (index >= v->data.list.len) return CROUS_ERR_INVALID_TYPE;
    v->data.list.items[index] = item;
    return CROUS_OK;
}

crous_err_t crous_value_list_append(crous_value *v, crous_value *item) {
    if (!v || (v->type != CROUS_TYPE_LIST && v->type != CROUS_TYPE_TUPLE))
        return CROUS_ERR_INVALID_TYPE;
    
    size_t new_len = v->data.list.len + 1;
    if (new_len < v->data.list.len) return CROUS_ERR_OVERFLOW;
    
    if (new_len > v->data.list.cap) {
        size_t new_cap = (v->data.list.cap == 0) ? 8 : v->data.list.cap * 2;
        while (new_cap < new_len) new_cap *= 2;
        
        crous_value **new_items = realloc(v->data.list.items, new_cap * sizeof(crous_value *));
        if (!new_items) return CROUS_ERR_OOM;
        v->data.list.items = new_items;
        v->data.list.cap = new_cap;
    }
    
    v->data.list.items[v->data.list.len] = item;
    v->data.list.len = new_len;
    return CROUS_OK;
}

/* ============================================================================
   DICT OPERATIONS
   ============================================================================ */

size_t crous_value_dict_size(const crous_value *v) {
    return (v && v->type == CROUS_TYPE_DICT) ? v->data.dict.len : 0;
}

crous_value* crous_value_dict_get(const crous_value *v, const char *key) {
    if (!v || v->type != CROUS_TYPE_DICT || !key) return NULL;
    
    for (size_t i = 0; i < v->data.dict.len; i++) {
        if (strcmp(v->data.dict.entries[i].key, key) == 0)
            return v->data.dict.entries[i].value;
    }
    return NULL;
}

/* Internal function that handles both null-terminated and binary-safe keys */
static crous_err_t crous_value_dict_set_internal(crous_value *v, const char *key, size_t key_len, crous_value *value) {
    if (!v || v->type != CROUS_TYPE_DICT || !key)
        return CROUS_ERR_INVALID_TYPE;
    
    /* Check if key already exists */
    for (size_t i = 0; i < v->data.dict.len; i++) {
        if (v->data.dict.entries[i].key_len == key_len &&
            memcmp(v->data.dict.entries[i].key, key, key_len) == 0) {
            v->data.dict.entries[i].value = value;
            return CROUS_OK;
        }
    }
    
    /* Add new entry */
    size_t new_len = v->data.dict.len + 1;
    if (new_len < v->data.dict.len) return CROUS_ERR_OVERFLOW;
    
    if (new_len > v->data.dict.cap) {
        size_t new_cap = (v->data.dict.cap == 0) ? 8 : v->data.dict.cap * 2;
        while (new_cap < new_len) new_cap *= 2;
        
        crous_dict_entry *new_entries = realloc(v->data.dict.entries, new_cap * sizeof(crous_dict_entry));
        if (!new_entries) return CROUS_ERR_OOM;
        v->data.dict.entries = new_entries;
        v->data.dict.cap = new_cap;
    }
    
    char *key_copy = malloc(key_len);
    if (!key_copy) return CROUS_ERR_OOM;
    memcpy(key_copy, key, key_len);
    
    v->data.dict.entries[v->data.dict.len].key = key_copy;
    v->data.dict.entries[v->data.dict.len].key_len = key_len;
    v->data.dict.entries[v->data.dict.len].value = value;
    v->data.dict.len = new_len;
    
    return CROUS_OK;
}

/* Public API - null-terminated key */
crous_err_t crous_value_dict_set(crous_value *v, const char *key, crous_value *value) {
    if (!key) return CROUS_ERR_INVALID_TYPE;
    return crous_value_dict_set_internal(v, key, strlen(key), value);
}

/* Public API - binary-safe key with explicit length */
crous_err_t crous_value_dict_set_binary(crous_value *v, const char *key, size_t key_len, crous_value *value) {
    return crous_value_dict_set_internal(v, key, key_len, value);
}

const crous_dict_entry* crous_value_dict_get_entry(const crous_value *v, size_t index) {
    if (!v || v->type != CROUS_TYPE_DICT || index >= v->data.dict.len) return NULL;
    return &v->data.dict.entries[index];
}

/* ============================================================================
   MEMORY MANAGEMENT
   ============================================================================ */

void crous_value_free_tree(crous_value *v) {
    if (!v) return;
    
    switch (v->type) {
        case CROUS_TYPE_STRING:
            free(v->data.s.data);
            break;
        case CROUS_TYPE_BYTES:
            free(v->data.bytes.data);
            break;
        case CROUS_TYPE_LIST:
        case CROUS_TYPE_TUPLE:
            for (size_t i = 0; i < v->data.list.len; i++) {
                crous_value_free_tree(v->data.list.items[i]);
            }
            free(v->data.list.items);
            break;
        case CROUS_TYPE_DICT:
            for (size_t i = 0; i < v->data.dict.len; i++) {
                free(v->data.dict.entries[i].key);
                crous_value_free_tree(v->data.dict.entries[i].value);
            }
            free(v->data.dict.entries);
            break;
        case CROUS_TYPE_TAGGED:
            crous_value_free_tree(v->data.tagged.value);
            break;
        default:
            break;
    }
    
    free(v);
}
