#include "../include/crous_flux.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============================================================================
   FLUX LEXER IMPLEMENTATION
   ============================================================================ */

typedef struct {
    flux_token_t token;
    int next_indent;
    int prev_indent;
} flux_lexer_internal_t;

struct flux_lexer {
    const char *text;
    size_t text_len;
    size_t pos;
    int line;
    int column;
    int indent_level;
    flux_token_t current_token;
    int indent_stack_size;
    int indent_stack[256];  /* Maximum 256 indentation levels */
};

flux_lexer_t* flux_lexer_new(const char *text, size_t text_len) {
    flux_lexer_t *lexer = malloc(sizeof(flux_lexer_t));
    if (!lexer) return NULL;
    
    lexer->text = text;
    lexer->text_len = text_len;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->indent_level = 0;
    lexer->indent_stack_size = 1;
    lexer->indent_stack[0] = 0;
    
    return lexer;
}

void flux_lexer_free(flux_lexer_t *lexer) {
    if (lexer) free(lexer);
}

static int is_identifier_char(char c) {
    return isalnum(c) || c == '_';
}

static int is_identifier_start(char c) {
    return isalpha(c) || c == '_';
}

static void skip_whitespace_same_line(flux_lexer_t *lexer) {
    while (lexer->pos < lexer->text_len && 
           (lexer->text[lexer->pos] == ' ' || lexer->text[lexer->pos] == '\t')) {
        lexer->pos++;
        lexer->column++;
    }
}

static void skip_comment(flux_lexer_t *lexer) {
    if (lexer->pos < lexer->text_len && lexer->text[lexer->pos] == '#') {
        while (lexer->pos < lexer->text_len && lexer->text[lexer->pos] != '\n') {
            lexer->pos++;
        }
    }
}

static flux_token_t* make_token(flux_lexer_t *lexer, flux_token_type_t type, 
                                const char *value, size_t value_len) {
    lexer->current_token.type = type;
    lexer->current_token.value = value;
    lexer->current_token.value_len = value_len;
    lexer->current_token.line = lexer->line;
    lexer->current_token.column = lexer->column;
    return &lexer->current_token;
}

static int count_indent(flux_lexer_t *lexer) {
    int indent = 0;
    while (lexer->pos < lexer->text_len && 
           (lexer->text[lexer->pos] == ' ' || lexer->text[lexer->pos] == '\t')) {
        indent += (lexer->text[lexer->pos] == '\t') ? 4 : 1;
        lexer->pos++;
        lexer->column++;
    }
    return indent;
}

