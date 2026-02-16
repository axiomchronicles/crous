#include "../include/crous_parser.h"
#include "../include/crous_value.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

/* ============================================================================
   STRING ESCAPE HANDLING
   ============================================================================ */

/**
 * Process escape sequences in a string.
 * Handles: \n, \t, \r, \\, \", \', \0, \xNN, \uXXXX
 * Returns: newly allocated string with escapes processed, or NULL on error.
 * Sets *out_len to the length of the processed string.
 */
static char* process_escapes(const char *src, size_t src_len, size_t *out_len) {
    /* Allocate buffer (escaped string is at most as long as source) */
    char *result = malloc(src_len + 1);
    if (!result) return NULL;
    
    size_t j = 0;  /* Output index */
    
    for (size_t i = 0; i < src_len; i++) {
        if (src[i] == '\\' && i + 1 < src_len) {
            char next = src[i + 1];
            switch (next) {
                case 'n':  result[j++] = '\n'; i++; break;
                case 't':  result[j++] = '\t'; i++; break;
                case 'r':  result[j++] = '\r'; i++; break;
                case '\\': result[j++] = '\\'; i++; break;
                case '"':  result[j++] = '"'; i++; break;
                case '\'': result[j++] = '\''; i++; break;
                case '0':  result[j++] = '\0'; i++; break;
                case 'b':  result[j++] = '\b'; i++; break;
                case 'f':  result[j++] = '\f'; i++; break;
                case 'v':  result[j++] = '\v'; i++; break;
                case 'x': {
                    /* Hex escape: \xNN */
                    if (i + 3 < src_len && isxdigit(src[i + 2]) && isxdigit(src[i + 3])) {
                        char hex[3] = {src[i + 2], src[i + 3], '\0'};
                        result[j++] = (char)strtol(hex, NULL, 16);
                        i += 3;
                    } else {
                        result[j++] = src[i];
                    }
                    break;
                }
                case 'u': {
                    /* Unicode escape: \uXXXX */
                    if (i + 5 < src_len && 
                        isxdigit(src[i + 2]) && isxdigit(src[i + 3]) &&
                        isxdigit(src[i + 4]) && isxdigit(src[i + 5])) {
                        char hex[5] = {src[i + 2], src[i + 3], src[i + 4], src[i + 5], '\0'};
                        uint32_t codepoint = (uint32_t)strtol(hex, NULL, 16);
                        
                        /* Encode as UTF-8 */
                        if (codepoint < 0x80) {
                            result[j++] = (char)codepoint;
                        } else if (codepoint < 0x800) {
                            result[j++] = (char)(0xC0 | (codepoint >> 6));
                            result[j++] = (char)(0x80 | (codepoint & 0x3F));
                        } else {
                            result[j++] = (char)(0xE0 | (codepoint >> 12));
                            result[j++] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
                            result[j++] = (char)(0x80 | (codepoint & 0x3F));
                        }
                        i += 5;
                    } else {
                        result[j++] = src[i];
                    }
                    break;
                }
                default:
                    /* Unknown escape, keep as-is */
                    result[j++] = src[i];
                    break;
            }
        } else {
            result[j++] = src[i];
        }
    }
    
    result[j] = '\0';
    *out_len = j;
    return result;
}

/**
 * Parse tag number from @tag token (e.g., "@123" or "@datetime")
 * Returns tag number, or 0 if not a numeric tag
 */
