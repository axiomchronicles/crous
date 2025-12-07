// crous 2.0 babyyy
// it goes hard
#include "crous.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/*============================================================================
  CONSTANTS
  ============================================================================*/

// magic numbers
#define CROUS_MAGIC_0 0x43  // 'C'
#define CROUS_MAGIC_1 0x52  // 'R'
#define CROUS_MAGIC_2 0x4F  // 'O'
#define CROUS_MAGIC_3 0x55  // 'U'
#define CROUS_VERSION 2     // leveled up

// binary type codes fr
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
    CROUS_TAG_TAGGED_VALUE = 0x0A,  /* NEW: Tagged value type */
    
    // tiny int flex
    CROUS_TAG_POSINT_BASE = 0x10,
    CROUS_TAG_POSINT_MAX = 0x28,   /* 0-28 */
    CROUS_TAG_NEGINT_BASE = 0x29,
    CROUS_TAG_NEGINT_MAX = 0x48,   /* -1 to -32 */
};

// helper structs era
struct crous_context {
    int dummy;  // reserved fr
};

// buffer goes brrr
typedef struct {
    uint8_t *data;
    size_t len;
    size_t cap;
} buffer_t;

// decoder vibes
typedef struct {
    const uint8_t *data;
    size_t pos;
    size_t len;
    int depth;
} decoder_t;

// buffer stream szn
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

// buffer helpers hit different

static buffer_t *buffer_new(void) {
    buffer_t *b = malloc(sizeof(*b));
    if (!b) return NULL;
    b->data = malloc(64);
    if (!b->data) {
        free(b);
        return NULL;
    }
    b->len = 0;
    b->cap = 64;
    return b;
}

static void buffer_free(buffer_t *b) {
    if (b) {
        free(b->data);
        free(b);
    }
}

static crous_err_t buffer_append(buffer_t *b, const void *data, size_t data_len) {
    if (data_len == 0) return CROUS_OK;
    
    while (b->len + data_len > b->cap) {
        size_t new_cap = (b->cap < 1024 * 1024) ? b->cap * 2 : b->cap + 1024 * 1024;
        uint8_t *new_data = realloc(b->data, new_cap);
        if (!new_data) return CROUS_ERR_OOM;
        b->data = new_data;
        b->cap = new_cap;
    }
    
    memcpy(b->data + b->len, data, data_len);
    b->len += data_len;
    return CROUS_OK;
}

// varint magic

static int varint_encode(uint64_t value, uint8_t *buf) {
    int count = 0;
    while (value >= 0x80) {
        buf[count++] = (uint8_t)((value & 0x7F) | 0x80);
        value >>= 7;
    }
    buf[count++] = (uint8_t)(value & 0x7F);
    return count;
}

static uint64_t varint_decode(decoder_t *dec, crous_err_t *err) {
    uint64_t result = 0;
    int shift = 0;
    
    while (dec->pos < dec->len && shift < 63) {
        uint8_t byte = dec->data[dec->pos++];
        result |= ((uint64_t)(byte & 0x7F)) << shift;
        if ((byte & 0x80) == 0) {
            *err = CROUS_OK;
            return result;
        }
        shift += 7;
    }
    
    *err = CROUS_ERR_DECODE;
    return 0;
}

static inline int is_small_posint(int64_t v) {
    return v >= 0 && v <= 28;
}

static inline int is_small_negint(int64_t v) {
    return v >= -32 && v <= -1;
}

// stream mode activated

// reading go brrr
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

// writing era
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

crous_input_stream crous_buffer_input_stream_create(const uint8_t *data, size_t len) {
    crous_input_stream s;
    buffer_input_stream_state *state = malloc(sizeof(*state));
    if (!state) {
        s.user_data = NULL;
        s.read = NULL;
        return s;
    }
    state->data = data;
    state->pos = 0;
    state->len = len;
    s.user_data = state;
    s.read = buffer_input_read;
    return s;
}

crous_output_stream crous_buffer_output_stream_create(void) {
    crous_output_stream s;
    buffer_output_stream_state *state = malloc(sizeof(*state));
    if (!state) {
        s.user_data = NULL;
        s.write = NULL;
        return s;
    }
    state->data = malloc(64);
    if (!state->data) {
        free(state);
        s.user_data = NULL;
        s.write = NULL;
        return s;
    }
    state->len = 0;
    state->cap = 64;
    s.user_data = state;
    s.write = buffer_output_write;
    return s;
}

