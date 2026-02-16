/**
 * CROUS Version Control System
 * 
 * Production-level versioning with:
 * - Semantic versioning (major.minor.patch)
 * - Wire format versioning (for binary compatibility)
 * - Feature flags for optional capabilities
 * - Backward/forward compatibility checks
 * - Migration support
 * 
 * Version History:
 *   Wire v1: Initial FLUX binary format
 *   Wire v2: Added tagged values, tuples, set/frozenset support
 * 
 * Library Version follows SemVer:
 *   MAJOR: Breaking API changes
 *   MINOR: New features, backward compatible
 *   PATCH: Bug fixes, backward compatible
 */

#ifndef CROUS_VERSION_H
#define CROUS_VERSION_H

#include <stdint.h>
#include <stddef.h>

/* ============================================================================
   LIBRARY VERSION (SemVer)
   ============================================================================ */

#define CROUS_VERSION_MAJOR 2
#define CROUS_VERSION_MINOR 0
#define CROUS_VERSION_PATCH 0

/* Pre-release identifier (empty for release, e.g., "alpha.1", "beta.2", "rc.1") */
#define CROUS_VERSION_PRERELEASE ""

/* Build metadata (e.g., git commit hash) */
#define CROUS_VERSION_BUILD ""

/* String representation: "2.0.0" or "2.0.0-alpha.1+build.123" */
#define CROUS_VERSION_STRING "2.0.0"

/* Numeric version for comparisons: 0x020000 for 2.0.0 */
#define CROUS_VERSION_HEX ((CROUS_VERSION_MAJOR << 16) | \
                           (CROUS_VERSION_MINOR << 8) | \
                           CROUS_VERSION_PATCH)

/* ============================================================================
   WIRE FORMAT VERSION (Binary Protocol)
   ============================================================================ */

/**
 * Wire format versions control binary serialization compatibility.
 * 
 * Rules:
 * - Incrementing wire version means new binary format
 * - Readers should support reading older wire versions
 * - Writers typically write the latest wire version
 * - Wire version is independent of library version
 */

/* Current wire format version */
#define CROUS_WIRE_VERSION_CURRENT 2

/* Minimum wire version this library can read */
#define CROUS_WIRE_VERSION_MIN_READ 1

/* Maximum wire version this library can read */
#define CROUS_WIRE_VERSION_MAX_READ 2

/* Wire version history */
#define CROUS_WIRE_V1 1  /* Initial format: basic types */
#define CROUS_WIRE_V2 2  /* Added: tagged values, tuples, set/frozenset */

/* ============================================================================
   FEATURE FLAGS
   ============================================================================ */

/**
 * Feature flags indicate optional capabilities.
 * Encoded in the binary header for forward compatibility.
 * 
 * Flags are cumulative bitmask values.
 */

typedef enum {
    CROUS_FEATURE_NONE          = 0x0000,
    
    /* Type features */
    CROUS_FEATURE_TAGGED        = 0x0001,  /* Tagged values (@tag: value) */
    CROUS_FEATURE_TUPLE         = 0x0002,  /* Tuple type (ordered, immutable) */
    CROUS_FEATURE_SET           = 0x0004,  /* Set type via tagged list */
    CROUS_FEATURE_FROZENSET     = 0x0008,  /* Frozenset type via tagged list */
    
    /* Encoding features */
    CROUS_FEATURE_COMPRESSION   = 0x0010,  /* LZ4/ZSTD compression */
    CROUS_FEATURE_STREAMING     = 0x0020,  /* Streaming mode */
    CROUS_FEATURE_SCHEMA        = 0x0040,  /* Schema validation */
    CROUS_FEATURE_ENCRYPTION    = 0x0080,  /* Encrypted payload */
    
    /* Extended types */
    CROUS_FEATURE_DATETIME      = 0x0100,  /* datetime/date/time types */
    CROUS_FEATURE_DECIMAL       = 0x0200,  /* Decimal type */
    CROUS_FEATURE_UUID          = 0x0400,  /* UUID type */
    CROUS_FEATURE_PATH          = 0x0800,  /* Path type */
    
    /* Format features */
    CROUS_FEATURE_COMMENTS      = 0x1000,  /* Embedded comments */
    CROUS_FEATURE_METADATA      = 0x2000,  /* Header metadata */
    CROUS_FEATURE_CHECKSUMS     = 0x4000,  /* Integrity checksums */
    
    /* All features for v2 */
    CROUS_FEATURE_V2_ALL = (CROUS_FEATURE_TAGGED | CROUS_FEATURE_TUPLE | 
                            CROUS_FEATURE_SET | CROUS_FEATURE_FROZENSET),
} crous_feature_flags_t;

