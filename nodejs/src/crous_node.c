#include <node_api.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "crous.h"

/* ============================================================================
   ERROR HANDLING
   ============================================================================ */

static napi_value throw_crous_error(napi_env env, const char* msg) {
    napi_throw_error(env, "CrousError", msg);
    return NULL;
}

static napi_value throw_encode_error(napi_env env, const char* msg) {
    napi_throw_error(env, "CrousEncodeError", msg);
    return NULL;
}

static napi_value throw_decode_error(napi_env env, const char* msg) {
    napi_throw_error(env, "CrousDecodeError", msg);
    return NULL;
}

/* ============================================================================
   CUSTOM SERIALIZER/DECODER REGISTRIES
   ============================================================================ */

/* Global references for custom serializers and decoders */
static napi_ref custom_serializers_ref = NULL;
static napi_ref custom_decoders_ref = NULL;
static napi_ref type_to_tag_ref = NULL;
static uint32_t next_custom_tag = 100;

/* ============================================================================
   CONVERSION: NODE.JS VALUE -> CROUS VALUE
   ============================================================================ */

static crous_value* napi_to_crous(napi_env env, napi_value value, napi_value default_func, crous_err_t *err);

static crous_value* try_custom_serializer(napi_env env, napi_value obj, napi_value default_func, 
                                          crous_err_t *err, int *handled) {
    *handled = 0;
    *err = CROUS_OK;
    
    // Get custom serializers map
    napi_value serializers;
    if (custom_serializers_ref == NULL) {
        return NULL;
    }
    
    napi_status status = napi_get_reference_value(env, custom_serializers_ref, &serializers);
    if (status != napi_ok || serializers == NULL) {
        return NULL;
    }
    
    // Get constructor of object
    napi_value constructor;
    status = napi_get_named_property(env, obj, "constructor", &constructor);
    if (status != napi_ok) {
        return NULL;
    }
    
    // Look up serializer
    napi_value serializer;
    bool has_property;
    status = napi_has_property(env, serializers, constructor, &has_property);
    if (status != napi_ok || !has_property) {
        // Try default_func if provided
        if (default_func != NULL) {
            napi_valuetype type;
            napi_typeof(env, default_func, &type);
            if (type == napi_function) {
                serializer = default_func;
                *handled = 1;
            } else {
                return NULL;
            }
        } else {
            return NULL;
        }
    } else {
        status = napi_get_property(env, serializers, constructor, &serializer);
        if (status != napi_ok) {
            return NULL;
        }
        *handled = 1;
    }
    
    // Call serializer
    napi_value global;
    napi_get_global(env, &global);
    napi_value result;
    status = napi_call_function(env, global, serializer, 1, &obj, &result);
    if (status != napi_ok) {
        *err = CROUS_ERR_ENCODE;
        return NULL;
    }
    
    // Get tag for this type (default 100)
    uint32_t tag = 100;
    if (type_to_tag_ref != NULL) {
        napi_value tag_map;
        status = napi_get_reference_value(env, type_to_tag_ref, &tag_map);
        if (status == napi_ok) {
            napi_value tag_value;
            bool has_tag;
            status = napi_has_property(env, tag_map, constructor, &has_tag);
            if (status == napi_ok && has_tag) {
                status = napi_get_property(env, tag_map, constructor, &tag_value);
                if (status == napi_ok) {
                    napi_get_value_uint32(env, tag_value, &tag);
                }
            }
        }
    }
    
    // Convert result to crous_value
    crous_value *inner = napi_to_crous(env, result, NULL, err);
    if (*err != CROUS_OK || inner == NULL) {
        return NULL;
    }
    
    // Wrap in tagged value
    crous_value *tagged = crous_value_new_tagged(tag, inner);
    if (!tagged) {
        crous_value_free_tree(inner);
        *err = CROUS_ERR_OOM;
        return NULL;
    }
    
    return tagged;
}

