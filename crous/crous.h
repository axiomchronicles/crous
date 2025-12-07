#ifndef CROUS_H
#define CROUS_H

#include <stdint.h>
#include <stddef.h>

// types innit

typedef struct crous_context crous_context;
typedef struct crous_value crous_value;
typedef struct crous_dict_entry crous_dict_entry;

// error mode activated

typedef enum {
    CROUS_OK = 0,
    CROUS_ERR_INVALID_TYPE = 1,
    CROUS_ERR_DECODE = 2,
    CROUS_ERR_ENCODE = 3,
    CROUS_ERR_OOM = 4,
    CROUS_ERR_OVERFLOW = 5,
    CROUS_ERR_INTERNAL = 6,
    CROUS_ERR_STREAM = 7,           /* Stream read/write failed */
    CROUS_ERR_TAG_UNKNOWN = 8,      /* Unknown tag in tagged value */
    CROUS_ERR_TRUNCATED = 9,        /* Input truncated */
    CROUS_ERR_INVALID_HEADER = 10,  /* Invalid file header */
} crous_err_t;

// the type vibes

typedef enum {
    CROUS_TYPE_NULL = 0,
    CROUS_TYPE_BOOL,
    CROUS_TYPE_INT,
    CROUS_TYPE_FLOAT,
    CROUS_TYPE_STRING,
    CROUS_TYPE_BYTES,
    CROUS_TYPE_LIST,
    CROUS_TYPE_TUPLE,
    CROUS_TYPE_DICT,
    CROUS_TYPE_TAGGED,  /* NEW: Tagged value (for extended types) */
} crous_type_t;

// stream szn

/**
 * Input stream interface for decoding.
 * user_data: Opaque pointer to stream state.
 * read: Read up to max_len bytes. Return bytes read, 0 for EOF, (size_t)-1 for error.
 */
typedef struct {
    void* user_data;
    size_t (*read)(void* user_data, uint8_t* buf, size_t max_len);
} crous_input_stream;

/**
 * Output stream interface for encoding.
 * user_data: Opaque pointer to stream state.
 * write: Write buf bytes. Return bytes written, (size_t)-1 for error.
 */
typedef struct {
    void* user_data;
    size_t (*write)(void* user_data, const uint8_t* buf, size_t len);
} crous_output_stream;

// value tree fr

/**
 * Tagged value (for datetime, Decimal, UUID, set, etc.)
 */
typedef struct {
    uint32_t tag;        /* Tag identifier (1-7 built-in, 100-199 user) */
    crous_value *value;  /* Inner value (already encoded) */
} crous_tagged;

/**
 * Dictionary entry (key-value pair)
 */
typedef struct crous_dict_entry {
    char *key;
    size_t key_len;
    crous_value *value;
} crous_dict_entry;

/**
 * Value union for different types
 */
typedef union {
    int b;
    int64_t i;
    double f;
    struct {
        char *data;
        size_t len;
    } s;
    struct {
        uint8_t *data;
        size_t len;
    } bytes;
    struct {
        crous_value **items;
        size_t len;
    } list;
    struct {
        crous_dict_entry *entries;
        size_t len;
    } dict;
    crous_tagged tagged;  /* NEW */
} crous_value_data;

/**
 * Main value structure
 */
struct crous_value {
    crous_type_t type;
    crous_value_data data;
};

// context energy

crous_context *crous_context_new(void);
void crous_context_free(crous_context *ctx);

// make new values go brrr

crous_value *crous_value_new_null(void);
crous_value *crous_value_new_bool(int b);
crous_value *crous_value_new_int(int64_t v);
crous_value *crous_value_new_float(double d);
crous_value *crous_value_new_string(const char *data, size_t len);
crous_value *crous_value_new_bytes(const uint8_t *data, size_t len);
crous_value *crous_value_new_list(size_t capacity);
crous_value *crous_value_new_tuple(size_t capacity);
crous_value *crous_value_new_dict(size_t capacity);
crous_value *crous_value_new_tagged(uint32_t tag, crous_value *inner);