uint8_t *crous_buffer_output_stream_data(crous_output_stream *s, size_t *out_len) {
    if (!s || !s->user_data) return NULL;
    buffer_output_stream_state *state = (buffer_output_stream_state *)s->user_data;
    if (out_len) *out_len = state->len;
    uint8_t *result = state->data;
    state->data = NULL;
    return result;
}

void crous_buffer_output_stream_free(crous_output_stream *s) {
    if (!s || !s->user_data) return;
    buffer_output_stream_state *state = (buffer_output_stream_state *)s->user_data;
    free(state->data);
    free(state);
    s->user_data = NULL;
}

// context setup

crous_context *crous_context_new(void) {
    return malloc(sizeof(crous_context));
}

void crous_context_free(crous_context *ctx) {
    free(ctx);
}

// spawn that value

crous_value *crous_value_new_null(void) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_NULL;
    return v;
}

crous_value *crous_value_new_bool(int b) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_BOOL;
    v->data.b = b ? 1 : 0;
    return v;
}

crous_value *crous_value_new_int(int64_t v_val) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_INT;
    v->data.i = v_val;
    return v;
}

crous_value *crous_value_new_float(double d) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_FLOAT;
    v->data.f = d;
    return v;
}

crous_value *crous_value_new_string(const char *data, size_t len) {
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

crous_value *crous_value_new_bytes(const uint8_t *data, size_t len) {
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

crous_value *crous_value_new_list(size_t capacity) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_LIST;
    v->data.list.items = capacity > 0 ? malloc(capacity * sizeof(crous_value *)) : NULL;
    if (capacity > 0 && !v->data.list.items) {
        free(v);
        return NULL;
    }
    v->data.list.len = 0;
    return v;
}

crous_value *crous_value_new_tuple(size_t capacity) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_TUPLE;
    v->data.list.items = capacity > 0 ? malloc(capacity * sizeof(crous_value *)) : NULL;
    if (capacity > 0 && !v->data.list.items) {
        free(v);
        return NULL;
    }
    v->data.list.len = 0;
    return v;
}

crous_value *crous_value_new_dict(size_t capacity) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_DICT;
    v->data.dict.entries = capacity > 0 ? malloc(capacity * sizeof(crous_dict_entry)) : NULL;
    if (capacity > 0 && !v->data.dict.entries) {
        free(v);
        return NULL;
    }
    v->data.dict.len = 0;
    return v;
}

crous_value *crous_value_new_tagged(uint32_t tag, crous_value *inner) {
    crous_value *v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->type = CROUS_TYPE_TAGGED;
    v->data.tagged.tag = tag;
    v->data.tagged.value = inner;
    return v;
}

// getters go hard

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

const char *crous_value_get_string(const crous_value *v, size_t *out_len) {
    if (v && v->type == CROUS_TYPE_STRING) {
        if (out_len) *out_len = v->data.s.len;
        return v->data.s.data;
    }
    if (out_len) *out_len = 0;
    return NULL;
}

