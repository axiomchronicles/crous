#include "../include/crous_flux.h"
#include "../include/crous_value.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
   FLUX PARSER IMPLEMENTATION
   ============================================================================ */

struct flux_parser {
    flux_lexer_t *lexer;
    flux_token_t *current_token;
    flux_token_t *peeked_token;
    int error_line;
    int error_column;
    char *error_msg;
};

flux_parser_t* flux_parser_new(flux_lexer_t *lexer) {
    flux_parser_t *parser = malloc(sizeof(flux_parser_t));
    if (!parser) return NULL;
    
    parser->lexer = lexer;
    parser->current_token = flux_lexer_next(lexer);
    parser->peeked_token = NULL;
    parser->error_line = 0;
    parser->error_column = 0;
    parser->error_msg = NULL;
    
    return parser;
}

void flux_parser_free(flux_parser_t *parser) {
    if (parser) {
        if (parser->error_msg) free(parser->error_msg);
        free(parser);
    }
}

static void set_error(flux_parser_t *parser, const char *msg) {
    if (parser->error_msg) free(parser->error_msg);
    parser->error_msg = malloc(strlen(msg) + 1);
    if (parser->error_msg) strcpy(parser->error_msg, msg);
    parser->error_line = parser->current_token->line;
    parser->error_column = parser->current_token->column;
}

static flux_token_t* peek(flux_parser_t *parser) {
    if (!parser->peeked_token) {
        parser->peeked_token = flux_lexer_next(parser->lexer);
    }
    return parser->peeked_token;
}

static flux_token_t* advance(flux_parser_t *parser) {
    flux_token_t *prev = parser->current_token;
    if (parser->peeked_token) {
        parser->current_token = parser->peeked_token;
        parser->peeked_token = NULL;
    } else {
        parser->current_token = flux_lexer_next(parser->lexer);
    }
    return prev;
}

static int match(flux_parser_t *parser, flux_token_type_t type) {
    if (parser->current_token->type == type) {
        advance(parser);
        return 1;
    }
    return 0;
}

static crous_value* parse_scalar(flux_parser_t *parser, crous_err_t *err) {
    crous_value *v = NULL;
    flux_token_t *token = parser->current_token;
    
    switch (token->type) {
        case FLUX_TOKEN_NULL:
            v = crous_value_new_null();
            advance(parser);
            break;
            
        case FLUX_TOKEN_BOOL_TRUE:
            v = crous_value_new_bool(1);
            advance(parser);
            break;
            
        case FLUX_TOKEN_BOOL_FALSE:
            v = crous_value_new_bool(0);
            advance(parser);
            break;
            
        case FLUX_TOKEN_INT: {
            int64_t val = strtoll(token->value, NULL, 10);
            v = crous_value_new_int(val);
            advance(parser);
            break;
        }
            
        case FLUX_TOKEN_FLOAT: {
            double val = strtod(token->value, NULL);
            v = crous_value_new_float(val);
            advance(parser);
            break;
        }
            
        case FLUX_TOKEN_STRING: {
            /* String value is already unquoted */
            v = crous_value_new_string(token->value, token->value_len);
            advance(parser);
            break;
        }
            
        case FLUX_TOKEN_KEY: {
            /* Unquoted identifier treated as string */
            v = crous_value_new_string(token->value, token->value_len);
            advance(parser);
            break;
        }
            
        default:
            set_error(parser, "Expected scalar value");
            *err = CROUS_ERR_DECODE;
            return NULL;
    }
    
    if (!v) *err = CROUS_ERR_OOM;
    return v;
}

static crous_value* parse_value(flux_parser_t *parser, crous_err_t *err);

static crous_value* parse_record(flux_parser_t *parser, crous_err_t *err) {
    crous_value *record = crous_value_new_dict(0);
    if (!record) {
        *err = CROUS_ERR_OOM;
        return NULL;
    }
    
    /* Parse key:value pairs */
    while (parser->current_token->type != FLUX_TOKEN_EOF &&
           parser->current_token->type != FLUX_TOKEN_DEDENT) {
        
        if (parser->current_token->type == FLUX_TOKEN_NEWLINE) {
            advance(parser);
            continue;
        }
        
        if (parser->current_token->type != FLUX_TOKEN_KEY) {
            set_error(parser, "Expected key");
            *err = CROUS_ERR_DECODE;
            crous_value_free_tree(record);
            return NULL;
        }
        
        /* Get key */
        char key_buf[256];
        size_t key_len = (parser->current_token->value_len < 255) ? 
                         parser->current_token->value_len : 255;
        memcpy(key_buf, parser->current_token->value, key_len);
        key_buf[key_len] = '\0';
        advance(parser);
        
        /* Expect colon */
        if (!match(parser, FLUX_TOKEN_COLON)) {
            set_error(parser, "Expected ':' after key");
            *err = CROUS_ERR_DECODE;
            crous_value_free_tree(record);
            return NULL;
        }
        
        /* Parse value */
        crous_value *val = parse_value(parser, err);
        if (*err != CROUS_OK) {
            crous_value_free_tree(record);
            return NULL;
        }
        
        /* Add to dict */
        if (crous_value_dict_set_binary(record, key_buf, key_len, val) != CROUS_OK) {
            *err = CROUS_ERR_OOM;
            crous_value_free_tree(record);
            crous_value_free_tree(val);
            return NULL;
        }
        
        /* Consume newline */
        if (parser->current_token->type == FLUX_TOKEN_NEWLINE) {
            advance(parser);
        }
    }
    
    return record;
}