// get the goods

crous_type_t crous_value_get_type(const crous_value *v);
int crous_value_get_bool(const crous_value *v);
int64_t crous_value_get_int(const crous_value *v);
double crous_value_get_float(const crous_value *v);
const char *crous_value_get_string(const crous_value *v, size_t *out_len);
const uint8_t *crous_value_get_bytes(const crous_value *v, size_t *out_len);

/* Tagged value accessors */
uint32_t crous_value_get_tag(const crous_value *v);
const crous_value *crous_value_get_tagged_inner(const crous_value *v);

/* List accessors */
size_t crous_value_list_size(const crous_value *v);
crous_value *crous_value_list_get(const crous_value *v, size_t index);
crous_err_t crous_value_list_set(crous_value *v, size_t index, crous_value *item);
crous_err_t crous_value_list_append(crous_value *v, crous_value *item);

/* Dict accessors */
size_t crous_value_dict_size(const crous_value *v);
crous_value *crous_value_dict_get(const crous_value *v, const char *key);
crous_err_t crous_value_dict_set(crous_value *v, const char *key, crous_value *value);
const crous_dict_entry *crous_value_dict_get_entry(const crous_value *v, size_t index);

// classic buffer style

crous_err_t crous_encode(crous_context *ctx, const crous_value *value,
                         uint8_t **out_buf, size_t *out_size);

crous_err_t crous_decode(crous_context *ctx, const uint8_t *buf, size_t buf_size,
                         crous_value **out_value);

// streaming hittin different

/**
 * Encode a value using output stream interface.
 * Encodes file header + value.
 */
crous_err_t crous_encode_stream(
    crous_context *ctx,
    const crous_value *value,
    crous_output_stream *out
);

/**
 * Decode a value using input stream interface.
 * Reads and verifies file header, then decodes value.
 */
crous_err_t crous_decode_stream(
    crous_context *ctx,
    crous_input_stream *in,
    crous_value **out_value
);

/**
 * Encode just the value payload (no header) to stream.
 * Used for stream-of-values format.
 */
crous_err_t crous_encode_value_to_stream(
    crous_context *ctx,
    const crous_value *value,
    crous_output_stream *out
);

/**
 * Decode just the value payload (no header) from stream.
 * Used for stream-of-values format.
 */
crous_err_t crous_decode_value_from_stream(
    crous_context *ctx,
    crous_input_stream *in,
    crous_value **out_value
);

// buffer stream helpers no cap

/**
 * Create an input stream from a memory buffer.
 */
crous_input_stream crous_buffer_input_stream_create(
    const uint8_t *data,
    size_t len
);

/**
 * Create an output stream backed by a growable buffer.
 */
crous_output_stream crous_buffer_output_stream_create(void);

/**
 * Get data from a buffer output stream (takes ownership, must be freed).
 */
uint8_t *crous_buffer_output_stream_data(
    crous_output_stream *s,
    size_t *out_len
);

/**
 * Free a buffer output stream.
 */
void crous_buffer_output_stream_free(crous_output_stream *s);

// file szn

crous_err_t crous_encode_file(crous_context *ctx, const crous_value *value,
                               const char *path);

crous_err_t crous_decode_file(crous_context *ctx, const char *path,
                               crous_value **out_value);

// cleanup department

void crous_value_free_tree(crous_value *v);

// the limits are real

#define CROUS_MAX_DEPTH 256
#define CROUS_MAX_LIST_SIZE (1UL << 30)      /* 1 billion items */
#define CROUS_MAX_STRING_BYTES (1UL << 30)   /* 1 GB strings */
#define CROUS_MAX_BYTES_SIZE (1UL << 30)     /* 1 GB bytes */
#define CROUS_MAX_DICT_SIZE (1UL << 30)      /* 1 billion entries */

#endif /* CROUS_H */