/**
 * CROUS Version Control Implementation
 */

#include "../include/crous_version.h"
#include "../include/crous_types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

/* ============================================================================
   VERSION INFO SINGLETON
   ============================================================================ */

static const crous_version_info_t VERSION_INFO = {
    /* Library version */
    .major = CROUS_VERSION_MAJOR,
    .minor = CROUS_VERSION_MINOR,
    .patch = CROUS_VERSION_PATCH,
    .prerelease = CROUS_VERSION_PRERELEASE,
    .build = CROUS_VERSION_BUILD,
    .string = CROUS_VERSION_STRING,
    .hex = CROUS_VERSION_HEX,
    
    /* Wire format */
    .wire_current = CROUS_WIRE_VERSION_CURRENT,
    .wire_min_read = CROUS_WIRE_VERSION_MIN_READ,
    .wire_max_read = CROUS_WIRE_VERSION_MAX_READ,
    
    /* Features */
    .features_supported = CROUS_FEATURES_SUPPORTED,
    
    /* Build info */
#if defined(__clang__)
    .compiler = "Clang " __clang_version__,
#elif defined(__GNUC__)
    .compiler = "GCC " __VERSION__,
#elif defined(_MSC_VER)
    .compiler = "MSVC",
#else
    .compiler = "Unknown",
#endif

#if defined(__APPLE__)
    .platform = "macOS",
#elif defined(__linux__)
    .platform = "Linux",
#elif defined(_WIN32)
    .platform = "Windows",
#elif defined(__FreeBSD__)
    .platform = "FreeBSD",
#else
    .platform = "Unknown",
#endif
    
    .build_date = __DATE__,
    .build_time = __TIME__,
};

const crous_version_info_t* crous_get_version_info(void) {
    return &VERSION_INFO;
}

const char* crous_get_version_string(void) {
    return CROUS_VERSION_STRING;
}

uint8_t crous_get_wire_version(void) {
    return CROUS_WIRE_VERSION_CURRENT;
}

/* ============================================================================
   COMPATIBILITY CHECKING
   ============================================================================ */

int crous_wire_version_readable(uint8_t version) {
    return version >= CROUS_WIRE_VERSION_MIN_READ && 
           version <= CROUS_WIRE_VERSION_MAX_READ;
}

int crous_features_supported(uint32_t features) {
    return (features & CROUS_FEATURES_SUPPORTED) == features;
}

crous_compat_t crous_check_compatibility(
    const uint8_t *data, 
    size_t size,
    crous_header_t *out_header) {
    
    if (!data || size < 6) {
        return CROUS_COMPAT_ERR_TOO_OLD;
    }
    
    /* Parse header */
    crous_header_t header;
    memcpy(header.magic, data, 4);
    header.wire_version = data[4];
    header.flags = data[5];
    header.feature_flags = 0;
    header.reserved = 0;
    
    /* Check magic bytes */
    if (header.magic[0] != 'F' || header.magic[1] != 'L' ||
        header.magic[2] != 'U' || header.magic[3] != 'X') {
        return CROUS_COMPAT_ERR_TOO_OLD;
    }
    
    /* Parse extended header if present */
    if ((header.flags & 0x80) && size >= 10) {
        /* Extended header with feature flags */
        header.feature_flags = (data[6] << 8) | data[7];
        header.reserved = (data[8] << 24) | (data[9] << 16);
        if (size >= 12) {
            header.reserved |= (data[10] << 8) | data[11];
        }
    }
    
    /* Output header if requested */
    if (out_header) {
        *out_header = header;
    }
    
    /* Check wire version */
    if (header.wire_version < CROUS_WIRE_VERSION_MIN_READ) {
        return CROUS_COMPAT_ERR_TOO_OLD;
    }
    
    if (header.wire_version > CROUS_WIRE_VERSION_MAX_READ) {
        return CROUS_COMPAT_ERR_TOO_NEW;
    }
    
    /* Check feature flags */
    if (header.feature_flags != 0) {
        uint32_t unsupported = header.feature_flags & ~CROUS_FEATURES_SUPPORTED;
        if (unsupported != 0) {
            /* Check if unsupported features are required */
            uint32_t required_mask = 0x8000;  /* Top bit indicates required */
            if (unsupported & required_mask) {
                return CROUS_COMPAT_ERR_FEATURES;
            }
            return CROUS_COMPAT_WARN_FEATURES;
        }
    }
    
    /* Check if newer than current */
    if (header.wire_version > CROUS_WIRE_VERSION_CURRENT) {
        return CROUS_COMPAT_WARN_NEWER;
    }
    
    return CROUS_COMPAT_OK;
}