static crous_value* napi_to_crous(napi_env env, napi_value value, napi_value default_func, crous_err_t *err) {
    *err = CROUS_OK;
    napi_status status;
    napi_valuetype type;
    
    status = napi_typeof(env, value, &type);
    if (status != napi_ok) {
        *err = CROUS_ERR_ENCODE;
        return NULL;
    }
    
    switch (type) {
        case napi_undefined:
        case napi_null:
            return crous_value_new_null();
        
        case napi_boolean: {
            bool val;
            status = napi_get_value_bool(env, value, &val);
            if (status != napi_ok) {
                *err = CROUS_ERR_ENCODE;
                return NULL;
            }
            return crous_value_new_bool(val ? 1 : 0);
        }
        
        case napi_number: {
            double num;
            status = napi_get_value_double(env, value, &num);
            if (status != napi_ok) {
                *err = CROUS_ERR_ENCODE;
                return NULL;
            }
            
            // Check if it's an integer
            if (num == (int64_t)num && num >= -9223372036854775807LL && num <= 9223372036854775807LL) {
                return crous_value_new_int((int64_t)num);
            } else {
                return crous_value_new_float(num);
            }
        }
        
        case napi_string: {
            size_t str_len;
            status = napi_get_value_string_utf8(env, value, NULL, 0, &str_len);
            if (status != napi_ok) {
                *err = CROUS_ERR_ENCODE;
                return NULL;
            }
            
            char *buf = (char*)malloc(str_len + 1);
            if (!buf) {
                *err = CROUS_ERR_OOM;
                return NULL;
            }
            
            size_t copied;
            status = napi_get_value_string_utf8(env, value, buf, str_len + 1, &copied);
            if (status != napi_ok) {
                free(buf);
                *err = CROUS_ERR_ENCODE;
                return NULL;
            }
            
            crous_value *result = crous_value_new_string(buf, copied);
            free(buf);
            return result;
        }
        
        case napi_object: {
            // Check if it's a Buffer
            bool is_buffer;
            status = napi_is_buffer(env, value, &is_buffer);
            if (status == napi_ok && is_buffer) {
                void *data;
                size_t length;
                status = napi_get_buffer_info(env, value, &data, &length);
                if (status != napi_ok) {
                    *err = CROUS_ERR_ENCODE;
                    return NULL;
                }
                return crous_value_new_bytes((const uint8_t*)data, length);
            }
            
            // Check if it's an Array
            bool is_array;
            status = napi_is_array(env, value, &is_array);
            if (status == napi_ok && is_array) {
                uint32_t length;
                status = napi_get_array_length(env, value, &length);
                if (status != napi_ok) {
                    *err = CROUS_ERR_ENCODE;
                    return NULL;
                }
                
                crous_value *list = crous_value_new_list((size_t)length);
                if (!list) {
                    *err = CROUS_ERR_OOM;
                    return NULL;
                }
                
                for (uint32_t i = 0; i < length; i++) {
                    napi_value element;
                    status = napi_get_element(env, value, i, &element);
                    if (status != napi_ok) {
                        crous_value_free_tree(list);
                        *err = CROUS_ERR_ENCODE;
                        return NULL;
                    }
                    
                    crous_value *item = napi_to_crous(env, element, default_func, err);
                    if (*err != CROUS_OK) {
                        crous_value_free_tree(list);
                        return NULL;
                    }
                    
                    if (crous_value_list_append(list, item) != CROUS_OK) {
                        crous_value_free_tree(list);
                        crous_value_free_tree(item);
                        *err = CROUS_ERR_OOM;
                        return NULL;
                    }
                }
                
                return list;
            }
            
            // Check if it's a Set
            napi_value set_constructor;
            napi_value global;
            napi_get_global(env, &global);
            napi_get_named_property(env, global, "Set", &set_constructor);
            
            bool is_set = false;
            napi_instanceof(env, value, set_constructor, &is_set);
            
            if (is_set) {
                int handled = 0;
                crous_value *result = try_custom_serializer(env, value, default_func, err, &handled);
                if (handled) return result;
                
                // Convert Set to Array
                napi_value array_from;
                napi_value array_constructor;
                napi_get_named_property(env, global, "Array", &array_constructor);
                napi_get_named_property(env, array_constructor, "from", &array_from);
                
                napi_value as_array;
                napi_call_function(env, global, array_from, 1, &value, &as_array);
                
                crous_value *list_val = napi_to_crous(env, as_array, default_func, err);
                if (*err != CROUS_OK) return NULL;
                
                crous_value *tagged = crous_value_new_tagged(90, list_val);
                if (!tagged) {
                    crous_value_free_tree(list_val);
                    *err = CROUS_ERR_OOM;
                    return NULL;
                }
                return tagged;
            }
            
            // Regular object (dictionary)
            napi_value property_names;
            status = napi_get_property_names(env, value, &property_names);
            if (status != napi_ok) {
                *err = CROUS_ERR_ENCODE;
                return NULL;
            }
            
            uint32_t prop_count;
            status = napi_get_array_length(env, property_names, &prop_count);
            if (status != napi_ok) {
                *err = CROUS_ERR_ENCODE;
                return NULL;
            }
            
            crous_value *dict = crous_value_new_dict((size_t)prop_count);
            if (!dict) {
                *err = CROUS_ERR_OOM;
                return NULL;
            }
            
            for (uint32_t i = 0; i < prop_count; i++) {
                napi_value prop_name;
                status = napi_get_element(env, property_names, i, &prop_name);
                if (status != napi_ok) {
                    crous_value_free_tree(dict);
                    *err = CROUS_ERR_ENCODE;
                    return NULL;
                }
                
                size_t key_len;
                status = napi_get_value_string_utf8(env, prop_name, NULL, 0, &key_len);
                if (status != napi_ok) {
                    crous_value_free_tree(dict);
                    *err = CROUS_ERR_ENCODE;
                    return NULL;
                }
                
                char *key_buf = (char*)malloc(key_len + 1);
                if (!key_buf) {
                    crous_value_free_tree(dict);
                    *err = CROUS_ERR_OOM;
                    return NULL;
                }
                
                size_t copied;
                status = napi_get_value_string_utf8(env, prop_name, key_buf, key_len + 1, &copied);
                if (status != napi_ok) {
                    free(key_buf);
                    crous_value_free_tree(dict);
                    *err = CROUS_ERR_ENCODE;
                    return NULL;
                }
                
                napi_value prop_value;
                status = napi_get_property(env, value, prop_name, &prop_value);
                if (status != napi_ok) {
                    free(key_buf);
                    crous_value_free_tree(dict);
                    *err = CROUS_ERR_ENCODE;
                    return NULL;
                }
                
                crous_value *cval = napi_to_crous(env, prop_value, default_func, err);
                if (*err != CROUS_OK) {
                    free(key_buf);
                    crous_value_free_tree(dict);
                    return NULL;
                }
                
                if (crous_value_dict_set_binary(dict, key_buf, copied, cval) != CROUS_OK) {
                    free(key_buf);
                    crous_value_free_tree(dict);
                    crous_value_free_tree(cval);
                    *err = CROUS_ERR_OOM;
                    return NULL;
                }
                
                free(key_buf);
            }
            
            return dict;
        }
        
        default: {
            // Try custom serializer
            int handled = 0;
            crous_value *result = try_custom_serializer(env, value, default_func, err, &handled);
            if (handled) return result;
            
            *err = CROUS_ERR_INVALID_TYPE;
            return NULL;
        }
    }
}

