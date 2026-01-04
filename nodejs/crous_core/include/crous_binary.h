#ifndef CROUS_BINARY_H
#define CROUS_BINARY_H

#include "crous_types.h"

/* ============================================================================
   BINARY ENCODING AND DECODING
   ============================================================================ */

/**
 * Encode value to binary format with header
 */
crous_err_t crous_encode_stream(
    const crous_value *value,
    crous_output_stream *out);

/**
 * Decode value from binary stream
 */
crous_err_t crous_decode_stream(
    crous_input_stream *in,
    crous_value **out_value);

/**
 * Encode value without header (just the value bytes)
 */
crous_err_t crous_encode_value_to_stream(
    const crous_value *value,
    crous_output_stream *out);

/**
 * Decode value without header
 */
crous_err_t crous_decode_value_from_stream(
    crous_input_stream *in,
    crous_value **out_value);

/**
 * Convenience: encode to buffer
 */
crous_err_t crous_encode(
    const crous_value *value,
    uint8_t **out_buf,
    size_t *out_size);

/**
 * Convenience: decode from buffer
 */
crous_err_t crous_decode(
    const uint8_t *buf,
    size_t buf_size,
    crous_value **out_value);

/**
 * Convenience: encode to file
 */
crous_err_t crous_encode_file(
    const crous_value *value,
    const char *path);

/**
 * Convenience: decode from file
 */
crous_err_t crous_decode_file(
    const char *path,
    crous_value **out_value);

#endif /* CROUS_BINARY_H */
