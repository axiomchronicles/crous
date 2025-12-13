#include "../include/crous_token.h"
#include <string.h>

crous_token_t crous_token_make(crous_token_type_t type, const char *start, size_t len, int line, int col) {
    crous_token_t tok;
    tok.type = type;
    tok.start = start;
    tok.len = len;
    tok.line = line;
    tok.col = col;
    tok.value.s = NULL;
    return tok;
}

const char* crous_token_type_str(crous_token_type_t type) {
    switch (type) {
        case CROUS_TOK_EOF: return "EOF";
        case CROUS_TOK_NULL: return "null";
        case CROUS_TOK_BOOL_FALSE: return "false";
        case CROUS_TOK_BOOL_TRUE: return "true";
        case CROUS_TOK_INT: return "int";
        case CROUS_TOK_FLOAT: return "float";
        case CROUS_TOK_STRING: return "string";
        case CROUS_TOK_BYTES: return "bytes";
        case CROUS_TOK_LBRACKET: return "[";
        case CROUS_TOK_RBRACKET: return "]";
        case CROUS_TOK_LPAREN: return "(";
        case CROUS_TOK_RPAREN: return ")";
        case CROUS_TOK_LBRACE: return "{";
        case CROUS_TOK_RBRACE: return "}";
        case CROUS_TOK_COLON: return ":";
        case CROUS_TOK_COMMA: return ",";
        case CROUS_TOK_TAGGED: return "@";
        case CROUS_TOK_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