static uint32_t parse_tag_number(const char *start, size_t len) {
    /* Skip the @ symbol */
    if (len <= 1 || start[0] != '@') return 0;
    
    const char *tag_str = start + 1;
    size_t tag_len = len - 1;
    
    /* Check if it's a numeric tag */
    int all_digits = 1;
    for (size_t i = 0; i < tag_len; i++) {
        if (!isdigit(tag_str[i])) {
            all_digits = 0;
            break;
        }
    }
    
    if (all_digits && tag_len > 0) {
        char buf[16];
        size_t copy_len = tag_len < 15 ? tag_len : 15;
        memcpy(buf, tag_str, copy_len);
        buf[copy_len] = '\0';
        return (uint32_t)strtoul(buf, NULL, 10);
    }
    
    /* Named tags - return hash-based ID */
    /* Common named tags */
    if (tag_len == 8 && strncmp(tag_str, "datetime", 8) == 0) return 80;
    if (tag_len == 4 && strncmp(tag_str, "date", 4) == 0) return 81;
    if (tag_len == 4 && strncmp(tag_str, "time", 4) == 0) return 82;
    if (tag_len == 7 && strncmp(tag_str, "decimal", 7) == 0) return 83;
    if (tag_len == 4 && strncmp(tag_str, "uuid", 4) == 0) return 84;
    if (tag_len == 3 && strncmp(tag_str, "set", 3) == 0) return 90;
    if (tag_len == 9 && strncmp(tag_str, "frozenset", 9) == 0) return 91;
    if (tag_len == 4 && strncmp(tag_str, "path", 4) == 0) return 92;
    
    /* Default: use simple hash */
    uint32_t hash = 100;
    for (size_t i = 0; i < tag_len; i++) {
        hash = hash * 31 + (uint8_t)tag_str[i];
    }
    return hash;
}

struct crous_parser_s {
    crous_lexer *lexer;
    crous_arena *arena;
    crous_err_t last_error;
    int error_line;
    int error_col;
};

crous_parser* crous_parser_create(crous_lexer *lexer, crous_arena *arena) {
    crous_parser *parser = malloc(sizeof(*parser));
    if (!parser) return NULL;
    
    parser->lexer = lexer;
    parser->arena = arena;
    parser->last_error = CROUS_OK;
    parser->error_line = 0;
    parser->error_col = 0;
    
    return parser;
}

static crous_err_t parse_value(crous_parser *parser, crous_value **out_value, int depth);

static crous_err_t parse_list(crous_parser *parser, crous_value **out_value, int depth) {
    crous_value *list = crous_value_new_list(0);
    if (!list) return CROUS_ERR_OOM;
    
    crous_lexer_next(parser->lexer); /* consume [ */
    
    crous_token_t tok = crous_lexer_peek(parser->lexer);
    
    /* Empty list */
    if (tok.type == CROUS_TOK_RBRACKET) {
        crous_lexer_next(parser->lexer);
        *out_value = list;
        return CROUS_OK;
    }
    
    while (1) {
        crous_value *item = NULL;
        crous_err_t err = parse_value(parser, &item, depth + 1);
        if (err != CROUS_OK) {
            crous_value_free_tree(list);
            return err;
        }
        
        err = crous_value_list_append(list, item);
        if (err != CROUS_OK) {
            crous_value_free_tree(list);
            crous_value_free_tree(item);
            return err;
        }
        
        tok = crous_lexer_peek(parser->lexer);
        
        if (tok.type == CROUS_TOK_RBRACKET) {
            crous_lexer_next(parser->lexer);
            break;
        }
        
        if (tok.type == CROUS_TOK_COMMA) {
            crous_lexer_next(parser->lexer);
            tok = crous_lexer_peek(parser->lexer);
            
            /* Allow trailing comma */
            if (tok.type == CROUS_TOK_RBRACKET) {
                crous_lexer_next(parser->lexer);
                break;
            }
        } else {
            crous_value_free_tree(list);
            parser->error_line = tok.line;
            parser->error_col = tok.col;
            return CROUS_ERR_SYNTAX;
        }
    }
    
    *out_value = list;
    return CROUS_OK;
}

