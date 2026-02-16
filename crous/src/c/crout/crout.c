/*
 * crout.c — CROUT encoder and decoder
 *
 * CROUT: Compact Readable Object Utility Text
 *
 * Format:
 *   CROUT1\n
 *   [@ <tok>=<key>\n ...]        ← optional token table
 *   <root-value>
 *
 * Value syntax:
 *   { key:val , key:val }        dict
 *   [ val , val ]                list
 *   ( val , val )                tuple
 *   s<len>:<bytes>               string (length-prefixed, binary-safe)
 *   b<len>:<hex>                 bytes  (length-prefixed, hex-encoded)
 *   i<decimal>                   integer
 *   f<decimal>                   float
 *   T                            true
 *   F                            false
 *   N                            null
 *   #<tag>:<value>               tagged value
 */

#include "../include/crous_crout.h"
#include "../include/crous_value.h"
#include "../include/crous_binary.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

/* ============================================================================
   INTERNAL GROWING BUFFER
   ============================================================================ */

typedef struct {
    char  *data;
    size_t len;
    size_t cap;
} cbuf_t;

static cbuf_t cbuf_new(size_t init_cap) {
    cbuf_t b;
    b.cap  = init_cap < 256 ? 256 : init_cap;
    b.data = (char *)malloc(b.cap);
    b.len  = 0;
    return b;
}

static crous_err_t cbuf_grow(cbuf_t *b, size_t need) {
    if (b->len + need <= b->cap) return CROUS_OK;
    size_t newcap = b->cap * 2;
    while (newcap < b->len + need) newcap *= 2;
    char *p = (char *)realloc(b->data, newcap);
    if (!p) return CROUS_ERR_OOM;
    b->data = p;
    b->cap  = newcap;
    return CROUS_OK;
}

static crous_err_t cbuf_append(cbuf_t *b, const char *s, size_t n) {
    crous_err_t e = cbuf_grow(b, n);
    if (e != CROUS_OK) return e;
    memcpy(b->data + b->len, s, n);
    b->len += n;
    return CROUS_OK;
}

static crous_err_t cbuf_appendc(cbuf_t *b, char c) {
    return cbuf_append(b, &c, 1);
}

static crous_err_t cbuf_append_decimal(cbuf_t *b, int64_t v) {
    char tmp[32];
    int n = snprintf(tmp, sizeof(tmp), "%lld", (long long)v);
    return cbuf_append(b, tmp, (size_t)n);
}

static crous_err_t cbuf_append_size(cbuf_t *b, size_t v) {
    char tmp[32];
    int n = snprintf(tmp, sizeof(tmp), "%zu", v);
    return cbuf_append(b, tmp, (size_t)n);
}

static void cbuf_free(cbuf_t *b) {
    free(b->data);
    b->data = NULL;
    b->len = b->cap = 0;
}

/* ============================================================================
   TOKEN TABLE BUILDER (encoder side)
   ============================================================================ */

#define MAX_TOKEN_ENTRIES 55   /* a-z + A-Z + 0-9 minus reserved prefixes */

typedef struct {
    char  *key;          /* owned copy */
    size_t key_len;
    int    count;        /* occurrence count */
    char   tok[4];       /* short token string (1-3 chars) */
    int    tok_len;
} token_entry_t;

typedef struct {
    token_entry_t entries[MAX_TOKEN_ENTRIES];
    int           len;
} token_table_t;

/* Walk the tree and count key occurrences. */
static void count_keys(const crous_value *v, token_table_t *tt, int max_entries) {
    if (!v) return;
    switch (crous_value_get_type(v)) {
        case CROUS_TYPE_DICT: {
            size_t n = crous_value_dict_size(v);
            for (size_t i = 0; i < n; i++) {
                const crous_dict_entry *e = crous_value_dict_get_entry(v, i);
                if (!e) continue;
                /* look up existing */
                int found = 0;
                for (int j = 0; j < tt->len; j++) {
                    if (tt->entries[j].key_len == e->key_len &&
                        memcmp(tt->entries[j].key, e->key, e->key_len) == 0) {
                        tt->entries[j].count++;
                        found = 1;
                        break;
                    }
                }
                if (!found && tt->len < max_entries) {
                    token_entry_t *te = &tt->entries[tt->len];
                    te->key = (char *)malloc(e->key_len + 1);
                    if (te->key) {
                        memcpy(te->key, e->key, e->key_len);
                        te->key[e->key_len] = '\0';
                        te->key_len = e->key_len;
                        te->count   = 1;
                        te->tok_len = 0;
                        tt->len++;
                    }
                }
                /* recurse into value */
                count_keys(e->value, tt, max_entries);
            }
            break;
        }
        case CROUS_TYPE_LIST:
        case CROUS_TYPE_TUPLE: {
            size_t n = crous_value_list_size(v);
            for (size_t i = 0; i < n; i++)
                count_keys(crous_value_list_get(v, i), tt, max_entries);
            break;
        }
        case CROUS_TYPE_TAGGED:
            count_keys((crous_value *)crous_value_get_tagged_inner(v), tt, max_entries);
            break;
        default:
            break;
    }
}