/* ============================================================================
   CONVERSION: CROUS VALUE -> NODE.JS VALUE
   ============================================================================ */

static napi_value crous_to_napi(napi_env env, const crous_value *v, napi_value object_hook) {
    if (!v) {
        napi_value result;
        napi_get_null(env, &result);
        return result;
    }
    
    napi_status status;
    napi_value result;
    
    switch (crous_value_get_type(v)) {
        case CROUS_TYPE_NULL:
            napi_get_null(env, &result);
            return result;
        
        case CROUS_TYPE_BOOL:
            napi_get_boolean(env, crous_value_get_bool(v), &result);
            return result;
        
        case CROUS_TYPE_INT:
            napi_create_int64(env, crous_value_get_int(v), &result);
            return result;
        
        case CROUS_TYPE_FLOAT:
            napi_create_double(env, crous_value_get_float(v), &result);
            return result;
        
        case CROUS_TYPE_STRING: {
            size_t len;
            const char *data = crous_value_get_string(v, &len);
            napi_create_string_utf8(env, data, len, &result);
            return result;
        }
        
        case CROUS_TYPE_BYTES: {
            size_t len;
            const uint8_t *data = crous_value_get_bytes(v, &len);
            void *buffer_data;
            status = napi_create_buffer_copy(env, len, data, &buffer_data, &result);
            if (status != napi_ok) {
                napi_get_null(env, &result);
            }
            return result;
        }
        
        case CROUS_TYPE_LIST: {
            size_t size = crous_value_list_size(v);
            napi_create_array_with_length(env, size, &result);
            
            for (size_t i = 0; i < size; i++) {
                napi_value element = crous_to_napi(env, crous_value_list_get(v, i), object_hook);
                napi_set_element(env, result, (uint32_t)i, element);
            }
            
            return result;
        }
        
        case CROUS_TYPE_TUPLE: {
            // Tuples are represented as arrays in JavaScript
            size_t size = crous_value_list_size(v);
            napi_create_array_with_length(env, size, &result);
            
            for (size_t i = 0; i < size; i++) {
                napi_value element = crous_to_napi(env, crous_value_list_get(v, i), object_hook);
                napi_set_element(env, result, (uint32_t)i, element);
            }
            
            return result;
        }
        
        case CROUS_TYPE_DICT: {
            napi_create_object(env, &result);
            
            size_t size = crous_value_dict_size(v);
            for (size_t i = 0; i < size; i++) {
                const crous_dict_entry *entry = crous_value_dict_get_entry(v, i);
                if (!entry) continue;
                
                napi_value key;
                napi_create_string_utf8(env, entry->key, entry->key_len, &key);
                
                napi_value val = crous_to_napi(env, entry->value, object_hook);
                napi_set_property(env, result, key, val);
            }
            
            // Apply object_hook if provided
            if (object_hook != NULL) {
                napi_valuetype hook_type;
                napi_typeof(env, object_hook, &hook_type);
                if (hook_type == napi_function) {
                    napi_value global;
                    napi_get_global(env, &global);
                    napi_value hooked_result;
                    status = napi_call_function(env, global, object_hook, 1, &result, &hooked_result);
                    if (status == napi_ok) {
                        return hooked_result;
                    }
                }
            }
            
            return result;
        }
        
        case CROUS_TYPE_TAGGED: {
            uint32_t tag = crous_value_get_tag(v);
            const crous_value *inner = crous_value_get_tagged_inner(v);
            
            // Check for built-in tags
            if (tag == 90) {
                // Set
                napi_value array = crous_to_napi(env, inner, object_hook);
                napi_value global;
                napi_get_global(env, &global);
                napi_value set_constructor;
                napi_get_named_property(env, global, "Set", &set_constructor);
                napi_new_instance(env, set_constructor, 1, &array, &result);
                return result;
            }
            
            // Check for custom decoder
            if (custom_decoders_ref != NULL) {
                napi_value decoders;
                status = napi_get_reference_value(env, custom_decoders_ref, &decoders);
                if (status == napi_ok) {
                    napi_value tag_key;
                    napi_create_uint32(env, tag, &tag_key);
                    
                    bool has_decoder;
                    status = napi_has_property(env, decoders, tag_key, &has_decoder);
                    if (status == napi_ok && has_decoder) {
                        napi_value decoder;
                        status = napi_get_property(env, decoders, tag_key, &decoder);
                        if (status == napi_ok) {
                            napi_value inner_js = crous_to_napi(env, inner, object_hook);
                            napi_value global;
                            napi_get_global(env, &global);
                            status = napi_call_function(env, global, decoder, 1, &inner_js, &result);
                            if (status == napi_ok) {
                                return result;
                            }
                        }
                    }
                }
            }
            
            // No decoder found, return inner value
            return crous_to_napi(env, inner, object_hook);
        }
        
        default:
            napi_get_null(env, &result);
            return result;
    }
}