static crous_err_t parse_tuple(crous_parser *parser, crous_value **out_value, int depth) {
    crous_value *tuple = crous_value_new_tuple(0);
    if (!tuple) return CROUS_ERR_OOM;
    
    crous_lexer_next(parser->lexer); /* consume ( */
    
    crous_token_t tok = crous_lexer_peek(parser->lexer);
    
    /* Empty tuple */
    if (tok.type == CROUS_TOK_RPAREN) {
        crous_lexer_next(parser->lexer);
        *out_value = tuple;
        return CROUS_OK;
    }
    
    while (1) {
        crous_value *item = NULL;
        crous_err_t err = parse_value(parser, &item, depth + 1);
        if (err != CROUS_OK) {
            crous_value_free_tree(tuple);
            return err;
        }
        
        err = crous_value_list_append(tuple, item);
        if (err != CROUS_OK) {
            crous_value_free_tree(tuple);
            crous_value_free_tree(item);
            return err;
        }
        
        tok = crous_lexer_peek(parser->lexer);
        
        if (tok.type == CROUS_TOK_RPAREN) {
            crous_lexer_next(parser->lexer);
            break;
        }
        
        if (tok.type == CROUS_TOK_COMMA) {
            crous_lexer_next(parser->lexer);
            tok = crous_lexer_peek(parser->lexer);
            
            if (tok.type == CROUS_TOK_RPAREN) {
                crous_lexer_next(parser->lexer);
                break;
            }
        } else {
            crous_value_free_tree(tuple);
            parser->error_line = tok.line;
            parser->error_col = tok.col;
            return CROUS_ERR_SYNTAX;
        }
    }
    
    *out_value = tuple;
    return CROUS_OK;
}

static crous_err_t parse_dict(crous_parser *parser, crous_value **out_value, int depth) {
    crous_value *dict = crous_value_new_dict(0);
    if (!dict) return CROUS_ERR_OOM;
    
    crous_lexer_next(parser->lexer); /* consume { */
    
    crous_token_t tok = crous_lexer_peek(parser->lexer);
    
    /* Empty dict */
    if (tok.type == CROUS_TOK_RBRACE) {
        crous_lexer_next(parser->lexer);
        *out_value = dict;
        return CROUS_OK;
    }
    
    while (1) {
        tok = crous_lexer_next(parser->lexer);
        
        if (tok.type != CROUS_TOK_STRING) {
            crous_value_free_tree(dict);
            parser->error_line = tok.line;
            parser->error_col = tok.col;
            return CROUS_ERR_SYNTAX;
        }
        
        /* Extract key string */
        char *key = malloc(tok.len + 1);
        if (!key) {
            crous_value_free_tree(dict);
            return CROUS_ERR_OOM;
        }
        memcpy(key, tok.start, tok.len);
        key[tok.len] = '\0';
        
        tok = crous_lexer_next(parser->lexer);
        if (tok.type != CROUS_TOK_COLON) {
            free(key);
            crous_value_free_tree(dict);
            parser->error_line = tok.line;
            parser->error_col = tok.col;
            return CROUS_ERR_SYNTAX;
        }
        
        crous_value *value = NULL;
        crous_err_t err = parse_value(parser, &value, depth + 1);
        if (err != CROUS_OK) {
            free(key);
            crous_value_free_tree(dict);
            return err;
        }
        
        err = crous_value_dict_set(dict, key, value);
        free(key);
        if (err != CROUS_OK) {
            crous_value_free_tree(dict);
            crous_value_free_tree(value);
            return err;
        }
        
        tok = crous_lexer_peek(parser->lexer);
        
        if (tok.type == CROUS_TOK_RBRACE) {
            crous_lexer_next(parser->lexer);
            break;
        }
        
        if (tok.type == CROUS_TOK_COMMA) {
            crous_lexer_next(parser->lexer);
            tok = crous_lexer_peek(parser->lexer);
            
            if (tok.type == CROUS_TOK_RBRACE) {
                crous_lexer_next(parser->lexer);
                break;
            }
        } else {
            crous_value_free_tree(dict);
            parser->error_line = tok.line;
            parser->error_col = tok.col;
            return CROUS_ERR_SYNTAX;
        }
    }
    
    *out_value = dict;
    return CROUS_OK;
}

