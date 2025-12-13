#include "../include/crous_errors.h"

const char* crous_err_str(crous_err_t err) {
    switch (err) {
        case CROUS_OK: return "OK";
        case CROUS_ERR_INVALID_TYPE: return "Invalid type";
        case CROUS_ERR_DECODE: return "Decode error";
        case CROUS_ERR_ENCODE: return "Encode error";
        case CROUS_ERR_OOM: return "Out of memory";
        case CROUS_ERR_OVERFLOW: return "Overflow";
        case CROUS_ERR_INTERNAL: return "Internal error";
        case CROUS_ERR_STREAM: return "Stream error";
        case CROUS_ERR_TAG_UNKNOWN: return "Unknown tag";
        case CROUS_ERR_TRUNCATED: return "Truncated input";
        case CROUS_ERR_INVALID_HEADER: return "Invalid header";
        case CROUS_ERR_SYNTAX: return "Syntax error";
        case CROUS_ERR_DEPTH_EXCEEDED: return "Depth exceeded";
        default: return "Unknown error";
    }
}

int crous_err_is_critical(crous_err_t err) {
    switch (err) {
        case CROUS_ERR_OOM:
        case CROUS_ERR_OVERFLOW:
        case CROUS_ERR_INTERNAL:
            return 1;
        default:
            return 0;
    }
}
