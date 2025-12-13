#include "../include/crous_binary.h"
#include "../include/crous_value.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Binary format tag codes */
enum {
    CROUS_TAG_NULL = 0x00,
    CROUS_TAG_FALSE = 0x01,
    CROUS_TAG_TRUE = 0x02,
    CROUS_TAG_INT64 = 0x03,
    CROUS_TAG_FLOAT64 = 0x04,
    CROUS_TAG_STRING = 0x05,
    CROUS_TAG_BYTES = 0x06,
    CROUS_TAG_LIST = 0x07,
    CROUS_TAG_TUPLE = 0x08,
    CROUS_TAG_DICT = 0x09,
    CROUS_TAG_TAGGED_VALUE = 0x0A,
    CROUS_TAG_POSINT_BASE = 0x10,
    CROUS_TAG_POSINT_MAX = 0x28,
    CROUS_TAG_NEGINT_BASE = 0x29,
    CROUS_TAG_NEGINT_MAX = 0x48,
};

/* ============================================================================
   VARINT ENCODING
   ============================================================================ */

static int varint_encode(uint64_t value, uint8_t *buf) {
    int count = 0;
    while (value >= 0x80) {
        buf[count++] = (uint8_t)((value & 0x7F) | 0x80);
        value >>= 7;
    }
    buf[count++] = (uint8_t)(value & 0x7F);
    return count;
}

/* ============================================================================
   STREAM HELPERS
   ============================================================================ */

static crous_err_t stream_write(crous_output_stream *out, const void *data, size_t len) {
    if (out->write == NULL) return CROUS_ERR_STREAM;
    size_t written = out->write(out->user_data, (const uint8_t *)data, len);
    if (written != len) return CROUS_ERR_STREAM;
    return CROUS_OK;
}

static crous_err_t stream_write_byte(crous_output_stream *out, uint8_t byte) {
    return stream_write(out, &byte, 1);
}

static crous_err_t stream_write_varint(crous_output_stream *out, uint64_t value) {
    uint8_t buf[10];
    int count = varint_encode(value, buf);
    return stream_write(out, buf, count);
}

static crous_err_t stream_read(crous_input_stream *in, uint8_t *buf, size_t len) {
    if (in->read == NULL) return CROUS_ERR_STREAM;
    size_t read = in->read(in->user_data, buf, len);
    if (read < len) return CROUS_ERR_TRUNCATED;
    return CROUS_OK;
}

static crous_err_t stream_read_byte(crous_input_stream *in, uint8_t *out) {
    return stream_read(in, out, 1);
}