/* Assign single-char tokens: a-z, A-Z, 0-9 */
static void assign_tokens(token_table_t *tt, int threshold) {
    /* Sort by count descending (simple selection sort — at most 62 items) */
    for (int i = 0; i < tt->len - 1; i++) {
        int best = i;
        for (int j = i + 1; j < tt->len; j++) {
            if (tt->entries[j].count > tt->entries[best].count)
                best = j;
        }
        if (best != i) {
            token_entry_t tmp = tt->entries[i];
            tt->entries[i] = tt->entries[best];
            tt->entries[best] = tmp;
        }
    }

    /* Assign letters a-z, A-Z, 0-9, skipping chars that clash with
     * CROUT value prefixes: s(string), b(bytes), i(int), f(float),
     * N(null), T(true), F(false) */
    static const char syms[] = "acdeghj"           /* a-z minus b,f,i,s */
                                "klmnopqruvwxyz"
                                "ABCDEGHI"          /* A-Z minus F,N,T */
                                "JKLMOPQRSUVWXYZ"
                                "0123456789";       /* digits are safe */
    int sym_idx = 0;
    int num_syms = (int)strlen(syms);
    int kept = 0;
    for (int i = 0; i < tt->len && sym_idx < num_syms; i++) {
        if (tt->entries[i].count < threshold) continue;
        tt->entries[kept] = tt->entries[i];
        tt->entries[kept].tok[0] = syms[sym_idx++];
        tt->entries[kept].tok[1] = '\0';
        tt->entries[kept].tok_len = 1;
        kept++;
    }
    /* Free discarded entries */
    for (int i = kept; i < tt->len; i++) {
        free(tt->entries[i].key);
        tt->entries[i].key = NULL;
    }
    tt->len = kept;
}

static void token_table_free(token_table_t *tt) {
    for (int i = 0; i < tt->len; i++) {
        free(tt->entries[i].key);
    }
    tt->len = 0;
}

/* Look up key in token table; return token string or NULL */
static const char* token_lookup(const token_table_t *tt, const char *key, size_t key_len) {
    for (int i = 0; i < tt->len; i++) {
        if (tt->entries[i].key_len == key_len &&
            memcmp(tt->entries[i].key, key, key_len) == 0)
            return tt->entries[i].tok;
    }
    return NULL;
}

/* ============================================================================
   ENCODER
   ============================================================================ */

static crous_err_t encode_value(cbuf_t *b, const crous_value *v,
                                const token_table_t *tt,
                                int pretty, int indent, int depth);

static crous_err_t write_indent(cbuf_t *b, int indent, int depth) {
    int n = indent * depth;
    for (int i = 0; i < n; i++) {
        crous_err_t e = cbuf_appendc(b, ' ');
        if (e != CROUS_OK) return e;
    }
    return CROUS_OK;
}

static crous_err_t encode_string_value(cbuf_t *b, const char *data, size_t len) {
    crous_err_t e;
    e = cbuf_appendc(b, 's');        if (e) return e;
    e = cbuf_append_size(b, len);    if (e) return e;
    e = cbuf_appendc(b, ':');        if (e) return e;
    e = cbuf_append(b, data, len);   if (e) return e;
    return CROUS_OK;
}

static crous_err_t encode_bytes_value(cbuf_t *b, const uint8_t *data, size_t len) {
    static const char hex[] = "0123456789abcdef";
    crous_err_t e;
    e = cbuf_appendc(b, 'b');                  if (e) return e;
    e = cbuf_append_size(b, len);              if (e) return e;
    e = cbuf_appendc(b, ':');                  if (e) return e;
    /* Hex-encode bytes so the output stays valid UTF-8 text */
    e = cbuf_grow(b, len * 2);                 if (e) return e;
    for (size_t i = 0; i < len; i++) {
        e = cbuf_appendc(b, hex[data[i] >> 4]);   if (e) return e;
        e = cbuf_appendc(b, hex[data[i] & 0x0f]); if (e) return e;
    }
    return CROUS_OK;
}

