#ifndef CROUS_FLUX_H
#define CROUS_FLUX_H

#include "crous_types.h"

/**
 * FLUX: Flattened Unified eXchange Format
 * 
 * A hybrid text/binary serialization format optimizing for:
 * - 40-60% smaller than JSON
 * - Human-readable plaintext
 * - Efficient binary representation
 * - Stream processing and zero-copy
 * - Round-trip fidelity between text and binary forms
 */

/* ============================================================================
   FLUX LEXER
   ============================================================================ */

typedef enum {
    FLUX_TOKEN_EOF,
    FLUX_TOKEN_KEY,           /* identifier or quoted string */
    FLUX_TOKEN_STRING,        /* quoted string value */
    FLUX_TOKEN_INT,           /* integer literal */
    FLUX_TOKEN_FLOAT,         /* float literal */
    FLUX_TOKEN_BOOL_TRUE,
    FLUX_TOKEN_BOOL_FALSE,
    FLUX_TOKEN_NULL,
    FLUX_TOKEN_SYMBOL,        /* @identifier */
    FLUX_TOKEN_COLON,         /* : */
    FLUX_TOKEN_LBRACKET,      /* [ */
    FLUX_TOKEN_RBRACKET,      /* ] */
    FLUX_TOKEN_INDENT,
    FLUX_TOKEN_DEDENT,
    FLUX_TOKEN_NEWLINE,
    FLUX_TOKEN_COMMENT,
    FLUX_TOKEN_ERROR,
} flux_token_type_t;

typedef struct {
    flux_token_type_t type;
    const char *value;
    size_t value_len;
    int line;
    int column;
} flux_token_t;

typedef struct flux_lexer flux_lexer_t;

/**
 * Create a FLUX lexer from text input
 */
flux_lexer_t* flux_lexer_new(const char *text, size_t text_len);

/**
 * Free lexer resources
 */
void flux_lexer_free(flux_lexer_t *lexer);

/**
 * Get next token from lexer
 */
flux_token_t* flux_lexer_next(flux_lexer_t *lexer);

/**
 * Peek at next token without consuming
 */
flux_token_t* flux_lexer_peek(flux_lexer_t *lexer);

/**
 * Get current indentation level
 */
int flux_lexer_indent_level(flux_lexer_t *lexer);

/* ============================================================================
   FLUX PARSER
   ============================================================================ */

typedef struct flux_parser flux_parser_t;

/**
 * Create a FLUX parser from token stream
 */
flux_parser_t* flux_parser_new(flux_lexer_t *lexer);

/**
 * Free parser resources
 */
void flux_parser_free(flux_parser_t *parser);

/**
 * Parse FLUX text into a crous_value tree
 * Returns error code; value is set via out_value
 */
crous_err_t flux_parse(
    flux_parser_t *parser,
    crous_value **out_value);

/* ============================================================================
   FLUX SERIALIZER
   ============================================================================ */

/**
 * Serialize crous_value to FLUX text format
 * Produces human-readable output with proper indentation
 */
crous_err_t flux_serialize_text(
    const crous_value *value,
    crous_output_stream *out);

/**
 * Serialize crous_value to FLUX binary format
 * Binary form has same structure as text but optimized for space/speed
 * Format: [MAGIC:4][VERSION:1][BODY]
 */
crous_err_t flux_serialize_binary(
    const crous_value *value,
    crous_output_stream *out);

/* ============================================================================
   FLUX CONVENIENCE API
   ============================================================================ */

/**
 * Encode to FLUX text format (buffer)
 */
crous_err_t flux_encode_text(
    const crous_value *value,
    char **out_buf,
    size_t *out_size);

/**
 * Decode from FLUX text format (buffer)
 */
crous_err_t flux_decode_text(
    const char *buf,
    size_t buf_size,
    crous_value **out_value);

/**
 * Encode to FLUX binary format (buffer)
 */
crous_err_t flux_encode_binary(
    const crous_value *value,
    uint8_t **out_buf,
    size_t *out_size);

/**
 * Decode from FLUX binary format (buffer)
 */
crous_err_t flux_decode_binary(
    const uint8_t *buf,
    size_t buf_size,
    crous_value **out_value);

/* ============================================================================
   FLUX BINARY FORMAT MAGIC
   ============================================================================ */

#define FLUX_MAGIC_0 'F'
#define FLUX_MAGIC_1 'L'
#define FLUX_MAGIC_2 'U'
#define FLUX_MAGIC_3 'X'
#define FLUX_VERSION 1

#endif /* CROUS_FLUX_H */
