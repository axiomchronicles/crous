#include "../include/crous_flux.h"
#include "../include/crous_value.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

/* ============================================================================
   FLUX TEXT SERIALIZER
   ============================================================================ */

typedef struct {
    crous_output_stream *out;
    int indent_level;
    char indent_char;
} flux_text_context_t;

static crous_err_t write_text(crous_output_stream *out, const char *text, size_t len) {
    if (out->write(out->user_data, (const uint8_t *)text, len) != len) {
        return CROUS_ERR_STREAM;
    }
    return CROUS_OK;
}

static crous_err_t write_indent(flux_text_context_t *ctx) {
    for (int i = 0; i < ctx->indent_level; i++) {
        if (write_text(ctx->out, &ctx->indent_char, 1) != CROUS_OK) {
            return CROUS_ERR_STREAM;
        }
    }
    return CROUS_OK;
}

static crous_err_t serialize_value_text(flux_text_context_t *ctx, const crous_value *v);

static crous_err_t serialize_string(flux_text_context_t *ctx, const crous_value *v) {
    size_t len;
    const char *data = crous_value_get_string(v, &len);
    
    /* Check if quoting is needed */
    int needs_quotes = 0;
    for (size_t i = 0; i < len; i++) {
        if (isspace(data[i]) || strchr(":@[]#\"'", data[i])) {
            needs_quotes = 1;
            break;
        }
    }
    
    if (needs_quotes) {
        if (write_text(ctx->out, "\"", 1) != CROUS_OK) return CROUS_ERR_STREAM;
        if (write_text(ctx->out, data, len) != CROUS_OK) return CROUS_ERR_STREAM;
        if (write_text(ctx->out, "\"", 1) != CROUS_OK) return CROUS_ERR_STREAM;
    } else {
        if (write_text(ctx->out, data, len) != CROUS_OK) return CROUS_ERR_STREAM;
    }
    
    return CROUS_OK;
}

static crous_err_t serialize_record_text(flux_text_context_t *ctx, const crous_value *v) {
    size_t count = crous_value_dict_size(v);
    
    for (size_t i = 0; i < count; i++) {
        const crous_dict_entry *entry = crous_value_dict_get_entry(v, i);
        if (!entry) return CROUS_ERR_INVALID_TYPE;
        
        /* Write indentation and key */
        if (write_indent(ctx) != CROUS_OK) return CROUS_ERR_STREAM;
        if (write_text(ctx->out, entry->key, entry->key_len) != CROUS_OK) 
            return CROUS_ERR_STREAM;
        if (write_text(ctx->out, ": ", 2) != CROUS_OK) return CROUS_ERR_STREAM;
        
        /* Write value */
        if (serialize_value_text(ctx, entry->value) != CROUS_OK)
            return CROUS_ERR_STREAM;
        
        if (write_text(ctx->out, "\n", 1) != CROUS_OK) return CROUS_ERR_STREAM;
    }
    
    return CROUS_OK;
}

static crous_err_t serialize_array_text(flux_text_context_t *ctx, const crous_value *v) {
    size_t count = crous_value_list_size(v);
    
    /* Write array type hint */
    if (write_text(ctx->out, "[item]\n", 7) != CROUS_OK) return CROUS_ERR_STREAM;
    
    /* Increase indent for array elements */
    ctx->indent_level++;
    
    for (size_t i = 0; i < count; i++) {
        crous_value *elem = crous_value_list_get(v, i);
        if (!elem) return CROUS_ERR_INVALID_TYPE;
        
        if (write_indent(ctx) != CROUS_OK) return CROUS_ERR_STREAM;
        if (serialize_value_text(ctx, elem) != CROUS_OK) return CROUS_ERR_STREAM;
        if (write_text(ctx->out, "\n", 1) != CROUS_OK) return CROUS_ERR_STREAM;
    }
    
    /* Decrease indent */
    ctx->indent_level--;
    
    return CROUS_OK;
}