static crous_err_t encode_key(cbuf_t *b, const char *key, size_t key_len,
                              const token_table_t *tt) {
    const char *tok = token_lookup(tt, key, key_len);
    if (tok) {
        return cbuf_append(b, tok, strlen(tok));
    }
    /* literal string key */
    return encode_string_value(b, key, key_len);
}

static crous_err_t encode_value(cbuf_t *b, const crous_value *v,
                                const token_table_t *tt,
                                int pretty, int indent, int depth)
{
    if (!v) return cbuf_appendc(b, 'N');
    crous_err_t e;

    switch (crous_value_get_type(v)) {
    case CROUS_TYPE_NULL:
        return cbuf_appendc(b, 'N');

    case CROUS_TYPE_BOOL:
        return cbuf_appendc(b, crous_value_get_bool(v) ? 'T' : 'F');

    case CROUS_TYPE_INT: {
        e = cbuf_appendc(b, 'i'); if (e) return e;
        return cbuf_append_decimal(b, crous_value_get_int(v));
    }

    case CROUS_TYPE_FLOAT: {
        double d = crous_value_get_float(v);
        char tmp[64];
        int n;
        if (isinf(d) || isnan(d)) {
            if (isnan(d))       n = snprintf(tmp, sizeof(tmp), "fnan");
            else if (d > 0)     n = snprintf(tmp, sizeof(tmp), "finf");
            else                n = snprintf(tmp, sizeof(tmp), "f-inf");
        } else {
            n = snprintf(tmp, sizeof(tmp), "f%.17g", d);
        }
        return cbuf_append(b, tmp, (size_t)n);
    }

    case CROUS_TYPE_STRING: {
        size_t len;
        const char *data = crous_value_get_string(v, &len);
        return encode_string_value(b, data, len);
    }

    case CROUS_TYPE_BYTES: {
        size_t len;
        const uint8_t *data = crous_value_get_bytes(v, &len);
        return encode_bytes_value(b, data, len);
    }

    case CROUS_TYPE_LIST: {
        size_t n = crous_value_list_size(v);
        e = cbuf_appendc(b, '['); if (e) return e;
        for (size_t i = 0; i < n; i++) {
            if (i > 0) {
                e = cbuf_append(b, " , ", 3); if (e) return e;
            }
            if (pretty) {
                e = cbuf_appendc(b, '\n'); if (e) return e;
                e = write_indent(b, indent, depth + 1); if (e) return e;
            }
            e = encode_value(b, crous_value_list_get(v, i), tt, pretty, indent, depth + 1);
            if (e) return e;
        }
        if (pretty && n > 0) {
            e = cbuf_appendc(b, '\n'); if (e) return e;
            e = write_indent(b, indent, depth); if (e) return e;
        }
        return cbuf_appendc(b, ']');
    }

    case CROUS_TYPE_TUPLE: {
        size_t n = crous_value_list_size(v);
        e = cbuf_appendc(b, '('); if (e) return e;
        for (size_t i = 0; i < n; i++) {
            if (i > 0) {
                e = cbuf_append(b, " , ", 3); if (e) return e;
            }
            if (pretty) {
                e = cbuf_appendc(b, '\n'); if (e) return e;
                e = write_indent(b, indent, depth + 1); if (e) return e;
            }
            e = encode_value(b, crous_value_list_get(v, i), tt, pretty, indent, depth + 1);
            if (e) return e;
        }
        if (pretty && n > 0) {
            e = cbuf_appendc(b, '\n'); if (e) return e;
            e = write_indent(b, indent, depth); if (e) return e;
        }
        return cbuf_appendc(b, ')');
    }

    case CROUS_TYPE_DICT: {
        size_t n = crous_value_dict_size(v);
        e = cbuf_appendc(b, '{'); if (e) return e;
        for (size_t i = 0; i < n; i++) {
            const crous_dict_entry *ent = crous_value_dict_get_entry(v, i);
            if (!ent) return CROUS_ERR_INTERNAL;
            if (i > 0) {
                e = cbuf_append(b, " , ", 3); if (e) return e;
            }
            if (pretty) {
                e = cbuf_appendc(b, '\n'); if (e) return e;
                e = write_indent(b, indent, depth + 1); if (e) return e;
            }
            e = encode_key(b, ent->key, ent->key_len, tt); if (e) return e;
            e = cbuf_appendc(b, ':'); if (e) return e;
            e = encode_value(b, ent->value, tt, pretty, indent, depth + 1);
            if (e) return e;
        }
        if (pretty && n > 0) {
            e = cbuf_appendc(b, '\n'); if (e) return e;
            e = write_indent(b, indent, depth); if (e) return e;
        }
        return cbuf_appendc(b, '}');
    }

    case CROUS_TYPE_TAGGED: {
        uint32_t tag = crous_value_get_tag(v);
        const crous_value *inner = crous_value_get_tagged_inner(v);
        char tmp[16];
        int n = snprintf(tmp, sizeof(tmp), "#%u:", (unsigned)tag);
        e = cbuf_append(b, tmp, (size_t)n); if (e) return e;
        return encode_value(b, (crous_value *)inner, tt, pretty, indent, depth);
    }

    default:
        return CROUS_ERR_INVALID_TYPE;
    }
}