/* ============================================================================
   EXPORTED FUNCTIONS
   ============================================================================ */

static napi_value dumps(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    
    if (status != napi_ok || argc < 1) {
        napi_throw_type_error(env, NULL, "Wrong number of arguments");
        return NULL;
    }
    
    napi_value default_func = (argc > 1) ? args[1] : NULL;
    
    crous_err_t err;
    crous_value *value = napi_to_crous(env, args[0], default_func, &err);
    if (!value || err != CROUS_OK) {
        return throw_encode_error(env, crous_err_str(err));
    }
    
    uint8_t *buf = NULL;
    size_t size = 0;
    err = crous_encode(value, &buf, &size);
    crous_value_free_tree(value);
    
    if (err != CROUS_OK) {
        free(buf);
        return throw_encode_error(env, crous_err_str(err));
    }
    
    napi_value result;
    void *result_data;
    status = napi_create_buffer_copy(env, size, buf, &result_data, &result);
    free(buf);
    
    if (status != napi_ok) {
        return throw_encode_error(env, "Failed to create buffer");
    }
    
    return result;
}

static napi_value loads(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    
    if (status != napi_ok || argc < 1) {
        napi_throw_type_error(env, NULL, "Wrong number of arguments");
        return NULL;
    }
    
    bool is_buffer;
    status = napi_is_buffer(env, args[0], &is_buffer);
    if (status != napi_ok || !is_buffer) {
        napi_throw_type_error(env, NULL, "First argument must be a Buffer");
        return NULL;
    }
    
    void *data;
    size_t length;
    status = napi_get_buffer_info(env, args[0], &data, &length);
    if (status != napi_ok) {
        return throw_decode_error(env, "Failed to get buffer data");
    }
    
    napi_value object_hook = (argc > 1) ? args[1] : NULL;
    
    crous_value *value = NULL;
    crous_err_t err = crous_decode((const uint8_t*)data, length, &value);
    
    if (err != CROUS_OK) {
        if (value) crous_value_free_tree(value);
        return throw_decode_error(env, crous_err_str(err));
    }
    
    napi_value result = crous_to_napi(env, value, object_hook);
    crous_value_free_tree(value);
    
    return result;
}