static crous_err_t serialize_value_text(flux_text_context_t *ctx, const crous_value *v) {
    if (!v) return CROUS_ERR_INVALID_TYPE;
    
    switch (crous_value_get_type(v)) {
        case CROUS_TYPE_NULL:
            return write_text(ctx->out, "null", 4);
        
        case CROUS_TYPE_BOOL: {
            if (crous_value_get_bool(v)) {
                return write_text(ctx->out, "true", 4);
            } else {
                return write_text(ctx->out, "false", 5);
            }
        }
        
        case CROUS_TYPE_INT: {
            int64_t val = crous_value_get_int(v);
            char buf[32];
            int len = snprintf(buf, sizeof(buf), "%lld", (long long)val);
            return write_text(ctx->out, buf, len);
        }
        
        case CROUS_TYPE_FLOAT: {
            double val = crous_value_get_float(v);
            char buf[32];
            int len = snprintf(buf, sizeof(buf), "%.17g", val);
            return write_text(ctx->out, buf, len);
        }
        
        case CROUS_TYPE_STRING:
            return serialize_string(ctx, v);
        
        case CROUS_TYPE_BYTES:
            /* Bytes as hex-encoded string */
            return write_text(ctx->out, "null", 4);  /* TODO: hex encoding */
        
        case CROUS_TYPE_LIST:
            return serialize_array_text(ctx, v);
        
        case CROUS_TYPE_DICT:
            return serialize_record_text(ctx, v);
        
        default:
            return CROUS_ERR_INVALID_TYPE;
    }
}

/* ============================================================================
   FLUX BINARY SERIALIZER
   ============================================================================ */

typedef struct {
    uint8_t *buf;
    size_t pos;
    size_t cap;
} flux_binary_context_t;

static crous_err_t binary_write(flux_binary_context_t *ctx, const uint8_t *data, size_t len) {
    if (ctx->pos + len > ctx->cap) {
        size_t new_cap = ctx->cap * 2;
        while (new_cap < ctx->pos + len) new_cap *= 2;
        
        uint8_t *new_buf = realloc(ctx->buf, new_cap);
        if (!new_buf) return CROUS_ERR_OOM;
        
        ctx->buf = new_buf;
        ctx->cap = new_cap;
    }
    
    memcpy(ctx->buf + ctx->pos, data, len);
    ctx->pos += len;
    return CROUS_OK;
}

static crous_err_t binary_write_varint(flux_binary_context_t *ctx, uint64_t val) {
    uint8_t buf[10];
    int count = 0;
    
    while (val >= 0x80) {
        buf[count++] = (uint8_t)((val & 0x7F) | 0x80);
        val >>= 7;
    }
    buf[count++] = (uint8_t)(val & 0x7F);
    
    return binary_write(ctx, buf, count);
}

static crous_err_t serialize_value_binary(flux_binary_context_t *ctx, const crous_value *v);

static crous_err_t serialize_record_binary(flux_binary_context_t *ctx, const crous_value *v) {
    size_t count = crous_value_dict_size(v);
    crous_err_t err = binary_write_varint(ctx, count);
    if (err != CROUS_OK) return err;
    
    for (size_t i = 0; i < count; i++) {
        const crous_dict_entry *entry = crous_value_dict_get_entry(v, i);
        if (!entry) return CROUS_ERR_INVALID_TYPE;
        
        /* Write key */
        err = binary_write_varint(ctx, entry->key_len);
        if (err != CROUS_OK) return err;
        err = binary_write(ctx, (const uint8_t *)entry->key, entry->key_len);
        if (err != CROUS_OK) return err;
        
        /* Write value */
        err = serialize_value_binary(ctx, entry->value);
        if (err != CROUS_OK) return err;
    }
    
    return CROUS_OK;
}

static crous_err_t serialize_array_binary(flux_binary_context_t *ctx, const crous_value *v) {
    size_t count = crous_value_list_size(v);
    crous_err_t err = binary_write_varint(ctx, count);
    if (err != CROUS_OK) return err;
    
    for (size_t i = 0; i < count; i++) {
        crous_value *elem = crous_value_list_get(v, i);
        if (!elem) return CROUS_ERR_INVALID_TYPE;
        
        err = serialize_value_binary(ctx, elem);
        if (err != CROUS_OK) return err;
    }
    
    return CROUS_OK;
}