crous_err_t crout_encode(
    const crous_value *value,
    const crout_options_t *opts_in,
    char **out_buf,
    size_t *out_size)
{
    if (!value || !out_buf || !out_size) return CROUS_ERR_INVALID_TYPE;

    crout_options_t opts = opts_in ? *opts_in : crout_options_default();

    /* Build token table if requested */
    token_table_t tt;
    memset(&tt, 0, sizeof(tt));
    if (opts.use_tokens) {
        int max_tok = opts.max_tokens;
        if (max_tok > MAX_TOKEN_ENTRIES) max_tok = MAX_TOKEN_ENTRIES;
        count_keys(value, &tt, max_tok);
        assign_tokens(&tt, opts.token_threshold);
    }

    cbuf_t buf = cbuf_new(512);
    if (!buf.data) { token_table_free(&tt); return CROUS_ERR_OOM; }

    crous_err_t e;

    /* Header */
    e = cbuf_append(&buf, CROUT_MAGIC "\n", CROUT_MAGIC_LEN + 1);
    if (e) goto fail;

    /* Token table */
    for (int i = 0; i < tt.len; i++) {
        e = cbuf_append(&buf, "@ ", 2);                           if (e) goto fail;
        e = cbuf_append(&buf, tt.entries[i].tok, (size_t)tt.entries[i].tok_len); if (e) goto fail;
        e = cbuf_appendc(&buf, '=');                              if (e) goto fail;
        e = cbuf_append(&buf, tt.entries[i].key, tt.entries[i].key_len); if (e) goto fail;
        e = cbuf_appendc(&buf, '\n');                             if (e) goto fail;
    }

    /* Root value */
    e = encode_value(&buf, value, &tt, opts.pretty, opts.indent, 0);
    if (e) goto fail;

    /* NUL-terminate for convenience */
    e = cbuf_appendc(&buf, '\0');
    if (e) goto fail;

    *out_buf  = buf.data;
    *out_size = buf.len - 1; /* exclude trailing NUL from reported size */
    token_table_free(&tt);
    return CROUS_OK;

fail:
    cbuf_free(&buf);
    token_table_free(&tt);
    return e;
}

/* ============================================================================
   DECODER
   ============================================================================ */

typedef struct {
    const char *src;
    size_t      len;
    size_t      pos;
    /* Reverse token table: tok -> (key, key_len) */
    struct { char tok[4]; int tok_len; char *key; size_t key_len; } tokens[MAX_TOKEN_ENTRIES];
    int token_count;
} crout_reader_t;

static inline int rd_eof(const crout_reader_t *r) {
    return r->pos >= r->len;
}
static inline char rd_peek(const crout_reader_t *r) {
    return r->pos < r->len ? r->src[r->pos] : '\0';
}
static inline char rd_next(crout_reader_t *r) {
    return r->pos < r->len ? r->src[r->pos++] : '\0';
}
static inline void rd_skip_ws(crout_reader_t *r) {
    while (r->pos < r->len) {
        char c = r->src[r->pos];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            r->pos++;
        else if (c == '/' && r->pos + 1 < r->len && r->src[r->pos + 1] == '/') {
            /* line comment */
            r->pos += 2;
            while (r->pos < r->len && r->src[r->pos] != '\n') r->pos++;
        } else break;
    }
}

/* Parse a decimal size_t from current position.  Stops at non-digit.
 * Returns -1 on overflow or no digits. */
static int64_t rd_parse_size(crout_reader_t *r) {
    if (rd_eof(r) || !isdigit((unsigned char)rd_peek(r))) return -1;
    int64_t val = 0;
    while (!rd_eof(r) && isdigit((unsigned char)rd_peek(r))) {
        int d = rd_next(r) - '0';
        if (val > (int64_t)(CROUS_MAX_STRING_BYTES / 10)) return -1;
        val = val * 10 + d;
    }
    return val;
}

