#include "../include/crous_parser.h"
#include "../include/crous_value.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
            /* For now, just use token content. Proper escape handling would be needed. */
            crous_value *v = crous_value_new_string(tok.start + 1, tok.len - 2);
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
            /* Parse @tag value */
            uint32_t tag = 0;
            // TODO: Extract tag from token
            
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