static crous_err_t parse_value(crous_parser *parser, crous_value **out_value, int depth) {
    if (depth > CROUS_MAX_DEPTH) {
        return CROUS_ERR_DEPTH_EXCEEDED;
    }
    
    crous_token_t tok = crous_lexer_next(parser->lexer);
    
    switch (tok.type) {
        case CROUS_TOK_NULL: {
            crous_value *v = crous_value_new_null();
            if (!v) return CROUS_ERR_OOM;
            *out_value = v;
            return CROUS_OK;
        }
        
        case CROUS_TOK_BOOL_TRUE: {
            crous_value *v = crous_value_new_bool(1);
            if (!v) return CROUS_ERR_OOM;
            *out_value = v;
            return CROUS_OK;
        }
        
        case CROUS_TOK_BOOL_FALSE: {
            crous_value *v = crous_value_new_bool(0);
            if (!v) return CROUS_ERR_OOM;
            *out_value = v;
            return CROUS_OK;
        }
        
        case CROUS_TOK_INT: {
            int64_t val = 0;
            char *endptr = NULL;
            val = strtoll(tok.start, &endptr, 10);
            if (errno == ERANGE) return CROUS_ERR_DECODE;
            
            crous_value *v = crous_value_new_int(val);
            if (!v) return CROUS_ERR_OOM;
            *out_value = v;
            return CROUS_OK;
        }
        
        case CROUS_TOK_FLOAT: {
            double val = strtod(tok.start, NULL);
            if (errno == ERANGE) return CROUS_ERR_DECODE;
            
            crous_value *v = crous_value_new_float(val);
            if (!v) return CROUS_ERR_OOM;
            *out_value = v;
            return CROUS_OK;
        }
        
        case CROUS_TOK_STRING: {
            /* Process escape sequences in the string */
            const char *str_content = tok.start + 1;  /* Skip opening quote */
            size_t str_len = tok.len - 2;             /* Exclude both quotes */
            
            size_t processed_len = 0;
            char *processed = process_escapes(str_content, str_len, &processed_len);
            if (!processed) return CROUS_ERR_OOM;
            
            crous_value *v = crous_value_new_string(processed, processed_len);
            free(processed);
            if (!v) return CROUS_ERR_OOM;
            *out_value = v;
            return CROUS_OK;
        }
        
        case CROUS_TOK_LBRACKET: {
            return parse_list(parser, out_value, depth);
        }
        
        case CROUS_TOK_LPAREN: {
            return parse_tuple(parser, out_value, depth);
        }
        
        case CROUS_TOK_LBRACE: {
            return parse_dict(parser, out_value, depth);
        }
        
        case CROUS_TOK_TAGGED: {
            /* Parse @tag value - extract tag number from token */
            uint32_t tag = parse_tag_number(tok.start, tok.len);
            
            crous_value *inner = NULL;
            crous_err_t err = parse_value(parser, &inner, depth + 1);
            if (err != CROUS_OK) return err;
            
            crous_value *v = crous_value_new_tagged(tag, inner);
            if (!v) {
                crous_value_free_tree(inner);
                return CROUS_ERR_OOM;
            }
            *out_value = v;
            return CROUS_OK;
        }
        
        default:
            parser->error_line = tok.line;
            parser->error_col = tok.col;
            return CROUS_ERR_SYNTAX;
    }
}

crous_err_t crous_parser_parse(crous_parser *parser, crous_value **out_value) {
    crous_err_t err = parse_value(parser, out_value, 0);
    if (err != CROUS_OK) {
        parser->last_error = err;
    }
    return err;
}

crous_err_t crous_parser_error(const crous_parser *parser) {
    return parser->last_error;
}

void crous_parser_error_location(const crous_parser *parser, int *line, int *col) {
    if (line) *line = parser->error_line;
    if (col) *col = parser->error_col;
}
