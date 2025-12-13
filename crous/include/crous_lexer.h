#ifndef CROUS_LEXER_H
#define CROUS_LEXER_H

#include "crous_types.h"
#include "crous_token.h"
#include "crous_arena.h"

/* ============================================================================
   LEXER (TOKENIZER)
   ============================================================================ */

/* Opaque lexer structure */
typedef struct crous_lexer_s crous_lexer;

/**
 * Create lexer for input text
 */
crous_lexer* crous_lexer_create(const char *input, size_t input_len, crous_arena *arena);

/**
 * Get next token from lexer
 */
crous_token_t crous_lexer_next(crous_lexer *lexer);

/**
 * Peek at next token without consuming
 */
crous_token_t crous_lexer_peek(crous_lexer *lexer);

/**
 * Get current position in input
 */
size_t crous_lexer_pos(const crous_lexer *lexer);

/**
 * Get current line and column
 */
void crous_lexer_location(const crous_lexer *lexer, int *line, int *col);

#endif /* CROUS_LEXER_H */
