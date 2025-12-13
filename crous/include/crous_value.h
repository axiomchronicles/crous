#ifndef CROUS_VALUE_H
#define CROUS_VALUE_H

#include "crous_types.h"

/* ============================================================================
   VALUE CONSTRUCTORS
   ============================================================================ */

crous_value* crous_value_new_null(void);
crous_value* crous_value_new_bool(int b);
crous_value* crous_value_new_int(int64_t v);
crous_value* crous_value_new_float(double d);
crous_value* crous_value_new_string(const char *data, size_t len);
crous_value* crous_value_new_bytes(const uint8_t *data, size_t len);
crous_value* crous_value_new_list(size_t capacity);
crous_value* crous_value_new_tuple(size_t capacity);
crous_value* crous_value_new_dict(size_t capacity);
crous_value* crous_value_new_tagged(uint32_t tag, crous_value *inner);

/* ============================================================================
   VALUE GETTERS
   ============================================================================ */

crous_type_t crous_value_get_type(const crous_value *v);
int crous_value_get_bool(const crous_value *v);
int64_t crous_value_get_int(const crous_value *v);
double crous_value_get_float(const crous_value *v);
const char* crous_value_get_string(const crous_value *v, size_t *out_len);
const uint8_t* crous_value_get_bytes(const crous_value *v, size_t *out_len);
uint32_t crous_value_get_tag(const crous_value *v);
const crous_value* crous_value_get_tagged_inner(const crous_value *v);

/* ============================================================================
   LIST/TUPLE OPERATIONS
   ============================================================================ */

size_t crous_value_list_size(const crous_value *v);
crous_value* crous_value_list_get(const crous_value *v, size_t index);
crous_err_t crous_value_list_set(crous_value *v, size_t index, crous_value *item);
crous_err_t crous_value_list_append(crous_value *v, crous_value *item);

/* ============================================================================
   DICT OPERATIONS
   ============================================================================ */

size_t crous_value_dict_size(const crous_value *v);
crous_value* crous_value_dict_get(const crous_value *v, const char *key);
crous_err_t crous_value_dict_set(crous_value *v, const char *key, crous_value *value);
crous_err_t crous_value_dict_set_binary(crous_value *v, const char *key, size_t key_len, crous_value *value);
const crous_dict_entry* crous_value_dict_get_entry(const crous_value *v, size_t index);

/* ============================================================================
   MEMORY MANAGEMENT
   ============================================================================ */

void crous_value_free_tree(crous_value *v);

#endif /* CROUS_VALUE_H */
