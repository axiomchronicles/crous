#include "../include/crous_lexer.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

struct crous_lexer_s {
    const char *input;
    size_t input_len;
    size_t pos;
    int line;
    int col;
    crous_arena *arena;
    crous_token_t peeked;
    int has_peeked;
};

crous_lexer* crous_lexer_create(const char *input, size_t input_len, crous_arena *arena) {
    crous_lexer *lexer = malloc(sizeof(*lexer));
    if (!lexer) return NULL;
    
    lexer->input = input;
    lexer->input_len = input_len;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->col = 1;
    lexer->arena = arena;
    lexer->has_peeked = 0;
    
    return lexer;
}

static void skip_whitespace(crous_lexer *lexer) {
    while (lexer->pos < lexer->input_len) {
        char c = lexer->input[lexer->pos];
        if (c == ' ' || c == '\t' || c == '\r') {
            lexer->pos++;
            lexer->col++;
        } else if (c == '\n') {
            lexer->pos++;
            lexer->line++;
            lexer->col = 1;
        } else if (c == '#') {
            /* Comment until end of line */
            while (lexer->pos < lexer->input_len && lexer->input[lexer->pos] != '\n') {
                lexer->pos++;
            }
        } else {
            break;
        }
    }
}

static crous_token_t make_token(crous_lexer *lexer, crous_token_type_t type, const char *start, size_t len) {
    return crous_token_make(type, start, len, lexer->line, lexer->col - len);
}

