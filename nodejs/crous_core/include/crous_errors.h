#ifndef CROUS_ERRORS_H
#define CROUS_ERRORS_H

#include "crous_types.h"

/* ============================================================================
   ERROR UTILITIES
   ============================================================================ */

/**
 * Get human-readable error message for error code
 */
const char* crous_err_str(crous_err_t err);

/**
 * Check if error is critical (non-recoverable)
 */
int crous_err_is_critical(crous_err_t err);

#endif /* CROUS_ERRORS_H */