/* Parse a (possibly negative) decimal int64 */
static int rd_parse_int64(crout_reader_t *r, int64_t *out) {
    int neg = 0;
    if (rd_peek(r) == '-') { neg = 1; rd_next(r); }
    if (rd_eof(r) || !isdigit((unsigned char)rd_peek(r))) return 0;
    int64_t val = 0;
    while (!rd_eof(r) && isdigit((unsigned char)rd_peek(r))) {
        val = val * 10 + (rd_next(r) - '0');
    }
    *out = neg ? -val : val;
    return 1;
}

/* Parse header: "CROUT1\n" + optional "@ tok=key\n" lines */
static crous_err_t rd_parse_header(crout_reader_t *r) {
    /* Check magic */
    if (r->len - r->pos < CROUT_MAGIC_LEN + 1) return CROUS_ERR_INVALID_HEADER;
    if (memcmp(r->src + r->pos, CROUT_MAGIC "\n", CROUT_MAGIC_LEN + 1) != 0)
        return CROUS_ERR_INVALID_HEADER;
    r->pos += CROUT_MAGIC_LEN + 1;

    /* Parse token lines */
    while (r->pos + 2 < r->len && r->src[r->pos] == '@' && r->src[r->pos + 1] == ' ') {
        r->pos += 2; /* skip "@ " */

        /* Read token chars until '=' */
        size_t tok_start = r->pos;
        while (r->pos < r->len && r->src[r->pos] != '=' && r->src[r->pos] != '\n')
            r->pos++;
        if (r->pos >= r->len || r->src[r->pos] != '=') return CROUS_ERR_SYNTAX;
        size_t tok_len = r->pos - tok_start;
        if (tok_len == 0 || tok_len > 3) return CROUS_ERR_SYNTAX;
        r->pos++; /* skip '=' */

        /* Read key until newline */
        size_t key_start = r->pos;
        while (r->pos < r->len && r->src[r->pos] != '\n')
            r->pos++;
        size_t key_len = r->pos - key_start;
        if (r->pos < r->len) r->pos++; /* skip '\n' */

        if (r->token_count >= MAX_TOKEN_ENTRIES) return CROUS_ERR_OVERFLOW;
        int idx = r->token_count++;
        memcpy(r->tokens[idx].tok, r->src + tok_start, tok_len);
        r->tokens[idx].tok[tok_len] = '\0';
        r->tokens[idx].tok_len = (int)tok_len;
        r->tokens[idx].key = (char *)malloc(key_len + 1);
        if (!r->tokens[idx].key) return CROUS_ERR_OOM;
        memcpy(r->tokens[idx].key, r->src + key_start, key_len);
        r->tokens[idx].key[key_len] = '\0';
        r->tokens[idx].key_len = key_len;
    }
    return CROUS_OK;
}

static void rd_free_tokens(crout_reader_t *r) {
    for (int i = 0; i < r->token_count; i++) {
        free(r->tokens[i].key);
    }
    r->token_count = 0;
}

/* Resolve a token string to its full key; returns NULL if not found */
static const char* rd_resolve_token(const crout_reader_t *r, const char *tok, size_t tok_len, size_t *out_key_len) {
    for (int i = 0; i < r->token_count; i++) {
        if ((size_t)r->tokens[i].tok_len == tok_len &&
            memcmp(r->tokens[i].tok, tok, tok_len) == 0) {
            *out_key_len = r->tokens[i].key_len;
            return r->tokens[i].key;
        }
    }
    return NULL;
}

/* Forward declaration */
static crous_value* rd_parse_value(crout_reader_t *r, crous_err_t *err, int depth);