enum {
    FLUX_TAG_NULL = 0x00,
    FLUX_TAG_FALSE = 0x01,
    FLUX_TAG_TRUE = 0x02,
    FLUX_TAG_INT = 0x03,
    FLUX_TAG_FLOAT = 0x04,
    FLUX_TAG_STRING = 0x05,
    FLUX_TAG_BYTES = 0x06,
    FLUX_TAG_LIST = 0x07,
    FLUX_TAG_DICT = 0x08,
};

static crous_err_t serialize_value_binary(flux_binary_context_t *ctx, const crous_value *v) {
    if (!v) return CROUS_ERR_INVALID_TYPE;
    
    uint8_t tag;
    crous_err_t err;
    
    switch (crous_value_get_type(v)) {
        case CROUS_TYPE_NULL:
            tag = FLUX_TAG_NULL;
            return binary_write(ctx, &tag, 1);
        
        case CROUS_TYPE_BOOL: {
            tag = crous_value_get_bool(v) ? FLUX_TAG_TRUE : FLUX_TAG_FALSE;
            return binary_write(ctx, &tag, 1);
        }
        
        case CROUS_TYPE_INT: {
            tag = FLUX_TAG_INT;
            err = binary_write(ctx, &tag, 1);
            if (err != CROUS_OK) return err;
            
            int64_t val = crous_value_get_int(v);
            uint64_t encoded = (uint64_t)((val << 1) ^ (val >> 63));
            return binary_write_varint(ctx, encoded);
        }
        
        case CROUS_TYPE_FLOAT: {
            tag = FLUX_TAG_FLOAT;
            err = binary_write(ctx, &tag, 1);
            if (err != CROUS_OK) return err;
            
            double val = crous_value_get_float(v);
            uint8_t bytes[8];
            memcpy(bytes, &val, 8);
            return binary_write(ctx, bytes, 8);
        }
        
        case CROUS_TYPE_STRING: {
            tag = FLUX_TAG_STRING;
            err = binary_write(ctx, &tag, 1);
            if (err != CROUS_OK) return err;
            
            size_t len;
            const char *data = crous_value_get_string(v, &len);
            err = binary_write_varint(ctx, len);
            if (err != CROUS_OK) return err;
            return binary_write(ctx, (const uint8_t *)data, len);
        }
        
        case CROUS_TYPE_BYTES: {
            tag = FLUX_TAG_BYTES;
            err = binary_write(ctx, &tag, 1);
            if (err != CROUS_OK) return err;
            
            size_t len;
            const uint8_t *data = crous_value_get_bytes(v, &len);
            err = binary_write_varint(ctx, len);
            if (err != CROUS_OK) return err;
            return binary_write(ctx, data, len);
        }
        
        case CROUS_TYPE_LIST: {
            tag = FLUX_TAG_LIST;
            err = binary_write(ctx, &tag, 1);
            if (err != CROUS_OK) return err;
            return serialize_array_binary(ctx, v);
        }
        
        case CROUS_TYPE_DICT: {
            tag = FLUX_TAG_DICT;
            err = binary_write(ctx, &tag, 1);
            if (err != CROUS_OK) return err;
            return serialize_record_binary(ctx, v);
        }
        
        default:
            return CROUS_ERR_INVALID_TYPE;
    }
}

/* ============================================================================
   PUBLIC API
   ============================================================================ */

crous_err_t flux_serialize_text(const crous_value *value, crous_output_stream *out) {
    if (!value || !out) return CROUS_ERR_INVALID_TYPE;
    
    flux_text_context_t ctx = {
        .out = out,
        .indent_level = 0,
        .indent_char = ' '
    };
    
    return serialize_value_text(&ctx, value);
}

crous_err_t flux_serialize_binary(const crous_value *value, crous_output_stream *out) {
    if (!value || !out) return CROUS_ERR_INVALID_TYPE;
    
    /* Build binary in memory first */
    flux_binary_context_t ctx = {
        .buf = malloc(1024),
        .pos = 0,
        .cap = 1024
    };
    
    if (!ctx.buf) return CROUS_ERR_OOM;
    
    /* Write header */
    uint8_t header[6] = {
        FLUX_MAGIC_0, FLUX_MAGIC_1, FLUX_MAGIC_2, FLUX_MAGIC_3,
        FLUX_VERSION, 0x00
    };
    crous_err_t err = binary_write(&ctx, header, 6);
    if (err != CROUS_OK) {
        free(ctx.buf);
        return err;
    }
    
    /* Serialize value */
    err = serialize_value_binary(&ctx, value);
    if (err != CROUS_OK) {
        free(ctx.buf);
        return err;
    }
    
    /* Write to output stream */
    if (out->write(out->user_data, ctx.buf, ctx.pos) != ctx.pos) {
        free(ctx.buf);
        return CROUS_ERR_STREAM;
    }
    
    free(ctx.buf);
    return CROUS_OK;
}