static crous_err_t stream_read_varint(crous_input_stream *in, uint64_t *out) {
    uint64_t result = 0;
    int shift = 0;
    
    for (int i = 0; i < 10; i++) {
        uint8_t byte;
        crous_err_t err = stream_read_byte(in, &byte);
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

/* ============================================================================
   HELPERS
   ============================================================================ */

static inline int is_small_posint(int64_t v) {
    return v >= 0 && v <= 24;
}

static inline int is_small_negint(int64_t v) {
    return v >= -32 && v <= -1;
}

static int is_utf8_valid(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; ) {
        uint8_t byte = data[i];
        int cont_bytes = 0;
        
        if ((byte & 0x80) == 0) {
            cont_bytes = 0;
        } else if ((byte & 0xE0) == 0xC0) {
            cont_bytes = 1;
        } else if ((byte & 0xF0) == 0xE0) {
            cont_bytes = 2;
        } else if ((byte & 0xF8) == 0xF0) {
            cont_bytes = 3;
        } else {
            return 0;
        }
        
        i++;
        for (int j = 0; j < cont_bytes; j++) {
            if (i >= len) return 0;
            uint8_t cont = data[i++];
            if ((cont & 0xC0) != 0x80) return 0;
        }
    }
    return 1;
}

/* ============================================================================
   ENCODING
   ============================================================================ */

static crous_err_t encode_value_to_stream(const crous_value *v, crous_output_stream *out);

static crous_err_t encode_value_to_stream(const crous_value *v, crous_output_stream *out) {
    if (!v) return CROUS_ERR_INVALID_TYPE;
    
    uint8_t tag_byte;
    
    switch (v->type) {
        case CROUS_TYPE_NULL: {
            tag_byte = CROUS_TAG_NULL;
            return stream_write_byte(out, tag_byte);
        }
        
        case CROUS_TYPE_BOOL: {
            tag_byte = v->data.b ? CROUS_TAG_TRUE : CROUS_TAG_FALSE;
            return stream_write_byte(out, tag_byte);
        }
        
        case CROUS_TYPE_INT: {
            int64_t val = v->data.i;
            
            if (is_small_posint(val)) {
                tag_byte = CROUS_TAG_POSINT_BASE + val;
                return stream_write_byte(out, tag_byte);
            } else if (is_small_negint(val)) {
                tag_byte = CROUS_TAG_NEGINT_BASE + (-1 - val);
                return stream_write_byte(out, tag_byte);
            } else {
                crous_err_t err = stream_write_byte(out, CROUS_TAG_INT64);
                if (err != CROUS_OK) return err;
                
                uint8_t bytes[8];
                for (int i = 0; i < 8; i++) {
                    bytes[i] = (uint8_t)((val >> (i * 8)) & 0xFF);
                }
                return stream_write(out, bytes, 8);
            }
        }
        
        case CROUS_TYPE_FLOAT: {
            crous_err_t err = stream_write_byte(out, CROUS_TAG_FLOAT64);
            if (err != CROUS_OK) return err;
            
            uint8_t bytes[8];
            double d = v->data.f;
            memcpy(bytes, &d, 8);
            return stream_write(out, bytes, 8);
        }
        
        case CROUS_TYPE_STRING: {
            crous_err_t err = stream_write_byte(out, CROUS_TAG_STRING);
            if (err != CROUS_OK) return err;
            
            size_t len = v->data.s.len;
            err = stream_write_varint(out, len);
            if (err != CROUS_OK) return err;
            if (len > 0) return stream_write(out, v->data.s.data, len);
            return CROUS_OK;
        }
        
        case CROUS_TYPE_BYTES: {
            crous_err_t err = stream_write_byte(out, CROUS_TAG_BYTES);
            if (err != CROUS_OK) return err;
            
            size_t len = v->data.bytes.len;
            err = stream_write_varint(out, len);
            if (err != CROUS_OK) return err;
            if (len > 0) return stream_write(out, v->data.bytes.data, len);
            return CROUS_OK;
        }
        
        case CROUS_TYPE_LIST: {
            crous_err_t err = stream_write_byte(out, CROUS_TAG_LIST);
            if (err != CROUS_OK) return err;
            
            size_t count = v->data.list.len;
            err = stream_write_varint(out, count);
            if (err != CROUS_OK) return err;
            
            for (size_t i = 0; i < count; i++) {
                err = encode_value_to_stream(v->data.list.items[i], out);
                if (err != CROUS_OK) return err;
            }
            return CROUS_OK;
        }
        
        case CROUS_TYPE_TUPLE: {
            crous_err_t err = stream_write_byte(out, CROUS_TAG_TUPLE);
            if (err != CROUS_OK) return err;
            
            size_t count = v->data.list.len;
            err = stream_write_varint(out, count);
            if (err != CROUS_OK) return err;
            
            for (size_t i = 0; i < count; i++) {
                err = encode_value_to_stream(v->data.list.items[i], out);
                if (err != CROUS_OK) return err;
            }
            return CROUS_OK;
        }
        
        case CROUS_TYPE_DICT: {
            crous_err_t err = stream_write_byte(out, CROUS_TAG_DICT);
            if (err != CROUS_OK) return err;
            
            size_t count = v->data.dict.len;
            err = stream_write_varint(out, count);
            if (err != CROUS_OK) return err;
            
            for (size_t i = 0; i < count; i++) {
                crous_dict_entry *entry = &v->data.dict.entries[i];
                crous_value *key_val = crous_value_new_string(entry->key, entry->key_len);
                if (!key_val) return CROUS_ERR_OOM;
                err = encode_value_to_stream(key_val, out);
                crous_value_free_tree(key_val);
                if (err != CROUS_OK) return err;
                
                err = encode_value_to_stream(entry->value, out);
                if (err != CROUS_OK) return err;
            }
            return CROUS_OK;
        }
        
        case CROUS_TYPE_TAGGED: {
            crous_err_t err = stream_write_byte(out, CROUS_TAG_TAGGED_VALUE);
            if (err != CROUS_OK) return err;
            
            err = stream_write_varint(out, v->data.tagged.tag);
            if (err != CROUS_OK) return err;
            
            return encode_value_to_stream(v->data.tagged.value, out);
        }
        
        default:
            return CROUS_ERR_INVALID_TYPE;
    }
}

/* ============================================================================
   DECODING
   ============================================================================ */

static crous_err_t decode_value_from_stream(crous_input_stream *in, crous_value **out_value, int depth);

static crous_err_t decode_value_from_stream(crous_input_stream *in, crous_value **out_value, int depth) {
    if (depth >= CROUS_MAX_DEPTH) return CROUS_ERR_DECODE;
    
    uint8_t tag;
    crous_err_t err = stream_read_byte(in, &tag);
    if (err != CROUS_OK) return err;
    
    crous_value *v = NULL;
    
    if (tag == CROUS_TAG_NULL) {
        v = crous_value_new_null();
        if (!v) return CROUS_ERR_OOM;
    } else if (tag == CROUS_TAG_FALSE) {
        v = crous_value_new_bool(0);
        if (!v) return CROUS_ERR_OOM;
    } else if (tag == CROUS_TAG_TRUE) {
        v = crous_value_new_bool(1);
        if (!v) return CROUS_ERR_OOM;
    } else if (tag >= CROUS_TAG_POSINT_BASE && tag <= CROUS_TAG_POSINT_MAX) {
        int64_t val = tag - CROUS_TAG_POSINT_BASE;
        v = crous_value_new_int(val);
        if (!v) return CROUS_ERR_OOM;
    } else if (tag >= CROUS_TAG_NEGINT_BASE && tag <= CROUS_TAG_NEGINT_MAX) {
        int64_t val = -1 - (tag - CROUS_TAG_NEGINT_BASE);
        v = crous_value_new_int(val);
        if (!v) return CROUS_ERR_OOM;
    } else if (tag == CROUS_TAG_INT64) {
        uint8_t bytes[8];
        err = stream_read(in, bytes, 8);
        if (err != CROUS_OK) return err;
        int64_t val = 0;
        for (int i = 0; i < 8; i++) {
            val |= ((int64_t)bytes[i]) << (i * 8);
        }
        v = crous_value_new_int(val);
        if (!v) return CROUS_ERR_OOM;
    } else if (tag == CROUS_TAG_FLOAT64) {
        uint8_t bytes[8];
        err = stream_read(in, bytes, 8);
        if (err != CROUS_OK) return err;
        double d;
        memcpy(&d, bytes, 8);
        v = crous_value_new_float(d);
        if (!v) return CROUS_ERR_OOM;
    } else if (tag == CROUS_TAG_STRING) {
        uint64_t len;
        err = stream_read_varint(in, &len);
        if (err != CROUS_OK || len > CROUS_MAX_STRING_BYTES) return CROUS_ERR_DECODE;
        
        uint8_t *str_data = malloc(len);
        if (!str_data) return CROUS_ERR_OOM;
        
        err = stream_read(in, str_data, len);
        if (err != CROUS_OK) {
            free(str_data);
            return err;
        }
        
        if (!is_utf8_valid(str_data, len)) {
            free(str_data);
            return CROUS_ERR_DECODE;
        }
        
        v = crous_value_new_string((const char *)str_data, len);
        free(str_data);
        if (!v) return CROUS_ERR_OOM;
    } else if (tag == CROUS_TAG_BYTES) {
        uint64_t len;
        err = stream_read_varint(in, &len);
        if (err != CROUS_OK || len > CROUS_MAX_BYTES_SIZE) return CROUS_ERR_DECODE;
        
        uint8_t *bytes_data = malloc(len);
        if (!bytes_data) return CROUS_ERR_OOM;
        
        err = stream_read(in, bytes_data, len);
        if (err != CROUS_OK) {
            free(bytes_data);
            return err;
        }
        
        v = crous_value_new_bytes(bytes_data, len);
        free(bytes_data);
        if (!v) return CROUS_ERR_OOM;
    } else if (tag == CROUS_TAG_LIST) {
        uint64_t count;
        err = stream_read_varint(in, &count);
        if (err != CROUS_OK || count > CROUS_MAX_LIST_SIZE) return CROUS_ERR_DECODE;
        
        v = crous_value_new_list(count);
        if (!v) return CROUS_ERR_OOM;
        
        for (uint64_t i = 0; i < count; i++) {
            crous_value *item = NULL;
            err = decode_value_from_stream(in, &item, depth + 1);
            if (err != CROUS_OK) {
                crous_value_free_tree(v);
                return err;
            }
            if (crous_value_list_append(v, item) != CROUS_OK) {
                crous_value_free_tree(v);
                crous_value_free_tree(item);
                return CROUS_ERR_OOM;
            }
        }
    } else if (tag == CROUS_TAG_TUPLE) {
        uint64_t count;
        err = stream_read_varint(in, &count);
        if (err != CROUS_OK || count > CROUS_MAX_LIST_SIZE) return CROUS_ERR_DECODE;
        
        v = crous_value_new_tuple(count);
        if (!v) return CROUS_ERR_OOM;
        
        for (uint64_t i = 0; i < count; i++) {
            crous_value *item = NULL;
            err = decode_value_from_stream(in, &item, depth + 1);
            if (err != CROUS_OK) {
                crous_value_free_tree(v);
                return err;
            }
            if (crous_value_list_append(v, item) != CROUS_OK) {
                crous_value_free_tree(v);
                crous_value_free_tree(item);
                return CROUS_ERR_OOM;
            }
        }
    } else if (tag == CROUS_TAG_DICT) {
        uint64_t count;
        err = stream_read_varint(in, &count);
        if (err != CROUS_OK || count > CROUS_MAX_DICT_SIZE) return CROUS_ERR_DECODE;
        
        v = crous_value_new_dict(count);
        if (!v) return CROUS_ERR_OOM;
        
        for (uint64_t i = 0; i < count; i++) {
            crous_value *key_val = NULL;
            err = decode_value_from_stream(in, &key_val, depth + 1);
            if (err != CROUS_OK || !key_val || key_val->type != CROUS_TYPE_STRING) {
                crous_value_free_tree(v);
                if (key_val) crous_value_free_tree(key_val);
                return CROUS_ERR_DECODE;
            }
            
            crous_value *val_val = NULL;
            err = decode_value_from_stream(in, &val_val, depth + 1);
            if (err != CROUS_OK) {
                crous_value_free_tree(v);
                crous_value_free_tree(key_val);
                return err;
            }
            
            const char *key_str;
            size_t key_len;
            key_str = crous_value_get_string(key_val, &key_len);
            if (crous_value_dict_set_binary(v, key_str, key_len, val_val) != CROUS_OK) {
                crous_value_free_tree(v);
                crous_value_free_tree(key_val);
                crous_value_free_tree(val_val);
                return CROUS_ERR_OOM;
            }
            
            crous_value_free_tree(key_val);
        }
    } else if (tag == CROUS_TAG_TAGGED_VALUE) {
        uint64_t tag_id;
        err = stream_read_varint(in, &tag_id);
        if (err != CROUS_OK) return err;
        
        crous_value *inner = NULL;
        err = decode_value_from_stream(in, &inner, depth + 1);
        if (err != CROUS_OK) return err;
        
        v = crous_value_new_tagged(tag_id, inner);
        if (!v) {
            crous_value_free_tree(inner);
            return CROUS_ERR_OOM;
        }
    } else {
        return CROUS_ERR_DECODE;
    }
    
    *out_value = v;
    return CROUS_OK;
}

/* ============================================================================
   PUBLIC API
   ============================================================================ */

crous_err_t crous_encode_stream(
    const crous_value *value,
    crous_output_stream *out) {
    
    if (!value || !out || out->write == NULL)
        return CROUS_ERR_INVALID_TYPE;
    
    uint8_t header[6] = {
        CROUS_MAGIC_0, CROUS_MAGIC_1, CROUS_MAGIC_2, CROUS_MAGIC_3,
        CROUS_VERSION, 0x00
    };
    crous_err_t err = stream_write(out, header, 6);
    if (err != CROUS_OK) return err;
    
    return encode_value_to_stream(value, out);
}

crous_err_t crous_decode_stream(
    crous_input_stream *in,
    crous_value **out_value) {
    
    if (!in || in->read == NULL || !out_value)
        return CROUS_ERR_INVALID_TYPE;
    
    uint8_t header[6];
    crous_err_t err = stream_read(in, header, 6);
    if (err != CROUS_OK) return err;
    
    if (header[0] != CROUS_MAGIC_0 || header[1] != CROUS_MAGIC_1 ||
        header[2] != CROUS_MAGIC_2 || header[3] != CROUS_MAGIC_3) {
        return CROUS_ERR_INVALID_HEADER;
    }
    
    if (header[4] != CROUS_VERSION) {
        return CROUS_ERR_INVALID_HEADER;
    }
    
    return decode_value_from_stream(in, out_value, 0);
}

crous_err_t crous_encode_value_to_stream(
    const crous_value *value,
    crous_output_stream *out) {
    return encode_value_to_stream(value, out);
}

crous_err_t crous_decode_value_from_stream(
    crous_input_stream *in,
    crous_value **out_value) {
    return decode_value_from_stream(in, out_value, 0);
}

/* ============================================================================
   BUFFER CONVENIENCE API
   ============================================================================ */

/* Buffer stream helpers */
typedef struct {
    const uint8_t *data;
    size_t pos;
    size_t len;
} buffer_input_stream_state;

typedef struct {
    uint8_t *data;
    size_t len;
    size_t cap;
} buffer_output_stream_state;

static size_t buffer_input_read(void *user_data, uint8_t *buf, size_t max_len) {
    buffer_input_stream_state *state = (buffer_input_stream_state *)user_data;
    size_t to_read = state->len - state->pos;
    if (to_read > max_len) to_read = max_len;
    if (to_read > 0) {
        memcpy(buf, state->data + state->pos, to_read);
        state->pos += to_read;
    }
    return to_read;
}

static size_t buffer_output_write(void *user_data, const uint8_t *buf, size_t len) {
    buffer_output_stream_state *state = (buffer_output_stream_state *)user_data;
    if (len == 0) return 0;
    
    while (state->len + len > state->cap) {
        size_t new_cap = (state->cap < 1024 * 1024) ? state->cap * 2 : state->cap + 1024 * 1024;
        uint8_t *new_data = realloc(state->data, new_cap);
        if (!new_data) return (size_t)-1;
        state->data = new_data;
        state->cap = new_cap;
    }
    
    memcpy(state->data + state->len, buf, len);
    state->len += len;
    return len;
}

crous_err_t crous_encode(
    const crous_value *value,
    uint8_t **out_buf,
    size_t *out_size) {
    
    crous_output_stream out;
    buffer_output_stream_state *state = malloc(sizeof(*state));
    if (!state) return CROUS_ERR_OOM;
    
    state->data = malloc(64);
    if (!state->data) {
        free(state);
        return CROUS_ERR_OOM;
    }
    state->len = 0;
    state->cap = 64;
    out.user_data = state;
    out.write = buffer_output_write;
    
    crous_err_t err = crous_encode_stream(value, &out);
    if (err != CROUS_OK) {
        free(state->data);
        free(state);
        return err;
    }
    
    *out_buf = state->data;
    *out_size = state->len;
    free(state);
    
    return CROUS_OK;
}

crous_err_t crous_decode(
    const uint8_t *buf,
    size_t buf_size,
    crous_value **out_value) {
    
    crous_input_stream in;
    buffer_input_stream_state *state = malloc(sizeof(*state));
    if (!state) return CROUS_ERR_OOM;
    
    state->data = buf;
    state->pos = 0;
    state->len = buf_size;
    in.user_data = state;
    in.read = buffer_input_read;
    
    crous_err_t err = crous_decode_stream(&in, out_value);
    free(state);
    return err;
}

/* ============================================================================
   FILE API
   ============================================================================ */

crous_err_t crous_encode_file(
    const crous_value *value,
    const char *path) {
    
    uint8_t *buf = NULL;
    size_t size = 0;
    
    crous_err_t err = crous_encode(value, &buf, &size);
    if (err != CROUS_OK) return err;
    
    FILE *f = fopen(path, "wb");
    if (!f) {
        free(buf);
        return CROUS_ERR_ENCODE;
    }
    
    size_t written = fwrite(buf, 1, size, f);
    int close_status = fclose(f);
    free(buf);
    
    if (written != size || close_status != 0) {
        return CROUS_ERR_ENCODE;
    }
    
    return CROUS_OK;
}

crous_err_t crous_decode_file(
    const char *path,
    crous_value **out_value) {
    
    FILE *f = fopen(path, "rb");
    if (!f) return CROUS_ERR_DECODE;
    
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (file_size < 0) {
        fclose(f);
        return CROUS_ERR_DECODE;
    }
    
    uint8_t *buf = malloc(file_size);
    if (!buf) {
        fclose(f);
        return CROUS_ERR_OOM;
    }
    
    size_t read_bytes = fread(buf, 1, file_size, f);
    fclose(f);
    
    if (read_bytes != (size_t)file_size) {
        free(buf);
        return CROUS_ERR_DECODE;
    }
    
    crous_err_t err = crous_decode(buf, file_size, out_value);
    free(buf);
    
    return err;
}
