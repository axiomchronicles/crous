#ifndef CROUS_TOKEN_H
#define CROUS_TOKEN_H

#include "crous_types.h"

/* ============================================================================
   TOKEN DEFINITIONS
   ============================================================================ */

/* Token types for lexer */
typedef enum {
    CROUS_TOK_EOF = 0,
    CROUS_TOK_NULL,
    CROUS_TOK_BOOL_FALSE,
    CROUS_TOK_BOOL_TRUE,
    CROUS_TOK_INT,
    CROUS_TOK_FLOAT,
    CROUS_TOK_STRING,
    CROUS_TOK_BYTES,
    CROUS_TOK_LBRACKET,     /* [ */
    CROUS_TOK_RBRACKET,     /* ] */
    CROUS_TOK_LPAREN,       /* ( */
    CROUS_TOK_RPAREN,       /* ) */
    CROUS_TOK_LBRACE,       /* { */
    CROUS_TOK_RBRACE,       /* } */
    CROUS_TOK_COLON,        /* : */
    CROUS_TOK_COMMA,        /* , */
    CROUS_TOK_TAGGED,       /* @tag */
    CROUS_TOK_ERROR,
} crous_token_type_t;

/* Token structure */
typedef struct {
    crous_token_type_t type;
    const char *start;
    size_t len;
    int line;
    int col;
    /* Token value storage for literals */
    union {
        int64_t i;
        double f;
        const char *s;
    } value;
} crous_token_t;

/**
 * Create a token
 */
crous_token_t crous_token_make(crous_token_type_t type, const char *start, size_t len, int line, int col);

/**
 * Get token type name
 */
const char* crous_token_type_str(crous_token_type_t type);

#endif /* CROUS_TOKEN_H */