const char* crous_compat_message(crous_compat_t compat) {
    switch (compat) {
        case CROUS_COMPAT_OK:
            return "Fully compatible";
        case CROUS_COMPAT_WARN_FEATURES:
            return "Compatible but some optional features not supported";
        case CROUS_COMPAT_WARN_NEWER:
            return "Newer format version, some data may be ignored";
        case CROUS_COMPAT_ERR_TOO_OLD:
            return "Format version too old, cannot read";
        case CROUS_COMPAT_ERR_TOO_NEW:
            return "Format version too new, cannot read";
        case CROUS_COMPAT_ERR_FEATURES:
            return "Required features not supported by this version";
        default:
            return "Unknown compatibility status";
    }
}

/* ============================================================================
   VERSION COMPARISON
   ============================================================================ */

/**
 * Parse a version component (major, minor, or patch).
 * Returns the number and advances the pointer.
 */
static int parse_version_component(const char **str) {
    int value = 0;
    while (**str && isdigit(**str)) {
        value = value * 10 + (**str - '0');
        (*str)++;
    }
    return value;
}

int crous_version_compare(const char *v1, const char *v2) {
    if (!v1 && !v2) return 0;
    if (!v1) return -1;
    if (!v2) return 1;
    
    const char *p1 = v1;
    const char *p2 = v2;
    
    /* Compare major */
    int major1 = parse_version_component(&p1);
    int major2 = parse_version_component(&p2);
    if (major1 != major2) return (major1 < major2) ? -1 : 1;
    
    /* Skip separator */
    if (*p1 == '.') p1++;
    if (*p2 == '.') p2++;
    
    /* Compare minor */
    int minor1 = parse_version_component(&p1);
    int minor2 = parse_version_component(&p2);
    if (minor1 != minor2) return (minor1 < minor2) ? -1 : 1;
    
    /* Skip separator */
    if (*p1 == '.') p1++;
    if (*p2 == '.') p2++;
    
    /* Compare patch */
    int patch1 = parse_version_component(&p1);
    int patch2 = parse_version_component(&p2);
    if (patch1 != patch2) return (patch1 < patch2) ? -1 : 1;
    
    /* Compare prerelease (if any) */
    int has_pre1 = (*p1 == '-');
    int has_pre2 = (*p2 == '-');
    
    /* A version without prerelease is greater than one with */
    if (!has_pre1 && has_pre2) return 1;
    if (has_pre1 && !has_pre2) return -1;
    
    /* Both have prerelease, compare alphabetically */
    if (has_pre1 && has_pre2) {
        return strcmp(p1 + 1, p2 + 1);
    }
    
    return 0;
}

/* ============================================================================
   MIGRATION REGISTRY
   ============================================================================ */

#define MAX_MIGRATIONS 32

typedef struct {
    uint8_t from_version;
    uint8_t to_version;
    crous_migration_fn fn;
} migration_entry_t;

static migration_entry_t migrations[MAX_MIGRATIONS];
static int migration_count = 0;

int crous_register_migration(
    uint8_t from_version,
    uint8_t to_version,
    crous_migration_fn fn) {
    
    if (migration_count >= MAX_MIGRATIONS) {
        return CROUS_ERR_INTERNAL;
    }
    
    if (!fn) {
        return CROUS_ERR_INVALID_TYPE;
    }
    
    migrations[migration_count].from_version = from_version;
    migrations[migration_count].to_version = to_version;
    migrations[migration_count].fn = fn;
    migration_count++;
    
    return CROUS_OK;
}