/* Helper for text encoding buffer output stream */
typedef struct {
    char *data;
    size_t pos;
    size_t cap;
} flux_text_enc_buf_t;

static size_t flux_text_enc_write(void *user_data, const uint8_t *data, size_t len) {
    flux_text_enc_buf_t *ctx = (flux_text_enc_buf_t *)user_data;
    if (len == 0) return 0;
    
    if (ctx->pos + len > ctx->cap) {
        size_t new_cap = ctx->cap * 2;
        while (new_cap < ctx->pos + len) new_cap *= 2;
        char *new_data = realloc(ctx->data, new_cap);
        if (!new_data) return (size_t)-1;
        ctx->data = new_data;
        ctx->cap = new_cap;
    }
    
    memcpy(ctx->data + ctx->pos, (const char *)data, len);
    ctx->pos += len;
    return len;
}

crous_err_t flux_encode_text(const crous_value *value, char **out_buf, size_t *out_size) {
    if (!value || !out_buf || !out_size) return CROUS_ERR_INVALID_TYPE;
    
    flux_text_enc_buf_t ctx = {
        .data = malloc(4096),
        .pos = 0,
        .cap = 4096
    };
    
    if (!ctx.data) return CROUS_ERR_OOM;
    
    crous_output_stream out;
    out.user_data = &ctx;
    out.write = flux_text_enc_write;
    
    crous_err_t err = flux_serialize_text(value, &out);
    
    if (err == CROUS_OK) {
        *out_buf = ctx.data;
        *out_size = ctx.pos;
    } else {
        free(ctx.data);
    }
    
    return err;
}

crous_err_t flux_decode_text(const char *buf, size_t buf_size, crous_value **out_value) {
    if (!buf || !out_value) return CROUS_ERR_INVALID_TYPE;
    
    flux_lexer_t *lexer = flux_lexer_new(buf, buf_size);
    if (!lexer) return CROUS_ERR_OOM;
    
    flux_parser_t *parser = flux_parser_new(lexer);
    if (!parser) {
        flux_lexer_free(lexer);
        return CROUS_ERR_OOM;
    }
    
    crous_err_t err = flux_parse(parser, out_value);
    
    flux_parser_free(parser);
    flux_lexer_free(lexer);
    
    return err;
}

/* Helper for binary encoding buffer output stream */
typedef struct {
    uint8_t *data;
    size_t pos;
    size_t cap;
} flux_encode_buf_t;

static size_t flux_encode_buf_write(void *user_data, const uint8_t *data, size_t len) {
    flux_encode_buf_t *ctx = (flux_encode_buf_t *)user_data;
    if (len == 0) return 0;
    
    if (ctx->pos + len > ctx->cap) {
        size_t new_cap = ctx->cap * 2;
        while (new_cap < ctx->pos + len) new_cap *= 2;
        uint8_t *new_data = realloc(ctx->data, new_cap);
        if (!new_data) return (size_t)-1;
        ctx->data = new_data;
        ctx->cap = new_cap;
    }
    
    memcpy(ctx->data + ctx->pos, data, len);
    ctx->pos += len;
    return len;
}

crous_err_t flux_encode_binary(const crous_value *value, uint8_t **out_buf, size_t *out_size) {
    if (!value || !out_buf || !out_size) return CROUS_ERR_INVALID_TYPE;
    
    flux_encode_buf_t ctx = {
        .data = malloc(1024),
        .pos = 0,
        .cap = 1024
    };
    
    if (!ctx.data) return CROUS_ERR_OOM;
    
    crous_output_stream out;
    out.user_data = &ctx;
    out.write = flux_encode_buf_write;
    
    crous_err_t err = flux_serialize_binary(value, &out);
    
    if (err == CROUS_OK) {
        *out_buf = ctx.data;
        *out_size = ctx.pos;
    } else {
        free(ctx.data);
    }
    
    return err;
}