/* Parse a key (token identifier or s<len>:... string literal) */
static crous_err_t rd_parse_key(crout_reader_t *r, char **out_key, size_t *out_len) {
    rd_skip_ws(r);
    char c = rd_peek(r);

    if (c == 's' && r->pos + 1 < r->len && isdigit((unsigned char)r->src[r->pos + 1])) {
        /* String literal key: s<len>:<bytes>  (only if 's' followed by digit) */
        rd_next(r); /* consume 's' */
        int64_t slen = rd_parse_size(r);
        if (slen < 0) return CROUS_ERR_SYNTAX;
        if (rd_eof(r) || rd_next(r) != ':') return CROUS_ERR_SYNTAX;
        if ((size_t)(r->len - r->pos) < (size_t)slen) return CROUS_ERR_TRUNCATED;
        char *key = (char *)malloc((size_t)slen + 1);
        if (!key) return CROUS_ERR_OOM;
        memcpy(key, r->src + r->pos, (size_t)slen);
        key[slen] = '\0';
        r->pos += (size_t)slen;
        *out_key = key;
        *out_len = (size_t)slen;
        return CROUS_OK;
    }

    /* Token identifier: [A-Za-z0-9_]{1,3} */
    if (isalnum((unsigned char)c) || c == '_') {
        size_t start = r->pos;
        while (!rd_eof(r) && (isalnum((unsigned char)rd_peek(r)) || rd_peek(r) == '_'))
            rd_next(r);
        size_t tok_len = r->pos - start;

        /* Try to resolve from token table */
        size_t resolved_len;
        const char *resolved = rd_resolve_token(r, r->src + start, tok_len, &resolved_len);
        if (resolved) {
            char *key = (char *)malloc(resolved_len + 1);
            if (!key) return CROUS_ERR_OOM;
            memcpy(key, resolved, resolved_len);
            key[resolved_len] = '\0';
            *out_key = key;
            *out_len = resolved_len;
        } else {
            /* Use the literal token as the key */
            char *key = (char *)malloc(tok_len + 1);
            if (!key) return CROUS_ERR_OOM;
            memcpy(key, r->src + start, tok_len);
            key[tok_len] = '\0';
            *out_key = key;
            *out_len = tok_len;
        }
        return CROUS_OK;
    }

    return CROUS_ERR_SYNTAX;
}

/* Parse dict: { key:value , key:value } */
static crous_value* rd_parse_dict(crout_reader_t *r, crous_err_t *err, int depth) {
    rd_next(r); /* consume '{' */
    crous_value *dict = crous_value_new_dict(4);
    if (!dict) { *err = CROUS_ERR_OOM; return NULL; }

    rd_skip_ws(r);
    if (rd_peek(r) == '}') { rd_next(r); return dict; }

    while (1) {
        rd_skip_ws(r);
        char *key = NULL;
        size_t key_len = 0;
        *err = rd_parse_key(r, &key, &key_len);
        if (*err != CROUS_OK) { crous_value_free_tree(dict); return NULL; }

        rd_skip_ws(r);
        if (rd_eof(r) || rd_next(r) != ':') {
            free(key); crous_value_free_tree(dict);
            *err = CROUS_ERR_SYNTAX; return NULL;
        }

        rd_skip_ws(r);
        crous_value *val = rd_parse_value(r, err, depth + 1);
        if (*err != CROUS_OK) { free(key); crous_value_free_tree(dict); return NULL; }

        *err = crous_value_dict_set_binary(dict, key, key_len, val);
        free(key);
        if (*err != CROUS_OK) { crous_value_free_tree(dict); crous_value_free_tree(val); return NULL; }

        rd_skip_ws(r);
        if (rd_peek(r) == '}') { rd_next(r); break; }
        if (rd_peek(r) == ',') { rd_next(r); continue; }
        crous_value_free_tree(dict);
        *err = CROUS_ERR_SYNTAX;
        return NULL;
    }
    return dict;
}

/* Parse list: [ value , value ] */
static crous_value* rd_parse_list(crout_reader_t *r, crous_err_t *err, int depth) {
    rd_next(r); /* consume '[' */
    crous_value *list = crous_value_new_list(4);
    if (!list) { *err = CROUS_ERR_OOM; return NULL; }

    rd_skip_ws(r);
    if (rd_peek(r) == ']') { rd_next(r); return list; }

    while (1) {
        rd_skip_ws(r);
        crous_value *elem = rd_parse_value(r, err, depth + 1);
        if (*err != CROUS_OK) { crous_value_free_tree(list); return NULL; }

        *err = crous_value_list_append(list, elem);
        if (*err != CROUS_OK) { crous_value_free_tree(list); crous_value_free_tree(elem); return NULL; }

        rd_skip_ws(r);
        if (rd_peek(r) == ']') { rd_next(r); break; }
        if (rd_peek(r) == ',') { rd_next(r); continue; }
        crous_value_free_tree(list);
        *err = CROUS_ERR_SYNTAX;
        return NULL;
    }
    return list;
}

/* Parse tuple: ( value , value ) */
static crous_value* rd_parse_tuple(crout_reader_t *r, crous_err_t *err, int depth) {
    rd_next(r); /* consume '(' */
    crous_value *tup = crous_value_new_tuple(4);
    if (!tup) { *err = CROUS_ERR_OOM; return NULL; }

    rd_skip_ws(r);
    if (rd_peek(r) == ')') { rd_next(r); return tup; }

    while (1) {
        rd_skip_ws(r);
        crous_value *elem = rd_parse_value(r, err, depth + 1);
        if (*err != CROUS_OK) { crous_value_free_tree(tup); return NULL; }

        *err = crous_value_list_append(tup, elem);
        if (*err != CROUS_OK) { crous_value_free_tree(tup); crous_value_free_tree(elem); return NULL; }

        rd_skip_ws(r);
        if (rd_peek(r) == ')') { rd_next(r); break; }
        if (rd_peek(r) == ',') { rd_next(r); continue; }
        crous_value_free_tree(tup);
        *err = CROUS_ERR_SYNTAX;
        return NULL;
    }
    return tup;
}

