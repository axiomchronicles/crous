---
sidebar_position: 2
---

# Versioning and Compatibility Policy

## Version Scheme

Crous follows Semantic Versioning 2.0.0:

```
MAJOR.MINOR.PATCH
```

- **MAJOR**: Incompatible changes
- **MINOR**: New features, backward compatible
- **PATCH**: Bug fixes, backward compatible

**Example:**
- `2.0.0`: First major version
- `2.1.0`: Added new features, backward compatible
- `2.1.3`: Bug fixes

## Release Cycle

- **Major releases**: Annually (or as needed for significant changes)
- **Minor releases**: Every 2-3 months (features, improvements)
- **Patch releases**: As needed (bug fixes, security)

## Backward Compatibility

### Guaranteed Stable (Won't Break)

✅ Public API functions:
- `dumps()`, `loads()`, `dump()`, `load()`
- `register_serializer()`, `unregister_serializer()`
- `register_decoder()`, `unregister_decoder()`
- Exception classes: `CrousError`, `CrousEncodeError`, `CrousDecodeError`

✅ Binary format:
- Version 2.0 format is stable
- Can decode future versions (with fallback)

✅ Serialized data:
- Any data serialized with v2.x can be deserialized with v2.y (x ≤ y)
- Forward compatibility: New features don't break old data

### May Change (With Notice)

⚠️ Implementation details:
- Exact error messages may change
- Exception message wording
- Internal module structure

⚠️ Performance characteristics:
- Encoding/decoding speed may improve
- Memory usage may vary

⚠️ Undocumented features:
- Private functions (starting with `_`)
- Internal classes not in `__all__`

### Will Not Break (Platform Support)

✅ Python 3.7-3.11+ supported
- Won't drop Python 3.7 support until 3.8+ is minimum (v3.0)
- Python version support aligned with official EOL dates

✅ Platform support:
- Linux, macOS, Windows remain supported
- Architecture support (x86_64, ARM64) maintained

## Deprecation Policy

When API needs to change:

1. **Announce**: Deprecation warning in release notes
2. **Warn**: Issue `DeprecationWarning` in code (2 minor versions)
3. **Remove**: Remove in next major version

**Example deprecation timeline:**
- v2.0: Feature introduced
- v2.2: Deprecation announced, warnings added
- v3.0: Feature removed

## Security Policy

### Reporting Vulnerabilities

**Do not** open public GitHub issue for security vulnerabilities.

Email: `security@crous.org`

Include:
- Vulnerability description
- Affected versions
- Proof of concept (if safe to share)
- Suggested fix (if available)

### Security Updates

- Security fixes released as patch version (e.g., 2.1.3)
- Security fixes backported to previous minor versions for 6 months
- Security advisories published after fix release

## Version Support Timeline

| Version | Release | Status | Support Until |
|---------|---------|--------|---------------|
| 2.0.x | 2023-Q1 | Maintenance | 2024-Q4 |
| 2.1.x | 2024-Q2 | Active | 2025-Q2 |
| 2.2.x | 2024-Q4 | Current | 2026-Q1 |

## Binary Format Compatibility

### Format Version

- **Current**: v2.0
- **Magic**: `CROU` (0x43 0x52 0x4F 0x55)
- **Version byte**: 0x02

### Compatibility Rules

- v2.0 format stable and won't change during v2.x
- v3.0 may introduce new format with fallback support
- Readers should implement fallback for unknown versions

## Checking Version

```python
import crous

# Version string
print(crous.__version__)  # e.g., '2.1.0'

# Version tuple (if available)
print(crous.version_info)  # e.g., (2, 1, 0)
```

## Migration Guides

### v2.0 → v2.1

No breaking changes. New features are opt-in:

```python
# Old way (still works)
binary = crous.dumps(data)

# New way (if introduced)
# (example only)
```

### v2.x → v3.0 (Planned)

Planned changes for major release:

- Possible format improvements
- Possible API refinements
- Deprecation warnings in v2.x leading up

Details TBD until v3.0 development begins.

## Python Version Support

| Python | Support | Notes |
|--------|---------|-------|
| 3.7 | ✅ Supported | EOL 2023-06-27 |
| 3.8 | ✅ Supported | EOL 2024-10-07 |
| 3.9 | ✅ Supported | EOL 2025-10-05 |
| 3.10 | ✅ Supported | EOL 2026-10-04 |
| 3.11 | ✅ Supported | EOL 2027-10-24 |
| 3.12 | ✅ Supported | EOL 2028-10-02 |
| 3.13+ | ✅ Planned | Future versions |

## Platform Support

| Platform | Supported | Status |
|----------|-----------|--------|
| Linux x86_64 | ✅ | Primary |
| macOS x86_64 | ✅ | Primary |
| macOS ARM64 | ✅ | Primary (M1/M2) |
| Windows x86_64 | ✅ | Primary |
| ARM64 (Linux) | ⚠️ | Experimental |

## Getting Help

- **Version-specific issues**: Include version in bug reports
- **Compatibility questions**: Ask in Discussions
- **Deprecation warnings**: Check [CHANGELOG](https://github.com/axiomchronicles/crous/blob/main/CHANGELOG.md)

## Changelog

See [CHANGELOG.md](https://github.com/axiomchronicles/crous/blob/main/CHANGELOG.md) for detailed version history.
