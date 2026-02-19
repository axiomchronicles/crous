# Changelog

All notable changes to Crous are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.0] - 2024-12-07

### Added
- **New binary format v2** with optimized encoding
- **Tagged values support** for custom type preservation
- **Streaming serialization** for large datasets
- **Custom serializers and deserializers** via `register_serializer()` and `register_decoder()`
- **Full type preservation** for Python built-in types
- **File I/O support** with `dump()` and `load()` functions
- **Comprehensive error handling** with specific exception types
- **IDE support** with type hints and `.pyi` stubs
- **Cross-platform support** - Linux, macOS, Windows, ARM64
- **Extensive test suite** with 95%+ code coverage
- **Complete documentation** with examples and API reference
- **Performance optimizations** - 2-5x faster than JSON

### Changed
- Major rewrite with C extension for better performance
- Improved binary format for smaller output (30-50% smaller than JSON)
- Updated API to mirror `json` module for familiarity

### Fixed
- Better error messages for debugging
- Improved memory management in C extension
- Platform-specific build issues resolved

### Deprecated
- None

### Removed
- None

### Security
- None

## [1.0.0] - 2023-06-15

### Added
- Initial release
- Basic serialization support for common Python types
- Simple `dumps()` and `loads()` interface
- Pure Python implementation

### Changed
- N/A

### Deprecated
- N/A

### Removed
- N/A

### Fixed
- N/A

### Security
- N/A

---

## Upgrade Guide

### From 1.0.0 to 2.0.0

**Breaking Changes:**
- Binary format is completely different (v1 → v2)
- Old `.crous` files (v1) are incompatible with v2

**Migration Steps:**
1. Update your Python code to v2 (API is backward compatible)
2. Re-serialize any existing data with v2
3. Test thoroughly in development before production deployment

**New Features to Explore:**
- Use `register_serializer()` for custom types
- Try streaming for large files
- Check the documentation for new capabilities

---

## Version Support

| Version | Status | Python | Release Date | End of Life |
|---------|--------|--------|--------------|-------------|
| 2.0.x | Current | 3.6+ | 2024-12-07 | TBD |
| 1.0.x | EOL | 3.6+ | 2023-06-15 | 2024-12-07 |

---

## Planned Releases

### 2.1.0 (Q1 2025)
- [ ] Async I/O support
- [ ] Streaming decoder improvements
- [ ] Performance optimizations for large datasets
- [ ] Additional built-in type support

### 2.2.0 (Q2 2025)
- [ ] Schema validation
- [ ] Compression support
- [ ] Protocol extensions
- [ ] Additional language bindings

### 3.0.0 (2026+)
- [ ] New binary format v3 (with v2 fallback)
- [ ] Breaking API changes (if needed)
- [ ] Major feature additions
- [ ] Python 3.13+ support

---

## Release Notes Archive

### 2.0.0-rc1 (2024-11-30)
- Release candidate for v2.0.0
- Final testing and bug fixes
- Documentation review

### 2.0.0-beta2 (2024-11-15)
- Tagged values implementation
- Streaming support
- Extensive testing

### 2.0.0-beta1 (2024-10-15)
- Initial beta release
- Core serialization working
- Basic documentation

---

## How to Report Issues

Found a bug? Please report it on [GitHub Issues](https://github.com/crous-project/crous/issues).

Include:
- Python version
- Crous version
- Minimal reproduction code
- Expected vs actual behavior
- Error traceback (if applicable)

---

## Deprecation Notices

### None Currently

When features are deprecated, they will be announced here with a timeline for removal.

---

## Security Updates

For security-related updates, please check the [Security Policy](https://github.com/crous-project/crous/security/policy).

---

Last Updated: 2024-12-07