/* Parse a single value */
static crous_value* rd_parse_value(crout_reader_t *r, crous_err_t *err, int depth) {
    if (depth > CROUS_MAX_DEPTH) {
        *err = CROUS_ERR_DEPTH_EXCEEDED;
        return NULL;
    }

    rd_skip_ws(r);
    if (rd_eof(r)) { *err = CROUS_ERR_TRUNCATED; return NULL; }

    char c = rd_peek(r);

    switch (c) {
    case 'N':
        rd_next(r);
        { crous_value *v = crous_value_new_null();
          if (!v) *err = CROUS_ERR_OOM;
          return v; }

    case 'T':
        rd_next(r);
        { crous_value *v = crous_value_new_bool(1);
          if (!v) *err = CROUS_ERR_OOM;
          return v; }

    case 'F':
        rd_next(r);
        { crous_value *v = crous_value_new_bool(0);
          if (!v) *err = CROUS_ERR_OOM;
          return v; }

    case 'i': {
        rd_next(r); /* consume 'i' */
        int64_t val;
        if (!rd_parse_int64(r, &val)) { *err = CROUS_ERR_SYNTAX; return NULL; }
        crous_value *v = crous_value_new_int(val);
        if (!v) *err = CROUS_ERR_OOM;
        return v;
    }

    case 'f': {
        rd_next(r); /* consume 'f' */
        /* Handle special values: nan, inf, -inf */
        if (r->pos + 3 <= r->len && memcmp(r->src + r->pos, "nan", 3) == 0) {
            r->pos += 3;
            crous_value *v = crous_value_new_float(NAN);
            if (!v) *err = CROUS_ERR_OOM;
            return v;
        }
        if (r->pos + 3 <= r->len && memcmp(r->src + r->pos, "inf", 3) == 0) {
            r->pos += 3;
            crous_value *v = crous_value_new_float(INFINITY);
            if (!v) *err = CROUS_ERR_OOM;
            return v;
        }
        if (r->pos + 4 <= r->len && memcmp(r->src + r->pos, "-inf", 4) == 0) {
            r->pos += 4;
            crous_value *v = crous_value_new_float(-INFINITY);
            if (!v) *err = CROUS_ERR_OOM;
            return v;
        }
        /* General float: consume until delimiter */
        size_t start = r->pos;
        while (!rd_eof(r)) {
            char fc = rd_peek(r);
            if (fc == ' ' || fc == ',' || fc == '}' || fc == ']' ||
                fc == ')' || fc == '\n' || fc == '\r' || fc == '\t')
                break;
            rd_next(r);
        }
        size_t flen = r->pos - start;
        if (flen == 0) { *err = CROUS_ERR_SYNTAX; return NULL; }
        /* Parse the float string */
        char tmp[128];
        if (flen >= sizeof(tmp)) { *err = CROUS_ERR_OVERFLOW; return NULL; }
        memcpy(tmp, r->src + start, flen);
        tmp[flen] = '\0';
        double d = strtod(tmp, NULL);
        crous_value *v = crous_value_new_float(d);
        if (!v) *err = CROUS_ERR_OOM;
        return v;
    }

    case 's': {
        rd_next(r); /* consume 's' */
        int64_t slen = rd_parse_size(r);
        if (slen < 0) { *err = CROUS_ERR_SYNTAX; return NULL; }
        if (rd_eof(r) || rd_next(r) != ':') { *err = CROUS_ERR_SYNTAX; return NULL; }
        if ((int64_t)(r->len - r->pos) < slen) { *err = CROUS_ERR_TRUNCATED; return NULL; }
        crous_value *v = crous_value_new_string(r->src + r->pos, (size_t)slen);
        r->pos += (size_t)slen;
        if (!v) *err = CROUS_ERR_OOM;
        return v;
    }

    case 'b': {
        rd_next(r); /* consume 'b' */
        int64_t blen = rd_parse_size(r);  /* original byte count */
        if (blen < 0) { *err = CROUS_ERR_SYNTAX; return NULL; }
        if (rd_eof(r) || rd_next(r) != ':') { *err = CROUS_ERR_SYNTAX; return NULL; }
        /* Hex-encoded: need 2*blen hex chars in the stream */
        size_t hex_len = (size_t)blen * 2;
        if ((size_t)(r->len - r->pos) < hex_len) { *err = CROUS_ERR_TRUNCATED; return NULL; }
        uint8_t *raw = (uint8_t *)malloc((size_t)blen);
        if (!raw) { *err = CROUS_ERR_OOM; return NULL; }
        for (int64_t i = 0; i < blen; i++) {
            char hi = r->src[r->pos++];
            char lo = r->src[r->pos++];
            int h = (hi >= '0' && hi <= '9') ? hi - '0' :
                    (hi >= 'a' && hi <= 'f') ? hi - 'a' + 10 :
                    (hi >= 'A' && hi <= 'F') ? hi - 'A' + 10 : -1;
            int l = (lo >= '0' && lo <= '9') ? lo - '0' :
                    (lo >= 'a' && lo <= 'f') ? lo - 'a' + 10 :
                    (lo >= 'A' && lo <= 'F') ? lo - 'A' + 10 : -1;
            if (h < 0 || l < 0) { free(raw); *err = CROUS_ERR_SYNTAX; return NULL; }
            raw[i] = (uint8_t)((h << 4) | l);
        }
        crous_value *v = crous_value_new_bytes(raw, (size_t)blen);
        free(raw);
        if (!v) *err = CROUS_ERR_OOM;
        return v;
    }

    case '#': {
        rd_next(r); /* consume '#' */
        int64_t tag;
        if (!rd_parse_int64(r, &tag) || tag < 0) { *err = CROUS_ERR_SYNTAX; return NULL; }
        if (rd_eof(r) || rd_next(r) != ':') { *err = CROUS_ERR_SYNTAX; return NULL; }
        crous_value *inner = rd_parse_value(r, err, depth + 1);
        if (*err != CROUS_OK) return NULL;
        crous_value *v = crous_value_new_tagged((uint32_t)tag, inner);
        if (!v) { crous_value_free_tree(inner); *err = CROUS_ERR_OOM; }
        return v;
    }

    case '{':
        return rd_parse_dict(r, err, depth);

    case '[':
        return rd_parse_list(r, err, depth);

    case '(':
        return rd_parse_tuple(r, err, depth);

    default:
        *err = CROUS_ERR_SYNTAX;
        return NULL;
    }
}