/* Features supported by this build */
#define CROUS_FEATURES_SUPPORTED (CROUS_FEATURE_V2_ALL | \
                                   CROUS_FEATURE_DATETIME | \
                                   CROUS_FEATURE_DECIMAL | \
                                   CROUS_FEATURE_UUID)

/* ============================================================================
   VERSION INFO STRUCTURE
   ============================================================================ */

/**
 * Version information structure for runtime queries.
 */
typedef struct {
    /* Library version (SemVer) */
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
    const char *prerelease;
    const char *build;
    const char *string;
    uint32_t hex;
    
    /* Wire format */
    uint8_t wire_current;
    uint8_t wire_min_read;
    uint8_t wire_max_read;
    
    /* Features */
    uint32_t features_supported;
    
    /* Build info */
    const char *compiler;
    const char *platform;
    const char *build_date;
    const char *build_time;
} crous_version_info_t;

/* ============================================================================
   COMPATIBILITY CHECKING
   ============================================================================ */

/**
 * Compatibility result codes
 */
typedef enum {
    CROUS_COMPAT_OK = 0,              /* Fully compatible */
    CROUS_COMPAT_WARN_FEATURES = 1,   /* Compatible but missing optional features */
    CROUS_COMPAT_WARN_NEWER = 2,      /* Newer format, may lose data */
    CROUS_COMPAT_ERR_TOO_OLD = -1,    /* Format too old, cannot read */
    CROUS_COMPAT_ERR_TOO_NEW = -2,    /* Format too new, cannot read */
    CROUS_COMPAT_ERR_FEATURES = -3,   /* Required features not supported */
} crous_compat_t;

/**
 * Header structure extracted from binary data
 */
typedef struct {
    uint8_t magic[4];           /* 'FLUX' */
    uint8_t wire_version;       /* Wire format version */
    uint8_t flags;              /* Header flags */
    uint16_t feature_flags;     /* Feature flags (if METADATA flag set) */
    uint32_t reserved;          /* Reserved for future use */
} crous_header_t;

/* ============================================================================
   API FUNCTIONS
   ============================================================================ */

/**
 * Get version information.
 * Returns pointer to static version info structure.
 */
const crous_version_info_t* crous_get_version_info(void);

/**
 * Get version string.
 * Returns "major.minor.patch" or "major.minor.patch-prerelease+build"
 */
const char* crous_get_version_string(void);

/**
 * Get wire format version.
 */
uint8_t crous_get_wire_version(void);

/**
 * Check if a wire version is supported for reading.
 */
int crous_wire_version_readable(uint8_t version);

/**
 * Check compatibility with binary data.
 * 
 * @param data Binary data to check
 * @param size Size of data
 * @param out_header Optional output for parsed header
 * @return Compatibility result code
 */
crous_compat_t crous_check_compatibility(
    const uint8_t *data, 
    size_t size,
    crous_header_t *out_header);

/**
 * Check if specific features are supported.
 */
int crous_features_supported(uint32_t features);

/**
 * Get human-readable compatibility message.
 */
const char* crous_compat_message(crous_compat_t compat);

/**
 * Compare two version strings.
 * Returns: -1 if v1 < v2, 0 if equal, 1 if v1 > v2
 */
int crous_version_compare(const char *v1, const char *v2);

/* ============================================================================
   DEPRECATION WARNINGS
   ============================================================================ */

#ifdef CROUS_ENABLE_DEPRECATION_WARNINGS
    #if defined(__GNUC__) || defined(__clang__)
        #define CROUS_DEPRECATED(msg) __attribute__((deprecated(msg)))
    #elif defined(_MSC_VER)
        #define CROUS_DEPRECATED(msg) __declspec(deprecated(msg))
    #else
        #define CROUS_DEPRECATED(msg)
    #endif
#else
    #define CROUS_DEPRECATED(msg)
#endif

/* ============================================================================
   MIGRATION SUPPORT
   ============================================================================ */

/**
 * Migration callback type for upgrading wire formats.
 * 
 * @param old_version Source wire version
 * @param new_version Target wire version
 * @param data Input data
 * @param size Input size
 * @param out_data Output data (caller must free)
 * @param out_size Output size
 * @return Error code
 */
typedef int (*crous_migration_fn)(
    uint8_t old_version,
    uint8_t new_version,
    const uint8_t *data,
    size_t size,
    uint8_t **out_data,
    size_t *out_size);

/**
 * Register a migration function.
 */
int crous_register_migration(
    uint8_t from_version,
    uint8_t to_version,
    crous_migration_fn fn);

/**
 * Migrate data from one wire version to another.
 */
int crous_migrate(
    const uint8_t *data,
    size_t size,
    uint8_t target_version,
    uint8_t **out_data,
    size_t *out_size);

#endif /* CROUS_VERSION_H */