/**
 * Find migration path from one version to another.
 * Uses simple linear search for direct migrations.
 * For production, could implement Dijkstra for optimal paths.
 */
static crous_migration_fn find_migration(uint8_t from, uint8_t to) {
    for (int i = 0; i < migration_count; i++) {
        if (migrations[i].from_version == from && 
            migrations[i].to_version == to) {
            return migrations[i].fn;
        }
    }
    return NULL;
}

int crous_migrate(
    const uint8_t *data,
    size_t size,
    uint8_t target_version,
    uint8_t **out_data,
    size_t *out_size) {
    
    if (!data || size < 6 || !out_data || !out_size) {
        return CROUS_ERR_INVALID_TYPE;
    }
    
    /* Get current version from header */
    uint8_t current_version = data[4];
    
    /* No migration needed */
    if (current_version == target_version) {
        uint8_t *copy = malloc(size);
        if (!copy) return CROUS_ERR_OOM;
        memcpy(copy, data, size);
        *out_data = copy;
        *out_size = size;
        return CROUS_OK;
    }
    
    /* Find direct migration */
    crous_migration_fn migrate_fn = find_migration(current_version, target_version);
    
    if (migrate_fn) {
        return migrate_fn(current_version, target_version, 
                         data, size, out_data, out_size);
    }
    
    /* Try multi-step migration (incremental versions) */
    if (current_version < target_version) {
        /* Upgrade path */
        uint8_t *current_data = malloc(size);
        if (!current_data) return CROUS_ERR_OOM;
        memcpy(current_data, data, size);
        size_t current_size = size;
        
        for (uint8_t v = current_version; v < target_version; v++) {
            migrate_fn = find_migration(v, v + 1);
            if (!migrate_fn) {
                free(current_data);
                return CROUS_ERR_INTERNAL;  /* No migration path */
            }
            
            uint8_t *new_data = NULL;
            size_t new_size = 0;
            int err = migrate_fn(v, v + 1, current_data, current_size,
                                 &new_data, &new_size);
            free(current_data);
            
            if (err != CROUS_OK) {
                return err;
            }
            
            current_data = new_data;
            current_size = new_size;
        }
        
        *out_data = current_data;
        *out_size = current_size;
        return CROUS_OK;
    }
    
    /* Downgrade path (from target to current, going backwards) */
    /* Note: Downgrades are generally not supported as they may lose data */
    return CROUS_ERR_INTERNAL;
}

/* ============================================================================
   BUILT-IN MIGRATIONS
   ============================================================================ */

/**
 * Migration from wire v1 to v2.
 * v1 and v2 are largely compatible, just update version byte.
 */
static int migrate_v1_to_v2(
    uint8_t old_version,
    uint8_t new_version,
    const uint8_t *data,
    size_t size,
    uint8_t **out_data,
    size_t *out_size) {
    
    (void)old_version;
    (void)new_version;
    
    uint8_t *output = malloc(size);
    if (!output) return CROUS_ERR_OOM;
    
    memcpy(output, data, size);
    output[4] = 2;  /* Update version byte */
    
    *out_data = output;
    *out_size = size;
    return CROUS_OK;
}

/**
 * Initialize built-in migrations.
 * Called automatically during module init.
 */
void crous_init_migrations(void) {
    crous_register_migration(1, 2, migrate_v1_to_v2);
}

/* ============================================================================
   UTILITY FUNCTIONS
   ============================================================================ */

/**
 * Format version string with all components.
 * 
 * @param major Major version
 * @param minor Minor version  
 * @param patch Patch version
 * @param prerelease Prerelease string (or NULL)
 * @param build Build metadata (or NULL)
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Length of formatted string, or -1 on error
 */