/* ============================================================================
   PUBLIC DECODER ENTRY POINT
   ============================================================================ */

crous_err_t crout_decode(
    const char *buf,
    size_t buf_size,
    crous_value **out_value)
{
    if (!buf || !out_value) return CROUS_ERR_INVALID_TYPE;

    crout_reader_t rd;
    memset(&rd, 0, sizeof(rd));
    rd.src = buf;
    rd.len = buf_size;
    rd.pos = 0;

    crous_err_t err = rd_parse_header(&rd);
    if (err != CROUS_OK) { rd_free_tokens(&rd); return err; }

    err = CROUS_OK;
    crous_value *v = rd_parse_value(&rd, &err, 0);
    rd_free_tokens(&rd);

    if (err != CROUS_OK) {
        if (v) crous_value_free_tree(v);
        return err;
    }
    *out_value = v;
    return CROUS_OK;
}

/* ============================================================================
   CROUT ↔ FLUX CONVERSION HELPERS
   ============================================================================ */

crous_err_t crout_text_to_flux(
    const char *text, size_t text_len,
    uint8_t **out_buf, size_t *out_size)
{
    if (!text || !out_buf || !out_size) return CROUS_ERR_INVALID_TYPE;

    /* Decode CROUT text → crous_value tree */
    crous_value *v = NULL;
    crous_err_t err = crout_decode(text, text_len, &v);
    if (err != CROUS_OK) return err;

    /* Encode crous_value tree → FLUX binary */
    err = crous_encode(v, out_buf, out_size);
    crous_value_free_tree(v);
    return err;
}

crous_err_t crout_flux_to_text(
    const uint8_t *flux, size_t flux_len,
    const crout_options_t *opts,
    char **out_buf, size_t *out_size)
{
    if (!flux || !out_buf || !out_size) return CROUS_ERR_INVALID_TYPE;

    /* Decode FLUX binary → crous_value tree */
    crous_value *v = NULL;
    crous_err_t err = crous_decode(flux, flux_len, &v);
    if (err != CROUS_OK) return err;

    /* Encode crous_value tree → CROUT text */
    err = crout_encode(v, opts, out_buf, out_size);
    crous_value_free_tree(v);
    return err;
}