static crous_value* parse_array(flux_parser_t *parser, crous_err_t *err) {
    crous_value *array = crous_value_new_list(0);
    if (!array) {
        *err = CROUS_ERR_OOM;
        return NULL;
    }
    
    /* Skip type hint if present */
    if (match(parser, FLUX_TOKEN_LBRACKET)) {
        /* Type hint like [int], [string], [record], etc. */
        if (parser->current_token->type == FLUX_TOKEN_KEY) {
            advance(parser);
        }
        if (!match(parser, FLUX_TOKEN_RBRACKET)) {
            set_error(parser, "Expected ']'");
            *err = CROUS_ERR_DECODE;
            crous_value_free_tree(array);
            return NULL;
        }
    }
    
    /* Consume newline after type hint */
    if (parser->current_token->type == FLUX_TOKEN_NEWLINE) {
        advance(parser);
    }
    
    /* Expect indent */
    if (parser->current_token->type != FLUX_TOKEN_INDENT) {
        set_error(parser, "Expected indented array elements");
        *err = CROUS_ERR_DECODE;
        crous_value_free_tree(array);
        return NULL;
    }
    advance(parser);
    
    /* Parse array elements */
    while (parser->current_token->type != FLUX_TOKEN_DEDENT &&
           parser->current_token->type != FLUX_TOKEN_EOF) {
        
        if (parser->current_token->type == FLUX_TOKEN_NEWLINE) {
            advance(parser);
            continue;
        }
        
        crous_value *elem = parse_scalar(parser, err);
        if (*err != CROUS_OK) {
            crous_value_free_tree(array);
            return NULL;
        }
        
        if (crous_value_list_append(array, elem) != CROUS_OK) {
            *err = CROUS_ERR_OOM;
            crous_value_free_tree(array);
            crous_value_free_tree(elem);
            return NULL;
        }
        
        /* Consume newline */
        if (parser->current_token->type == FLUX_TOKEN_NEWLINE) {
            advance(parser);
        }
    }
    
    /* Expect dedent */
    if (!match(parser, FLUX_TOKEN_DEDENT)) {
        set_error(parser, "Expected dedent");
        *err = CROUS_ERR_DECODE;
        crous_value_free_tree(array);
        return NULL;
    }
    
    return array;
}

static crous_value* parse_value(flux_parser_t *parser, crous_err_t *err) {
    /* Peek ahead for array detection */
    if (parser->current_token->type == FLUX_TOKEN_LBRACKET) {
        return parse_array(parser, err);
    }
    
    /* Try to parse as scalar */
    return parse_scalar(parser, err);
}

crous_err_t flux_parse(flux_parser_t *parser, crous_value **out_value) {
    crous_err_t err = CROUS_OK;
    crous_value *root = NULL;
    
    if (!parser || !out_value) {
        return CROUS_ERR_INVALID_TYPE;
    }
    
    /* Skip initial newlines */
    while (parser->current_token->type == FLUX_TOKEN_NEWLINE) {
        advance(parser);
    }
    
    /* Parse root structure */
    if (parser->current_token->type == FLUX_TOKEN_EOF) {
        /* Empty document */
        root = crous_value_new_null();
    } else if (parser->current_token->type == FLUX_TOKEN_KEY) {
        /* Root is a record */
        root = parse_record(parser, &err);
    } else {
        /* Root is a single value */
        root = parse_value(parser, &err);
    }
    
    if (err != CROUS_OK || !root) {
        return err;
    }
    
    /* Ensure we're at EOF */
    while (parser->current_token->type == FLUX_TOKEN_NEWLINE) {
        advance(parser);
    }
    
    if (parser->current_token->type != FLUX_TOKEN_EOF) {
        set_error(parser, "Unexpected token after document");
        crous_value_free_tree(root);
        return CROUS_ERR_DECODE;
    }
    
    *out_value = root;
    return CROUS_OK;
}