int crous_format_version(
    int major, int minor, int patch,
    const char *prerelease,
    const char *build,
    char *buffer,
    size_t buffer_size) {
    
    if (!buffer || buffer_size == 0) return -1;
    
    int len;
    
    if (prerelease && *prerelease && build && *build) {
        len = snprintf(buffer, buffer_size, "%d.%d.%d-%s+%s",
                      major, minor, patch, prerelease, build);
    } else if (prerelease && *prerelease) {
        len = snprintf(buffer, buffer_size, "%d.%d.%d-%s",
                      major, minor, patch, prerelease);
    } else if (build && *build) {
        len = snprintf(buffer, buffer_size, "%d.%d.%d+%s",
                      major, minor, patch, build);
    } else {
        len = snprintf(buffer, buffer_size, "%d.%d.%d",
                      major, minor, patch);
    }
    
    return (len >= 0 && (size_t)len < buffer_size) ? len : -1;
}

/**
 * Parse version string into components.
 * 
 * @param version Version string to parse
 * @param out_major Output major version
 * @param out_minor Output minor version
 * @param out_patch Output patch version
 * @return 0 on success, -1 on error
 */
int crous_parse_version(
    const char *version,
    int *out_major,
    int *out_minor,
    int *out_patch) {
    
    if (!version) return -1;
    
    int major = 0, minor = 0, patch = 0;
    int matched = sscanf(version, "%d.%d.%d", &major, &minor, &patch);
    
    if (matched < 1) return -1;
    
    if (out_major) *out_major = major;
    if (out_minor) *out_minor = minor;
    if (out_patch) *out_patch = patch;
    
    return 0;
}

/**
 * Check if version satisfies a requirement.
 * 
 * Supported operators:
 *   ">=1.0.0" - Greater than or equal
 *   ">1.0.0"  - Greater than
 *   "<=1.0.0" - Less than or equal
 *   "<1.0.0"  - Less than
 *   "==1.0.0" - Exact match
 *   "^1.0.0"  - Compatible with (same major)
 *   "~1.0.0"  - Approximately (same major.minor)
 */
int crous_version_satisfies(const char *version, const char *requirement) {
    if (!version || !requirement) return 0;
    
    const char *req_version = requirement;
    int op = 0;  /* 0=eq, 1=gt, 2=gte, 3=lt, 4=lte, 5=caret, 6=tilde */
    
    /* Parse operator */
    if (requirement[0] == '>') {
        if (requirement[1] == '=') {
            op = 2;
            req_version = requirement + 2;
        } else {
            op = 1;
            req_version = requirement + 1;
        }
    } else if (requirement[0] == '<') {
        if (requirement[1] == '=') {
            op = 4;
            req_version = requirement + 2;
        } else {
            op = 3;
            req_version = requirement + 1;
        }
    } else if (requirement[0] == '=') {
        if (requirement[1] == '=') {
            req_version = requirement + 2;
        } else {
            req_version = requirement + 1;
        }
        op = 0;
    } else if (requirement[0] == '^') {
        op = 5;
        req_version = requirement + 1;
    } else if (requirement[0] == '~') {
        op = 6;
        req_version = requirement + 1;
    }
    
    /* Skip whitespace */
    while (*req_version == ' ') req_version++;
    
    int cmp = crous_version_compare(version, req_version);
    
    switch (op) {
        case 0: return cmp == 0;
        case 1: return cmp > 0;
        case 2: return cmp >= 0;
        case 3: return cmp < 0;
        case 4: return cmp <= 0;
        case 5: {
            /* Caret: same major version */
            int v_major, r_major;
            crous_parse_version(version, &v_major, NULL, NULL);
            crous_parse_version(req_version, &r_major, NULL, NULL);
            return v_major == r_major && cmp >= 0;
        }
        case 6: {
            /* Tilde: same major.minor */
            int v_major, v_minor, r_major, r_minor;
            crous_parse_version(version, &v_major, &v_minor, NULL);
            crous_parse_version(req_version, &r_major, &r_minor, NULL);
            return v_major == r_major && v_minor == r_minor && cmp >= 0;
        }
        default:
            return 0;
    }
}