flux_token_t* flux_lexer_next(flux_lexer_t *lexer) {
    /* Skip empty lines and comments */
    while (lexer->pos < lexer->text_len) {
        int line_start_pos = lexer->pos;
        int indent = 0;
        
        /* Count indentation at line start */
        if (lexer->column == 1) {
            indent = count_indent(lexer);
            skip_whitespace_same_line(lexer);
        }
        
        /* Skip empty lines and comments */
        if (lexer->pos < lexer->text_len && 
            (lexer->text[lexer->pos] == '\n' || lexer->text[lexer->pos] == '#')) {
            skip_comment(lexer);
            if (lexer->pos < lexer->text_len && lexer->text[lexer->pos] == '\n') {
                lexer->pos++;
                lexer->line++;
                lexer->column = 1;
            }
            continue;
        }
        
        /* Handle indentation changes */
        if (lexer->column == 1 && lexer->pos < lexer->text_len && 
            lexer->text[lexer->pos] != '\n') {
            if (indent > lexer->indent_stack[lexer->indent_stack_size - 1]) {
                lexer->indent_stack[lexer->indent_stack_size++] = indent;
                return make_token(lexer, FLUX_TOKEN_INDENT, NULL, 0);
            } else if (indent < lexer->indent_stack[lexer->indent_stack_size - 1]) {
                lexer->indent_stack_size--;
                return make_token(lexer, FLUX_TOKEN_DEDENT, NULL, 0);
            }
        }
        
        break;
    }
    
    /* Skip inline whitespace */
    skip_whitespace_same_line(lexer);
    
    if (lexer->pos >= lexer->text_len) {
        return make_token(lexer, FLUX_TOKEN_EOF, NULL, 0);
    }
    
    char c = lexer->text[lexer->pos];
    
    /* Newline */
    if (c == '\n') {
        lexer->pos++;
        lexer->line++;
        lexer->column = 1;
        return make_token(lexer, FLUX_TOKEN_NEWLINE, "\n", 1);
    }
    
    /* Colon */
    if (c == ':') {
        lexer->pos++;
        lexer->column++;
        return make_token(lexer, FLUX_TOKEN_COLON, ":", 1);
    }
    
    /* Brackets */
    if (c == '[') {
        lexer->pos++;
        lexer->column++;
        return make_token(lexer, FLUX_TOKEN_LBRACKET, "[", 1);
    }
    if (c == ']') {
        lexer->pos++;
        lexer->column++;
        return make_token(lexer, FLUX_TOKEN_RBRACKET, "]", 1);
    }
    
    /* Symbol reference */
    if (c == '@' && lexer->pos + 1 < lexer->text_len && 
        is_identifier_start(lexer->text[lexer->pos + 1])) {
        lexer->pos++;
        lexer->column++;
        const char *start = lexer->text + lexer->pos;
        while (lexer->pos < lexer->text_len && is_identifier_char(lexer->text[lexer->pos])) {
            lexer->pos++;
            lexer->column++;
        }
        return make_token(lexer, FLUX_TOKEN_SYMBOL, start, 
                         lexer->text + lexer->pos - start);
    }
    
    /* Quoted strings */
    if (c == '"' || c == '\'') {
        char quote = c;
        lexer->pos++;
        lexer->column++;
        const char *start = lexer->text + lexer->pos;
        while (lexer->pos < lexer->text_len && lexer->text[lexer->pos] != quote) {
            if (lexer->text[lexer->pos] == '\\' && lexer->pos + 1 < lexer->text_len) {
                lexer->pos += 2;
            } else {
                lexer->pos++;
            }
            lexer->column++;
        }
        size_t len = lexer->text + lexer->pos - start;
        if (lexer->pos < lexer->text_len) {
            lexer->pos++;  /* skip closing quote */
            lexer->column++;
        }
        return make_token(lexer, FLUX_TOKEN_STRING, start, len);
    }
    
    /* Numbers and identifiers */
    if (isdigit(c) || (c == '-' && lexer->pos + 1 < lexer->text_len && 
                      isdigit(lexer->text[lexer->pos + 1]))) {
        const char *start = lexer->text + lexer->pos;
        if (c == '-') {
            lexer->pos++;
            lexer->column++;
        }
        
        int has_dot = 0;
        int has_exp = 0;
        
        while (lexer->pos < lexer->text_len) {
            c = lexer->text[lexer->pos];
            if (isdigit(c)) {
                lexer->pos++;
                lexer->column++;
            } else if (c == '.' && !has_dot && !has_exp) {
                has_dot = 1;
                lexer->pos++;
                lexer->column++;
            } else if ((c == 'e' || c == 'E') && !has_exp) {
                has_exp = 1;
                lexer->pos++;
                lexer->column++;
                if (lexer->pos < lexer->text_len && 
                    (lexer->text[lexer->pos] == '+' || lexer->text[lexer->pos] == '-')) {
                    lexer->pos++;
                    lexer->column++;
                }
            } else {
                break;
            }
        }
        
        flux_token_type_t type = has_dot || has_exp ? FLUX_TOKEN_FLOAT : FLUX_TOKEN_INT;
        return make_token(lexer, type, start, lexer->text + lexer->pos - start);
    }
    
    /* Keywords and identifiers */
    if (is_identifier_start(c)) {
        const char *start = lexer->text + lexer->pos;
        while (lexer->pos < lexer->text_len && is_identifier_char(lexer->text[lexer->pos])) {
            lexer->pos++;
            lexer->column++;
        }
        size_t len = lexer->text + lexer->pos - start;
        
        /* Check for keywords */
        if (len == 4 && strncmp(start, "true", 4) == 0) {
            return make_token(lexer, FLUX_TOKEN_BOOL_TRUE, start, len);
        }
        if (len == 5 && strncmp(start, "false", 5) == 0) {
            return make_token(lexer, FLUX_TOKEN_BOOL_FALSE, start, len);
        }
        if (len == 4 && strncmp(start, "null", 4) == 0) {
            return make_token(lexer, FLUX_TOKEN_NULL, start, len);
        }
        
        return make_token(lexer, FLUX_TOKEN_KEY, start, len);
    }
    
    /* Unknown character */
    lexer->pos++;
    lexer->column++;
    return make_token(lexer, FLUX_TOKEN_ERROR, &c, 1);
}

flux_token_t* flux_lexer_peek(flux_lexer_t *lexer) {
    /* Save state */
    size_t saved_pos = lexer->pos;
    int saved_line = lexer->line;
    int saved_column = lexer->column;
    int saved_indent_size = lexer->indent_stack_size;
    int saved_indent_stack[256];
    memcpy(saved_indent_stack, lexer->indent_stack, sizeof(lexer->indent_stack));
    
    /* Get next token */
    flux_token_t *token = flux_lexer_next(lexer);
    flux_token_t result = *token;
    
    /* Restore state */
    lexer->pos = saved_pos;
    lexer->line = saved_line;
    lexer->column = saved_column;
    lexer->indent_stack_size = saved_indent_size;
    memcpy(lexer->indent_stack, saved_indent_stack, sizeof(lexer->indent_stack));
    
    return &lexer->current_token;
}

int flux_lexer_indent_level(flux_lexer_t *lexer) {
    return lexer->indent_stack[lexer->indent_stack_size - 1];
}