/* ============================================================================
   FLUX BINARY DESERIALIZATION
   ============================================================================ */

typedef struct {
    const uint8_t *buf;
    size_t pos;
    size_t len;
} flux_decode_buf_t;

static crous_err_t binary_read(flux_decode_buf_t *ctx, uint8_t *out, size_t len) {
    if (ctx->pos + len > ctx->len) return CROUS_ERR_TRUNCATED;
    memcpy(out, ctx->buf + ctx->pos, len);
    ctx->pos += len;
    return CROUS_OK;
}

static crous_err_t binary_read_varint(flux_decode_buf_t *ctx, uint64_t *out) {
    uint64_t result = 0;
    int shift = 0;
    
    for (int i = 0; i < 10; i++) {
        uint8_t byte;
        crous_err_t err = binary_read(ctx, &byte, 1);
        if (err != CROUS_OK) return err;
        
        result |= ((uint64_t)(byte & 0x7F)) << shift;
        if ((byte & 0x80) == 0) {
            *out = result;
            return CROUS_OK;
        }
        shift += 7;
    }
    
    return CROUS_ERR_DECODE;
}

static crous_err_t deserialize_value_binary(flux_decode_buf_t *ctx, crous_value **out_value, int depth);

static crous_err_t deserialize_array_binary(flux_decode_buf_t *ctx, crous_value **out_array, int depth) {
    uint64_t count;
    crous_err_t err = binary_read_varint(ctx, &count);
    if (err != CROUS_OK) return err;
    
    if (count > CROUS_MAX_LIST_SIZE) return CROUS_ERR_DECODE;
    
    *out_array = crous_value_new_list(count);
    if (!*out_array) return CROUS_ERR_OOM;
    
    for (uint64_t i = 0; i < count; i++) {
        crous_value *elem = NULL;
        err = deserialize_value_binary(ctx, &elem, depth + 1);
        if (err != CROUS_OK) {
            crous_value_free_tree(*out_array);
            return err;
        }
        
        err = crous_value_list_append(*out_array, elem);
        if (err != CROUS_OK) {
            crous_value_free_tree(*out_array);
            crous_value_free_tree(elem);
            return err;
        }
    }
    
    return CROUS_OK;
}

static crous_err_t deserialize_dict_binary(flux_decode_buf_t *ctx, crous_value **out_dict, int depth) {
    uint64_t count;
    crous_err_t err = binary_read_varint(ctx, &count);
    if (err != CROUS_OK) return err;
    
    if (count > CROUS_MAX_DICT_SIZE) return CROUS_ERR_DECODE;
    
    *out_dict = crous_value_new_dict(count);
    if (!*out_dict) return CROUS_ERR_OOM;
    
    for (uint64_t i = 0; i < count; i++) {
        /* Read key */
        uint64_t key_len;
        err = binary_read_varint(ctx, &key_len);
        if (err != CROUS_OK) {
            crous_value_free_tree(*out_dict);
            return err;
        }
        
        if (key_len > CROUS_MAX_STRING_BYTES) {
            crous_value_free_tree(*out_dict);
            return CROUS_ERR_DECODE;
        }
        
        uint8_t *key_data = malloc(key_len);
        if (!key_data) {
            crous_value_free_tree(*out_dict);
            return CROUS_ERR_OOM;
        }
        
        err = binary_read(ctx, key_data, key_len);
        if (err != CROUS_OK) {
            free(key_data);
            crous_value_free_tree(*out_dict);
            return err;
        }
        
        /* Read value */
        crous_value *val = NULL;
        err = deserialize_value_binary(ctx, &val, depth + 1);
        if (err != CROUS_OK) {
            free(key_data);
            crous_value_free_tree(*out_dict);
            return err;
        }
        
        /* Set in dictionary */
        err = crous_value_dict_set_binary(*out_dict, (const char *)key_data, key_len, val);
        free(key_data);
        
        if (err != CROUS_OK) {
            crous_value_free_tree(*out_dict);
            crous_value_free_tree(val);
            return err;
        }
    }
    
    return CROUS_OK;
}

