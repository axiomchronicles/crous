#ifndef CROUS_CROUT_H
#define CROUS_CROUT_H

#include "crous_types.h"
#include "crous_value.h"

/**
 * CROUT: Compact Readable Object Utility Text
 *
 * A token-table + netstring-like textual format:
 *   - Human-readable with short tokens and typed prefixes
 *   - Compact via local token table (repeated keys → 1-3 char tokens)
 *   - Binary-safe strings via length-prefix (s<len>:<bytes>)
 *   - Fast single-pass parse with minimal allocations
 *   - Lossless mapping to/from the FLUX binary tree
 *
 * File layout:
 *   CROUT1\n
 *   [@ <tok>=<keyname>\n ...]
 *   <root-value>
 *
 * Value syntax:
 *   object  { key:value , key:value }
 *   array   [ value , value ]
 *   string  s<len>:<raw-bytes>
 *   bytes   b<len>:<raw-bytes>
 *   int     i<decimal>
 *   float   f<decimal>
 *   bool    T / F
 *   null    N
 *   tagged  #<tag>:<value>
 *   tuple   ( value , value )
 */

/* ============================================================================
   CROUT OPTIONS
   ============================================================================ */

typedef struct {
    int use_tokens;    /* 1 = build token table for repeated keys */
    int pretty;        /* 1 = newlines + indentation */
    int indent;        /* spaces per indent level (default 2) */
    int token_threshold; /* minimum occurrences to tokenize a key (default 2) */
    int max_tokens;    /* maximum token table entries (default 55) */
} crout_options_t;

static inline crout_options_t crout_options_default(void) {
    crout_options_t o;
    o.use_tokens = 1;
    o.pretty = 0;
    o.indent = 2;
    o.token_threshold = 2;
    o.max_tokens = 55;
    return o;
}

/* ============================================================================
   CROUT ENCODER
   ============================================================================ */

/**
 * Encode a crous_value tree to CROUT text format (buffer).
 * Caller must free(*out_buf) on success.
 */
crous_err_t crout_encode(
    const crous_value *value,
    const crout_options_t *opts,
    char **out_buf,
    size_t *out_size);

/* ============================================================================
   CROUT DECODER
   ============================================================================ */

/**
 * Decode CROUT text into a crous_value tree.
 * Caller must crous_value_free_tree(*out_value) on success.
 */
crous_err_t crout_decode(
    const char *buf,
    size_t buf_size,
    crous_value **out_value);

/* ============================================================================
   CROUT ↔ FLUX HELPERS
   ============================================================================ */

/**
 * Convert CROUT text to FLUX binary.
 * Caller must free(*out_buf).
 */
crous_err_t crout_text_to_flux(
    const char *text, size_t text_len,
    uint8_t **out_buf, size_t *out_size);

/**
 * Convert FLUX binary to CROUT text.
 * Caller must free(*out_buf).
 */
crous_err_t crout_flux_to_text(
    const uint8_t *flux, size_t flux_len,
    const crout_options_t *opts,
    char **out_buf, size_t *out_size);

/* Magic and version */
#define CROUT_MAGIC   "CROUT1"
#define CROUT_MAGIC_LEN 6

#endif /* CROUS_CROUT_H */
