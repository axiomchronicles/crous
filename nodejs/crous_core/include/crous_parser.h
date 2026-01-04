#ifndef CROUS_PARSER_H
#define CROUS_PARSER_H

#include "crous_types.h"
#include "crous_lexer.h"
#include "crous_arena.h"

/* ============================================================================
   PARSER
   ============================================================================ */

/* Opaque parser structure */
typedef struct crous_parser_s crous_parser;

/**
 * Create parser from lexer
 */
crous_parser* crous_parser_create(crous_lexer *lexer, crous_arena *arena);

/**
 * Parse complete value from token stream
 */
crous_err_t crous_parser_parse(crous_parser *parser, crous_value **out_value);

/**
 * Get last error from parser
 */
crous_err_t crous_parser_error(const crous_parser *parser);

/**
 * Get error position
 */
void crous_parser_error_location(const crous_parser *parser, int *line, int *col);

#endif /* CROUS_PARSER_H */
