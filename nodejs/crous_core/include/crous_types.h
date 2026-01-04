#ifndef CROUS_TYPES_H
#define CROUS_TYPES_H

#include <stdint.h>
#include <stddef.h>

/* ============================================================================
   CORE DATA STRUCTURES
   ============================================================================ */

/* Forward declarations */
typedef struct crous_value crous_value;
typedef struct crous_dict_entry crous_dict_entry;
typedef struct crous_list crous_list;
typedef struct crous_dict crous_dict;

/* ============================================================================
   ENUMS
   ============================================================================ */

/* Value type enumeration */
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
    CROUS_TYPE_TAGGED,
} crous_type_t;

/* Error codes */
typedef enum {
    CROUS_OK = 0,
    CROUS_ERR_INVALID_TYPE = 1,
    CROUS_ERR_DECODE = 2,
    CROUS_ERR_ENCODE = 3,
    CROUS_ERR_OOM = 4,
    CROUS_ERR_OVERFLOW = 5,
    CROUS_ERR_INTERNAL = 6,
    CROUS_ERR_STREAM = 7,
    CROUS_ERR_TAG_UNKNOWN = 8,
    CROUS_ERR_TRUNCATED = 9,
    CROUS_ERR_INVALID_HEADER = 10,
    CROUS_ERR_SYNTAX = 11,
    CROUS_ERR_DEPTH_EXCEEDED = 12,
} crous_err_t;

/* ============================================================================
   STREAM INTERFACES
   ============================================================================ */

/* Input stream for decoding */
typedef struct {
    void* user_data;
    size_t (*read)(void* user_data, uint8_t* buf, size_t max_len);
} crous_input_stream;

/* Output stream for encoding */
typedef struct {
    void* user_data;
    size_t (*write)(void* user_data, const uint8_t* buf, size_t len);
} crous_output_stream;

/* ============================================================================
   VALUE STRUCTURES
   ============================================================================ */

/* Tagged value for extended types (datetime, Decimal, UUID, set, etc.) */
typedef struct {
    uint32_t tag;
    crous_value *value;
} crous_tagged_t;

/* Dictionary entry (key-value pair) */
struct crous_dict_entry {
    char *key;
    size_t key_len;
    crous_value *value;
};

/* List/Tuple structure */
struct crous_list {
    crous_value **items;
    size_t len;
    size_t cap;
};

/* Dictionary structure */
struct crous_dict {
    crous_dict_entry *entries;
    size_t len;
    size_t cap;
};

/* String/Bytes structure */
typedef struct {
    uint8_t *data;
    size_t len;
} crous_buffer_t;

/* Value union */
typedef union {
    int b;                      /* bool */
    int64_t i;                  /* int64 */
    double f;                   /* float64 */
    crous_buffer_t s;           /* string */
    crous_buffer_t bytes;       /* bytes */
    crous_list list;            /* list/tuple */
    crous_dict dict;            /* dict */
    crous_tagged_t tagged;      /* tagged value */
} crous_value_data_t;

/* Main value structure */
struct crous_value {
    crous_type_t type;
    crous_value_data_t data;
};

/* ============================================================================
   CONSTANTS
   ============================================================================ */

#define CROUS_MAX_DEPTH 256
#define CROUS_MAX_STRING_BYTES (1UL << 30)
#define CROUS_MAX_BYTES_SIZE (1UL << 30)
#define CROUS_MAX_LIST_SIZE (1UL << 30)
#define CROUS_MAX_DICT_SIZE (1UL << 30)

#define CROUS_MAGIC_0 0x43  /* 'C' */
#define CROUS_MAGIC_1 0x52  /* 'R' */
#define CROUS_MAGIC_2 0x4F  /* 'O' */
#define CROUS_MAGIC_3 0x55  /* 'U' */
#define CROUS_VERSION 2

#endif /* CROUS_TYPES_H */