const uint8_t *crous_value_get_bytes(const crous_value *v, size_t *out_len) {
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

const crous_value *crous_value_get_tagged_inner(const crous_value *v) {
    if (v && v->type == CROUS_TYPE_TAGGED) return v->data.tagged.value;
    return NULL;
}

size_t crous_value_list_size(const crous_value *v) {
    return (v && (v->type == CROUS_TYPE_LIST || v->type == CROUS_TYPE_TUPLE)) ? v->data.list.len : 0;
}

crous_value *crous_value_list_get(const crous_value *v, size_t index) {
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
    
    if (new_len > (v->data.list.items ? (1UL << 30) : 0)) {
        size_t new_cap = (v->data.list.len == 0) ? 8 : v->data.list.len * 2;
        crous_value **new_items = realloc(v->data.list.items, new_cap * sizeof(crous_value *));
        if (!new_items) return CROUS_ERR_OOM;
        v->data.list.items = new_items;
    }
    
    v->data.list.items[v->data.list.len] = item;
    v->data.list.len = new_len;
    return CROUS_OK;
}

size_t crous_value_dict_size(const crous_value *v) {
    return (v && v->type == CROUS_TYPE_DICT) ? v->data.dict.len : 0;
}

crous_value *crous_value_dict_get(const crous_value *v, const char *key) {
    if (!v || v->type != CROUS_TYPE_DICT || !key) return NULL;
    
    for (size_t i = 0; i < v->data.dict.len; i++) {
        if (strcmp(v->data.dict.entries[i].key, key) == 0)
            return v->data.dict.entries[i].value;
    }
    return NULL;
}

crous_err_t crous_value_dict_set(crous_value *v, const char *key, crous_value *value) {
    if (!v || v->type != CROUS_TYPE_DICT || !key)
        return CROUS_ERR_INVALID_TYPE;
    
    for (size_t i = 0; i < v->data.dict.len; i++) {
        if (strcmp(v->data.dict.entries[i].key, key) == 0) {
            v->data.dict.entries[i].value = value;
            return CROUS_OK;
        }
    }
    
    size_t new_len = v->data.dict.len + 1;
    if (new_len < v->data.dict.len) return CROUS_ERR_OVERFLOW;
    
    size_t new_cap = (v->data.dict.len == 0) ? 8 : v->data.dict.len * 2;
    crous_dict_entry *new_entries = realloc(v->data.dict.entries, new_cap * sizeof(crous_dict_entry));
    if (!new_entries) return CROUS_ERR_OOM;
    v->data.dict.entries = new_entries;
    
    char *key_copy = malloc(strlen(key) + 1);
    if (!key_copy) return CROUS_ERR_OOM;
    strcpy(key_copy, key);
    
    v->data.dict.entries[v->data.dict.len].key = key_copy;
    v->data.dict.entries[v->data.dict.len].key_len = strlen(key);
    v->data.dict.entries[v->data.dict.len].value = value;
    v->data.dict.len = new_len;
    
    return CROUS_OK;
}

const crous_dict_entry *crous_value_dict_get_entry(const crous_value *v, size_t index) {
    if (!v || v->type != CROUS_TYPE_DICT || index >= v->data.dict.len) return NULL;
    return &v->data.dict.entries[index];
}

// encode that data babyyy

static crous_err_t encode_value_to_stream(const crous_value *v, crous_output_stream *out);

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

crous_err_t crous_encode_stream(
    crous_context *ctx,
    const crous_value *value,
    crous_output_stream *out) {
    (void)ctx;
    
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

crous_err_t crous_encode_value_to_stream(
    crous_context *ctx,
    const crous_value *value,
    crous_output_stream *out) {
    (void)ctx;
    return encode_value_to_stream(value, out);
}

// decode that data innit

static crous_err_t decode_value_from_stream(crous_input_stream *in, crous_value **out_value, int depth);

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

static int is_utf8_valid(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; ) {
        uint8_t byte = data[i];
        int cont_bytes = 0;
        
        if ((byte & 0x80) == 0) {
            i++;
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
            
            const char *key_str = crous_value_get_string(key_val, NULL);
            if (crous_value_dict_set(v, key_str, val_val) != CROUS_OK) {
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

crous_err_t crous_decode_stream(
    crous_context *ctx,
    crous_input_stream *in,
    crous_value **out_value) {
    (void)ctx;
    
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

crous_err_t crous_decode_value_from_stream(
    crous_context *ctx,
    crous_input_stream *in,
    crous_value **out_value) {
    (void)ctx;
    return decode_value_from_stream(in, out_value, 0);
}

// classic api energy

crous_err_t crous_encode(crous_context *ctx, const crous_value *value,
                         uint8_t **out_buf, size_t *out_size) {
    crous_output_stream out = crous_buffer_output_stream_create();
    if (out.user_data == NULL) return CROUS_ERR_OOM;
    
    crous_err_t err = crous_encode_stream(ctx, value, &out);
    if (err != CROUS_OK) {
        crous_buffer_output_stream_free(&out);
        return err;
    }
    
    *out_buf = crous_buffer_output_stream_data(&out, out_size);
    crous_buffer_output_stream_free(&out);
    
    if (*out_buf == NULL) return CROUS_ERR_OOM;
    return CROUS_OK;
}

crous_err_t crous_decode(crous_context *ctx, const uint8_t *buf, size_t buf_size,
                         crous_value **out_value) {
    crous_input_stream in = crous_buffer_input_stream_create(buf, buf_size);
    if (in.user_data == NULL) return CROUS_ERR_OOM;
    
    crous_err_t err = crous_decode_stream(ctx, &in, out_value);
    free(in.user_data);
    return err;
}

// file time fr

crous_err_t crous_encode_file(crous_context *ctx, const crous_value *value,
                               const char *path) {
    uint8_t *buf = NULL;
    size_t size = 0;
    
    crous_err_t err = crous_encode(ctx, value, &buf, &size);
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

crous_err_t crous_decode_file(crous_context *ctx, const char *path,
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
    
    crous_err_t err = crous_decode(ctx, buf, file_size, out_value);
    free(buf);
    
    return err;
}

// cleanup szn fr fr

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