static int is_identifier_start(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_identifier_char(char c) {
    return is_identifier_start(c) || (c >= '0' && c <= '9');
}

static crous_token_t scan_number(crous_lexer *lexer) {
    const char *start = lexer->input + lexer->pos;
    int line = lexer->line;
    int col = lexer->col;
    
    /* Handle negative numbers */
    if (lexer->input[lexer->pos] == '-') {
        lexer->pos++;
        lexer->col++;
    }
    
    /* Scan digits */
    while (lexer->pos < lexer->input_len && isdigit(lexer->input[lexer->pos])) {
        lexer->pos++;
        lexer->col++;
    }
    
    /* Check for float */
    if (lexer->pos < lexer->input_len && lexer->input[lexer->pos] == '.') {
        lexer->pos++;
        lexer->col++;
        
        while (lexer->pos < lexer->input_len && isdigit(lexer->input[lexer->pos])) {
            lexer->pos++;
            lexer->col++;
        }
        
        /* Handle exponent */
        if (lexer->pos < lexer->input_len && (lexer->input[lexer->pos] == 'e' || lexer->input[lexer->pos] == 'E')) {
            lexer->pos++;
            lexer->col++;
            
            if (lexer->pos < lexer->input_len && (lexer->input[lexer->pos] == '+' || lexer->input[lexer->pos] == '-')) {
                lexer->pos++;
                lexer->col++;
            }
            
            while (lexer->pos < lexer->input_len && isdigit(lexer->input[lexer->pos])) {
                lexer->pos++;
                lexer->col++;
            }
        }
        
        return crous_token_make(CROUS_TOK_FLOAT, start, lexer->pos - (start - lexer->input), line, col);
    }
    
    return crous_token_make(CROUS_TOK_INT, start, lexer->pos - (start - lexer->input), line, col);
}

static crous_token_t scan_string(crous_lexer *lexer, char quote) {
    const char *start = lexer->input + lexer->pos;
    int line = lexer->line;
    int col = lexer->col;
    
    lexer->pos++;  /* Skip opening quote */
    lexer->col++;
    
    while (lexer->pos < lexer->input_len) {
        char c = lexer->input[lexer->pos];
        
        if (c == quote) {
            lexer->pos++;
            lexer->col++;
            return crous_token_make(CROUS_TOK_STRING, start, lexer->pos - (start - lexer->input), line, col);
        }
        
        if (c == '\\') {
            lexer->pos++;
            lexer->col++;
            if (lexer->pos < lexer->input_len) {
                lexer->pos++;
                lexer->col++;
            }
        } else if (c == '\n') {
            lexer->line++;
            lexer->col = 1;
            lexer->pos++;
        } else {
            lexer->pos++;
            lexer->col++;
        }
    }
    
    /* Unterminated string */
    return crous_token_make(CROUS_TOK_ERROR, start, lexer->pos - (start - lexer->input), line, col);
}

static crous_token_t scan_identifier(crous_lexer *lexer) {
    const char *start = lexer->input + lexer->pos;
    int line = lexer->line;
    int col = lexer->col;
    
    while (lexer->pos < lexer->input_len && is_identifier_char(lexer->input[lexer->pos])) {
        lexer->pos++;
        lexer->col++;
    }
    
    size_t len = lexer->pos - (start - lexer->input);
    
    /* Check for keywords */
    if (len == 4 && strncmp(start, "null", 4) == 0) {
        return crous_token_make(CROUS_TOK_NULL, start, len, line, col);
    }
    if (len == 4 && strncmp(start, "true", 4) == 0) {
        return crous_token_make(CROUS_TOK_BOOL_TRUE, start, len, line, col);
    }
    if (len == 5 && strncmp(start, "false", 5) == 0) {
        return crous_token_make(CROUS_TOK_BOOL_FALSE, start, len, line, col);
    }
    if (len == 5 && strncmp(start, "bytes", 5) == 0) {
        return crous_token_make(CROUS_TOK_BYTES, start, len, line, col);
    }
    
    return crous_token_make(CROUS_TOK_ERROR, start, len, line, col);
}

static crous_token_t scan_tagged(crous_lexer *lexer) {
    const char *start = lexer->input + lexer->pos;
    int line = lexer->line;
    int col = lexer->col;
    
    lexer->pos++;  /* Skip @ */
    lexer->col++;
    
    if (lexer->pos < lexer->input_len && is_identifier_start(lexer->input[lexer->pos])) {
        while (lexer->pos < lexer->input_len && is_identifier_char(lexer->input[lexer->pos])) {
            lexer->pos++;
            lexer->col++;
        }
        return crous_token_make(CROUS_TOK_TAGGED, start, lexer->pos - (start - lexer->input), line, col);
    }
    
    return crous_token_make(CROUS_TOK_ERROR, start, 1, line, col);
}

crous_token_t crous_lexer_next(crous_lexer *lexer) {
    if (lexer->has_peeked) {
        lexer->has_peeked = 0;
        return lexer->peeked;
    }
    
    skip_whitespace(lexer);
    
    if (lexer->pos >= lexer->input_len) {
        return crous_token_make(CROUS_TOK_EOF, lexer->input + lexer->pos, 0, lexer->line, lexer->col);
    }
    
    const char *start = lexer->input + lexer->pos;
    char c = lexer->input[lexer->pos];
    int line = lexer->line;
    int col = lexer->col;
    
    /* Single character tokens */
    if (c == '[') {
        lexer->pos++;
        lexer->col++;
        return crous_token_make(CROUS_TOK_LBRACKET, start, 1, line, col);
    }
    if (c == ']') {
        lexer->pos++;
        lexer->col++;
        return crous_token_make(CROUS_TOK_RBRACKET, start, 1, line, col);
    }
    if (c == '(') {
        lexer->pos++;
        lexer->col++;
        return crous_token_make(CROUS_TOK_LPAREN, start, 1, line, col);
    }
    if (c == ')') {
        lexer->pos++;
        lexer->col++;
        return crous_token_make(CROUS_TOK_RPAREN, start, 1, line, col);
    }
    if (c == '{') {
        lexer->pos++;
        lexer->col++;
        return crous_token_make(CROUS_TOK_LBRACE, start, 1, line, col);
    }
    if (c == '}') {
        lexer->pos++;
        lexer->col++;
        return crous_token_make(CROUS_TOK_RBRACE, start, 1, line, col);
    }
    if (c == ':') {
        lexer->pos++;
        lexer->col++;
        return crous_token_make(CROUS_TOK_COLON, start, 1, line, col);
    }
    if (c == ',') {
        lexer->pos++;
        lexer->col++;
        return crous_token_make(CROUS_TOK_COMMA, start, 1, line, col);
    }
    
    /* Numbers */
    if ((c >= '0' && c <= '9') || (c == '-' && lexer->pos + 1 < lexer->input_len && 
        isdigit(lexer->input[lexer->pos + 1]))) {
        return scan_number(lexer);
    }
    
    /* Strings */
    if (c == '"' || c == '\'') {
        return scan_string(lexer, c);
    }
    
    /* Tagged values */
    if (c == '@') {
        return scan_tagged(lexer);
    }
    
    /* Identifiers/keywords */
    if (is_identifier_start(c)) {
        return scan_identifier(lexer);
    }
    
    /* Unknown character */
    lexer->pos++;
    lexer->col++;
    return crous_token_make(CROUS_TOK_ERROR, start, 1, line, col);
}

crous_token_t crous_lexer_peek(crous_lexer *lexer) {
    if (!lexer->has_peeked) {
        lexer->peeked = crous_lexer_next(lexer);
        lexer->has_peeked = 1;
    }
    return lexer->peeked;
}

size_t crous_lexer_pos(const crous_lexer *lexer) {
    return lexer->pos;
}

void crous_lexer_location(const crous_lexer *lexer, int *line, int *col) {
    if (line) *line = lexer->line;
    if (col) *col = lexer->col;
}