static napi_value register_serializer(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    
    if (status != napi_ok || argc < 2) {
        napi_throw_type_error(env, NULL, "Expected 2 arguments: type and function");
        return NULL;
    }
    
    napi_valuetype arg1_type;
    napi_typeof(env, args[1], &arg1_type);
    if (arg1_type != napi_function) {
        napi_throw_type_error(env, NULL, "Second argument must be a function");
        return NULL;
    }
    
    // Initialize serializers map if needed
    if (custom_serializers_ref == NULL) {
        napi_value serializers;
        napi_create_object(env, &serializers);
        napi_create_reference(env, serializers, 1, &custom_serializers_ref);
    }
    
    napi_value serializers;
    napi_get_reference_value(env, custom_serializers_ref, &serializers);
    napi_set_property(env, serializers, args[0], args[1]);
    
    // Initialize type_to_tag if needed
    if (type_to_tag_ref == NULL) {
        napi_value tag_map;
        napi_create_object(env, &tag_map);
        napi_create_reference(env, tag_map, 1, &type_to_tag_ref);
    }
    
    // Assign tag
    napi_value tag_map;
    napi_get_reference_value(env, type_to_tag_ref, &tag_map);
    napi_value tag_value;
    napi_create_uint32(env, next_custom_tag++, &tag_value);
    napi_set_property(env, tag_map, args[0], tag_value);
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value unregister_serializer(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    
    if (status != napi_ok || argc < 1) {
        napi_throw_type_error(env, NULL, "Expected 1 argument");
        return NULL;
    }
    
    if (custom_serializers_ref != NULL) {
        napi_value serializers;
        napi_get_reference_value(env, custom_serializers_ref, &serializers);
        bool deleted;
        napi_delete_property(env, serializers, args[0], &deleted);
    }
    
    if (type_to_tag_ref != NULL) {
        napi_value tag_map;
        napi_get_reference_value(env, type_to_tag_ref, &tag_map);
        bool deleted;
        napi_delete_property(env, tag_map, args[0], &deleted);
    }
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value register_decoder(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    
    if (status != napi_ok || argc < 2) {
        napi_throw_type_error(env, NULL, "Expected 2 arguments: tag and function");
        return NULL;
    }
    
    napi_valuetype arg1_type;
    napi_typeof(env, args[1], &arg1_type);
    if (arg1_type != napi_function) {
        napi_throw_type_error(env, NULL, "Second argument must be a function");
        return NULL;
    }
    
    // Initialize decoders map if needed
    if (custom_decoders_ref == NULL) {
        napi_value decoders;
        napi_create_object(env, &decoders);
        napi_create_reference(env, decoders, 1, &custom_decoders_ref);
    }
    
    napi_value decoders;
    napi_get_reference_value(env, custom_decoders_ref, &decoders);
    napi_set_property(env, decoders, args[0], args[1]);
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value unregister_decoder(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    
    if (status != napi_ok || argc < 1) {
        napi_throw_type_error(env, NULL, "Expected 1 argument");
        return NULL;
    }
    
    if (custom_decoders_ref != NULL) {
        napi_value decoders;
        napi_get_reference_value(env, custom_decoders_ref, &decoders);
        bool deleted;
        napi_delete_property(env, decoders, args[0], &deleted);
    }
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

/* ============================================================================
   MODULE INITIALIZATION
   ============================================================================ */

static napi_value Init(napi_env env, napi_value exports) {
    napi_status status;
    napi_value fn;
    
    // Create dumps function
    status = napi_create_function(env, "dumps", NAPI_AUTO_LENGTH, dumps, NULL, &fn);
    if (status != napi_ok) return NULL;
    status = napi_set_named_property(env, exports, "dumps", fn);
    if (status != napi_ok) return NULL;
    
    // Create loads function
    status = napi_create_function(env, "loads", NAPI_AUTO_LENGTH, loads, NULL, &fn);
    if (status != napi_ok) return NULL;
    status = napi_set_named_property(env, exports, "loads", fn);
    if (status != napi_ok) return NULL;
    
    // Create register_serializer function
    status = napi_create_function(env, "register_serializer", NAPI_AUTO_LENGTH, 
                                   register_serializer, NULL, &fn);
    if (status != napi_ok) return NULL;
    status = napi_set_named_property(env, exports, "registerSerializer", fn);
    if (status != napi_ok) return NULL;
    
    // Create unregister_serializer function
    status = napi_create_function(env, "unregister_serializer", NAPI_AUTO_LENGTH, 
                                   unregister_serializer, NULL, &fn);
    if (status != napi_ok) return NULL;
    status = napi_set_named_property(env, exports, "unregisterSerializer", fn);
    if (status != napi_ok) return NULL;
    
    // Create register_decoder function
    status = napi_create_function(env, "register_decoder", NAPI_AUTO_LENGTH, 
                                   register_decoder, NULL, &fn);
    if (status != napi_ok) return NULL;
    status = napi_set_named_property(env, exports, "registerDecoder", fn);
    if (status != napi_ok) return NULL;
    
    // Create unregister_decoder function
    status = napi_create_function(env, "unregister_decoder", NAPI_AUTO_LENGTH, 
                                   unregister_decoder, NULL, &fn);
    if (status != napi_ok) return NULL;
    status = napi_set_named_property(env, exports, "unregisterDecoder", fn);
    if (status != napi_ok) return NULL;
    
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