static crous_err_t deserialize_value_binary(flux_decode_buf_t *ctx, crous_value **out_value, int depth) {
    if (depth >= CROUS_MAX_DEPTH) return CROUS_ERR_DECODE;
    
    uint8_t tag;
    crous_err_t err = binary_read(ctx, &tag, 1);
    if (err != CROUS_OK) return err;
    
    crous_value *v = NULL;
    
    switch (tag) {
        case FLUX_TAG_NULL:
            v = crous_value_new_null();
            if (!v) return CROUS_ERR_OOM;
            break;
        
        case FLUX_TAG_FALSE:
            v = crous_value_new_bool(0);
            if (!v) return CROUS_ERR_OOM;
            break;
        
        case FLUX_TAG_TRUE:
            v = crous_value_new_bool(1);
            if (!v) return CROUS_ERR_OOM;
            break;
        
        case FLUX_TAG_INT: {
            uint64_t encoded;
            err = binary_read_varint(ctx, &encoded);
            if (err != CROUS_OK) return err;
            
            /* Decode zigzag encoding */
            int64_t val = (int64_t)((encoded >> 1) ^ (-(int64_t)(encoded & 1)));
            v = crous_value_new_int(val);
            if (!v) return CROUS_ERR_OOM;
            break;
        }
        
        case FLUX_TAG_FLOAT: {
            uint8_t bytes[8];
            err = binary_read(ctx, bytes, 8);
            if (err != CROUS_OK) return err;
            
            double val;
            memcpy(&val, bytes, 8);
            v = crous_value_new_float(val);
            if (!v) return CROUS_ERR_OOM;
            break;
        }
        
        case FLUX_TAG_STRING: {
            uint64_t len;
            err = binary_read_varint(ctx, &len);
            if (err != CROUS_OK) return err;
            
            if (len > CROUS_MAX_STRING_BYTES) return CROUS_ERR_DECODE;
            
            uint8_t *str_data = malloc(len);
            if (!str_data) return CROUS_ERR_OOM;
            
            err = binary_read(ctx, str_data, len);
            if (err != CROUS_OK) {
                free(str_data);
                return err;
            }
            
            v = crous_value_new_string((const char *)str_data, len);
            free(str_data);
            if (!v) return CROUS_ERR_OOM;
            break;
        }
        
        case FLUX_TAG_BYTES: {
            uint64_t len;
            err = binary_read_varint(ctx, &len);
            if (err != CROUS_OK) return err;
            
            if (len > CROUS_MAX_BYTES_SIZE) return CROUS_ERR_DECODE;
            
            uint8_t *bytes_data = malloc(len);
            if (!bytes_data) return CROUS_ERR_OOM;
            
            err = binary_read(ctx, bytes_data, len);
            if (err != CROUS_OK) {
                free(bytes_data);
                return err;
            }
            
            v = crous_value_new_bytes(bytes_data, len);
            free(bytes_data);
            if (!v) return CROUS_ERR_OOM;
            break;
        }
        
        case FLUX_TAG_LIST:
            err = deserialize_array_binary(ctx, &v, depth);
            if (err != CROUS_OK) return err;
            break;
        
        case FLUX_TAG_DICT:
            err = deserialize_dict_binary(ctx, &v, depth);
            if (err != CROUS_OK) return err;
            break;
        
        default:
            return CROUS_ERR_DECODE;
    }
    
    *out_value = v;
    return CROUS_OK;
}

crous_err_t flux_decode_binary(const uint8_t *buf, size_t buf_size, crous_value **out_value) {
    if (!buf || !out_value) return CROUS_ERR_INVALID_TYPE;
    
    if (buf_size < 6) return CROUS_ERR_TRUNCATED;
    
    /* Check FLUX magic */
    if (buf[0] != FLUX_MAGIC_0 || buf[1] != FLUX_MAGIC_1 ||
        buf[2] != FLUX_MAGIC_2 || buf[3] != FLUX_MAGIC_3) {
        return CROUS_ERR_INVALID_HEADER;
    }
    
    if (buf[4] != FLUX_VERSION) {
        return CROUS_ERR_INVALID_HEADER;
    }
    
    flux_decode_buf_t ctx = {
        .buf = buf,
        .pos = 6,  /* Skip header */
        .len = buf_size
    };
    
    return deserialize_value_binary(&ctx, out_value, 0);
}
