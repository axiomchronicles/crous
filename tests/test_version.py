"""
Test suite for CROUS version control system.

Tests:
    - Version info and constants
    - Semantic versioning
    - Compatibility checking
    - Feature flags
    - Deprecation system
    - Migration support
"""

import pytest
import warnings
from typing import Optional

# Import version module
from crous.version import (
    # Constants
    VERSION_MAJOR,
    VERSION_MINOR,
    VERSION_PATCH,
    VERSION_STRING,
    VERSION_TUPLE,
    VERSION_HEX,
    WIRE_VERSION_CURRENT,
    WIRE_VERSION_MIN_READ,
    WIRE_VERSION_MAX_READ,
    # Classes
    VersionInfo,
    SemanticVersion,
    Feature,
    Compatibility,
    Header,
    CompatibilityResult,
    DeprecationInfo,
    # Functions
    get_version_info,
    check_compatibility,
    register_deprecation,
    get_deprecation,
    warn_deprecated,
    deprecated,
    register_migration,
    get_migration_path,
    migrate,
)

import crous


# ============================================================================
# VERSION CONSTANTS TESTS
# ============================================================================

class TestVersionConstants:
    """Test version constant values."""
    
    def test_version_major_positive(self):
        """Version major should be non-negative."""
        assert VERSION_MAJOR >= 0
    
    def test_version_minor_positive(self):
        """Version minor should be non-negative."""
        assert VERSION_MINOR >= 0
    
    def test_version_patch_positive(self):
        """Version patch should be non-negative."""
        assert VERSION_PATCH >= 0
    
    def test_version_tuple_matches_components(self):
        """VERSION_TUPLE should match individual components."""
        assert VERSION_TUPLE == (VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)
    
    def test_version_hex_calculation(self):
        """VERSION_HEX should be correctly calculated."""
        expected = (VERSION_MAJOR << 16) | (VERSION_MINOR << 8) | VERSION_PATCH
        assert VERSION_HEX == expected
    
    def test_version_string_format(self):
        """VERSION_STRING should be properly formatted."""
        assert VERSION_STRING.startswith(f"{VERSION_MAJOR}.{VERSION_MINOR}.{VERSION_PATCH}")
    
    def test_wire_version_range(self):
        """Wire version range should be valid."""
        assert WIRE_VERSION_MIN_READ <= WIRE_VERSION_CURRENT <= WIRE_VERSION_MAX_READ
    
    def test_crous_module_version(self):
        """crous.__version__ should match VERSION_STRING."""
        assert crous.__version__ == VERSION_STRING
    
    def test_crous_module_version_info(self):
        """crous.__version_info__ should match VERSION_TUPLE."""
        assert crous.__version_info__ == VERSION_TUPLE


# ============================================================================
# VERSION INFO TESTS
# ============================================================================

class TestVersionInfo:
    """Test VersionInfo class."""
    
    def test_get_version_info_returns_version_info(self):
        """get_version_info() should return VersionInfo instance."""
        info = get_version_info()
        assert isinstance(info, VersionInfo)
    
    def test_get_version_info_singleton(self):
        """get_version_info() should return same instance."""
        info1 = get_version_info()
        info2 = get_version_info()
        assert info1 is info2
    
    def test_version_info_components(self):
        """VersionInfo should have correct component values."""
        info = get_version_info()
        assert info.major == VERSION_MAJOR
        assert info.minor == VERSION_MINOR
        assert info.patch == VERSION_PATCH
    
    def test_version_info_tuple(self):
        """VersionInfo.tuple should return tuple."""
        info = get_version_info()
        assert info.tuple == VERSION_TUPLE
    
    def test_version_info_hex(self):
        """VersionInfo.hex should return hex value."""
        info = get_version_info()
        assert info.hex == VERSION_HEX
    
    def test_version_info_string(self):
        """VersionInfo.string should return version string."""
        info = get_version_info()
        assert info.string == VERSION_STRING
    
    def test_version_info_str(self):
        """str(VersionInfo) should return version string."""
        info = get_version_info()
        assert str(info) == VERSION_STRING
    
    def test_version_info_repr(self):
        """repr(VersionInfo) should be meaningful."""
        info = get_version_info()
        assert "VersionInfo" in repr(info)
        assert VERSION_STRING in repr(info)
    
    def test_version_info_can_read_wire_current(self):
        """VersionInfo should be able to read current wire version."""
        info = get_version_info()
        assert info.can_read_wire(WIRE_VERSION_CURRENT)
    
    def test_version_info_can_read_wire_min(self):
        """VersionInfo should be able to read minimum wire version."""
        info = get_version_info()
        assert info.can_read_wire(WIRE_VERSION_MIN_READ)
    
    def test_version_info_cannot_read_wire_too_old(self):
        """VersionInfo should not read wire versions below minimum."""
        info = get_version_info()
        assert not info.can_read_wire(0)
    
    def test_version_info_supports_tagged_feature(self):
        """VersionInfo should support tagged feature."""
        info = get_version_info()
        assert info.supports_feature(Feature.TAGGED)
    
    def test_version_info_supports_tuple_feature(self):
        """VersionInfo should support tuple feature."""
        info = get_version_info()
        assert info.supports_feature(Feature.TUPLE)
    
    def test_version_info_frozen(self):
        """VersionInfo should be frozen (immutable)."""
        info = get_version_info()
        with pytest.raises(Exception):  # FrozenInstanceError or similar
            info.major = 99
    
    def test_crous_version_info_function(self):
        """crous.version_info() should return VersionInfo."""
        info = crous.version_info()
        assert isinstance(info, VersionInfo)


# ============================================================================
# SEMANTIC VERSION TESTS
# ============================================================================

class TestSemanticVersion:
    """Test SemanticVersion class."""
    
    def test_parse_simple_version(self):
        """Parse simple version string."""
        v = SemanticVersion.parse("1.2.3")
        assert v.major == 1
        assert v.minor == 2
        assert v.patch == 3
    
    def test_parse_with_prerelease(self):
        """Parse version with prerelease."""
        v = SemanticVersion.parse("1.0.0-alpha.1")
        assert v.major == 1
        assert v.prerelease == "alpha.1"
    
    def test_parse_with_build(self):
        """Parse version with build metadata."""
        v = SemanticVersion.parse("1.0.0+build.123")
        assert v.major == 1
        assert v.build == "build.123"
    
    def test_parse_full_version(self):
        """Parse version with prerelease and build."""
        v = SemanticVersion.parse("1.0.0-beta.2+build.456")
        assert v.major == 1
        assert v.prerelease == "beta.2"
        assert v.build == "build.456"
    
    def test_parse_invalid_version(self):
        """Parsing invalid version should raise ValueError."""
        with pytest.raises(ValueError):
            SemanticVersion.parse("invalid")
    
    def test_parse_invalid_format(self):
        """Parsing wrong format should raise ValueError."""
        with pytest.raises(ValueError):
            SemanticVersion.parse("1.2")  # Missing patch
    
    def test_current_version(self):
        """SemanticVersion.current() should return current version."""
        v = SemanticVersion.current()
        assert v.major == VERSION_MAJOR
        assert v.minor == VERSION_MINOR
        assert v.patch == VERSION_PATCH
    
    def test_version_str(self):
        """str(SemanticVersion) should format correctly."""
        v = SemanticVersion(1, 2, 3)
        assert str(v) == "1.2.3"
    
    def test_version_str_with_prerelease(self):
        """str should include prerelease."""
        v = SemanticVersion(1, 2, 3, "rc.1")
        assert str(v) == "1.2.3-rc.1"
    
    def test_version_repr(self):
        """repr should be meaningful."""
        v = SemanticVersion(1, 2, 3)
        assert "SemanticVersion" in repr(v)
        assert "1.2.3" in repr(v)
    
    def test_version_equality(self):
        """Test version equality."""
        v1 = SemanticVersion(1, 2, 3)
        v2 = SemanticVersion(1, 2, 3)
        assert v1 == v2
    
    def test_version_equality_with_string(self):
        """Test version equality with string."""
        v = SemanticVersion(1, 2, 3)
        assert v == "1.2.3"
    
    def test_version_inequality(self):
        """Test version inequality."""
        v1 = SemanticVersion(1, 2, 3)
        v2 = SemanticVersion(1, 2, 4)
        assert v1 != v2
    
    def test_version_less_than(self):
        """Test version less than."""
        v1 = SemanticVersion(1, 2, 3)
        v2 = SemanticVersion(1, 2, 4)
        assert v1 < v2
    
    def test_version_greater_than(self):
        """Test version greater than."""
        v1 = SemanticVersion(2, 0, 0)
        v2 = SemanticVersion(1, 9, 9)
        assert v1 > v2
    
    def test_prerelease_less_than_release(self):
        """Prerelease should be less than release."""
        alpha = SemanticVersion.parse("1.0.0-alpha")
        release = SemanticVersion.parse("1.0.0")
        assert alpha < release
    
    def test_prerelease_ordering(self):
        """Prerelease versions should order correctly."""
        alpha = SemanticVersion.parse("1.0.0-alpha")
        beta = SemanticVersion.parse("1.0.0-beta")
        assert alpha < beta
    
    def test_numeric_prerelease_ordering(self):
        """Numeric prerelease identifiers should order numerically."""
        v1 = SemanticVersion.parse("1.0.0-alpha.2")
        v2 = SemanticVersion.parse("1.0.0-alpha.10")
        assert v1 < v2
    
    def test_is_prerelease(self):
        """is_prerelease should detect prerelease versions."""
        alpha = SemanticVersion.parse("1.0.0-alpha")
        release = SemanticVersion.parse("1.0.0")
        assert alpha.is_prerelease
        assert not release.is_prerelease
    
    def test_is_stable(self):
        """is_stable should detect stable versions."""
        stable = SemanticVersion.parse("1.0.0")
        alpha = SemanticVersion.parse("1.0.0-alpha")
        zero = SemanticVersion.parse("0.9.0")
        assert stable.is_stable
        assert not alpha.is_stable
        assert not zero.is_stable
    
    def test_bump_major(self):
        """bump_major should increment major and reset others."""
        v = SemanticVersion(1, 2, 3)
        bumped = v.bump_major()
        assert bumped == SemanticVersion(2, 0, 0)
    
    def test_bump_minor(self):
        """bump_minor should increment minor and reset patch."""
        v = SemanticVersion(1, 2, 3)
        bumped = v.bump_minor()
        assert bumped == SemanticVersion(1, 3, 0)
    
    def test_bump_patch(self):
        """bump_patch should increment patch."""
        v = SemanticVersion(1, 2, 3)
        bumped = v.bump_patch()
        assert bumped == SemanticVersion(1, 2, 4)
    
    def test_tuple_property(self):
        """tuple property should return version tuple."""
        v = SemanticVersion(1, 2, 3)
        assert v.tuple == (1, 2, 3)
    
    def test_hash(self):
        """Versions should be hashable."""
        v = SemanticVersion(1, 2, 3)
        assert hash(v) == hash(SemanticVersion(1, 2, 3))
    
    def test_satisfies_exact(self):
        """Test satisfies with exact match."""
        v = SemanticVersion.parse("1.2.3")
        assert v.satisfies("1.2.3")
        assert v.satisfies("==1.2.3")
    
    def test_satisfies_greater_equal(self):
        """Test satisfies with >=."""
        v = SemanticVersion.parse("2.0.0")
        assert v.satisfies(">=1.0.0")
        assert v.satisfies(">=2.0.0")
        assert not v.satisfies(">=3.0.0")
    
    def test_satisfies_less_than(self):
        """Test satisfies with <."""
        v = SemanticVersion.parse("1.0.0")
        assert v.satisfies("<2.0.0")
        assert not v.satisfies("<1.0.0")
    
    def test_satisfies_not_equal(self):
        """Test satisfies with !=."""
        v = SemanticVersion.parse("1.0.0")
        assert v.satisfies("!=2.0.0")
        assert not v.satisfies("!=1.0.0")
    
    def test_satisfies_caret(self):
        """Test satisfies with ^ (caret - same major)."""
        v = SemanticVersion.parse("2.1.0")
        assert v.satisfies("^2.0.0")
        assert not v.satisfies("^1.0.0")
        assert not v.satisfies("^3.0.0")
    
    def test_satisfies_tilde(self):
        """Test satisfies with ~ (tilde - same major.minor)."""
        v = SemanticVersion.parse("2.1.5")
        assert v.satisfies("~2.1.0")
        assert not v.satisfies("~2.0.0")
        assert not v.satisfies("~2.2.0")
    
    def test_negative_version_raises(self):
        """Negative version numbers should raise ValueError."""
        with pytest.raises(ValueError):
            SemanticVersion(-1, 0, 0)


# ============================================================================
# FEATURE FLAGS TESTS
# ============================================================================

class TestFeatureFlags:
    """Test Feature flag operations."""
    
    def test_feature_none(self):
        """Feature.NONE should be zero."""
        assert Feature.NONE == 0
    
    def test_feature_combination(self):
        """Features should combine with |."""
        combined = Feature.TAGGED | Feature.TUPLE
        assert combined & Feature.TAGGED
        assert combined & Feature.TUPLE
        assert not (combined & Feature.COMPRESSION)
    
    def test_feature_has_tagged(self):
        """Should have TAGGED feature."""
        from crous.version import FEATURES_SUPPORTED
        assert FEATURES_SUPPORTED & Feature.TAGGED
    
    def test_feature_has_tuple(self):
        """Should have TUPLE feature."""
        from crous.version import FEATURES_SUPPORTED
        assert FEATURES_SUPPORTED & Feature.TUPLE
    
    def test_feature_has_set(self):
        """Should have SET feature."""
        from crous.version import FEATURES_SUPPORTED
        assert FEATURES_SUPPORTED & Feature.SET
    
    def test_feature_has_frozenset(self):
        """Should have FROZENSET feature."""
        from crous.version import FEATURES_SUPPORTED
        assert FEATURES_SUPPORTED & Feature.FROZENSET


# ============================================================================
# COMPATIBILITY TESTS
# ============================================================================

class TestCompatibility:
    """Test Compatibility enum."""
    
    def test_ok_is_compatible(self):
        """Compatibility.OK should be compatible."""
        assert Compatibility.OK.is_compatible
    
    def test_warn_is_compatible(self):
        """Warning statuses should be compatible."""
        assert Compatibility.WARN_FEATURES.is_compatible
        assert Compatibility.WARN_NEWER.is_compatible
    
    def test_errors_not_compatible(self):
        """Error statuses should not be compatible."""
        assert not Compatibility.ERR_TOO_OLD.is_compatible
        assert not Compatibility.ERR_TOO_NEW.is_compatible
        assert not Compatibility.ERR_FEATURES.is_compatible
        assert not Compatibility.ERR_INVALID.is_compatible
    
    def test_warn_is_warning(self):
        """Warning statuses should be warnings."""
        assert Compatibility.WARN_FEATURES.is_warning
        assert Compatibility.WARN_NEWER.is_warning
    
    def test_error_is_error(self):
        """Error statuses should be errors."""
        assert Compatibility.ERR_TOO_OLD.is_error
        assert Compatibility.ERR_TOO_NEW.is_error
    
    def test_message_not_empty(self):
        """All statuses should have messages."""
        for status in Compatibility:
            assert status.message


# ============================================================================
# HEADER PARSING TESTS
# ============================================================================

class TestHeader:
    """Test Header parsing."""
    
    def test_parse_valid_header(self):
        """Parse valid FLUX header."""
        data = b'FLUX\x02\x00' + b'\x00' * 10
        header = Header.parse(data)
        assert header.magic == b'FLUX'
        assert header.wire_version == 2
        assert header.is_valid
    
    def test_parse_invalid_magic(self):
        """Header with wrong magic is invalid."""
        data = b'XXXX\x02\x00' + b'\x00' * 10
        header = Header.parse(data)
        assert not header.is_valid
    
    def test_parse_too_short(self):
        """Parsing too-short data should raise ValueError."""
        with pytest.raises(ValueError):
            Header.parse(b'FLUX')
    
    def test_has_extended_header(self):
        """Test extended header detection."""
        # Flag 0x80 indicates extended header
        data = b'FLUX\x02\x80' + b'\x00' * 10
        header = Header.parse(data)
        assert header.has_extended_header


# ============================================================================
# COMPATIBILITY CHECKING TESTS
# ============================================================================

class TestCheckCompatibility:
    """Test check_compatibility function."""
    
    def test_empty_data_invalid(self):
        """Empty data should be invalid."""
        result = check_compatibility(b'')
        assert result.status == Compatibility.ERR_INVALID
    
    def test_short_data_invalid(self):
        """Too-short data should be invalid."""
        result = check_compatibility(b'FLU')
        assert result.status == Compatibility.ERR_INVALID
    
    def test_wrong_magic_invalid(self):
        """Wrong magic bytes should be invalid."""
        result = check_compatibility(b'XXXX\x02\x00' + b'\x00' * 10)
        assert result.status == Compatibility.ERR_INVALID
    
    def test_valid_current_version(self):
        """Valid data with current version should be OK."""
        data = b'FLUX' + bytes([WIRE_VERSION_CURRENT, 0]) + b'\x00' * 10
        result = check_compatibility(data)
        assert result.is_compatible
    
    def test_valid_min_version(self):
        """Valid data with minimum version should be OK."""
        data = b'FLUX' + bytes([WIRE_VERSION_MIN_READ, 0]) + b'\x00' * 10
        result = check_compatibility(data)
        assert result.is_compatible
    
    def test_version_too_old(self):
        """Version below minimum should be ERR_TOO_OLD."""
        if WIRE_VERSION_MIN_READ > 0:
            data = b'FLUX' + bytes([WIRE_VERSION_MIN_READ - 1, 0]) + b'\x00' * 10
            result = check_compatibility(data)
            assert result.status == Compatibility.ERR_TOO_OLD
    
    def test_version_too_new(self):
        """Version above maximum should be ERR_TOO_NEW."""
        data = b'FLUX' + bytes([WIRE_VERSION_MAX_READ + 1, 0]) + b'\x00' * 10
        result = check_compatibility(data)
        assert result.status == Compatibility.ERR_TOO_NEW
    
    def test_result_has_header(self):
        """Result should include parsed header."""
        data = b'FLUX' + bytes([WIRE_VERSION_CURRENT, 0]) + b'\x00' * 10
        result = check_compatibility(data)
        assert result.header is not None
        assert result.header.magic == b'FLUX'
    
    def test_crous_check_compatibility(self):
        """crous.check_compatibility should work."""
        data = b'FLUX' + bytes([WIRE_VERSION_CURRENT, 0]) + b'\x00' * 10
        result = crous.check_compatibility(data)
        assert result.is_compatible


# ============================================================================
# DEPRECATION SYSTEM TESTS
# ============================================================================

class TestDeprecationSystem:
    """Test deprecation warning system."""
    
    def test_register_deprecation(self):
        """Can register a deprecation."""
        register_deprecation(
            "test_old_func",
            "1.0.0",
            removed_in="2.0.0",
            replacement="test_new_func()",
        )
        info = get_deprecation("test_old_func")
        assert info is not None
        assert info.name == "test_old_func"
        assert info.deprecated_in == "1.0.0"
    
    def test_deprecation_message(self):
        """Deprecation info should have a message."""
        register_deprecation(
            "test_feature",
            "1.5.0",
            removed_in="3.0.0",
            replacement="new_feature()",
            reason="Old feature is slow",
        )
        info = get_deprecation("test_feature")
        msg = info.message
        assert "test_feature" in msg
        assert "1.5.0" in msg
        assert "3.0.0" in msg
        assert "new_feature()" in msg
    
    def test_deprecated_decorator(self):
        """@deprecated decorator should issue warning."""
        @deprecated("1.0.0", replacement="new_func()")
        def old_func():
            return 42
        
        with warnings.catch_warnings(record=True) as w:
            warnings.simplefilter("always")
            result = old_func()
            
            assert result == 42
            assert len(w) == 1
            assert issubclass(w[0].category, DeprecationWarning)
            assert "deprecated" in str(w[0].message).lower()


# ============================================================================
# MIGRATION TESTS
# ============================================================================

class TestMigration:
    """Test migration system."""
    
    def test_get_migration_path_same_version(self):
        """Migration path for same version is empty."""
        path = get_migration_path(2, 2)
        assert path == []
    
    def test_get_migration_path_downgrade_unsupported(self):
        """Downgrade migration is not supported."""
        path = get_migration_path(2, 1)
        assert path == []
    
    def test_migrate_same_version(self):
        """Migrate to same version returns same data."""
        data = b'FLUX\x02\x00test'
        result = migrate(data, 2)
        assert result == data
    
    def test_migrate_v1_to_v2(self):
        """Migrate from v1 to v2."""
        data = b'FLUX\x01\x00test'
        result = migrate(data, 2)
        assert result[4] == 2  # Version byte updated
    
    def test_migrate_default_target(self):
        """Migrate without target uses current version."""
        data = b'FLUX\x01\x00test'
        result = migrate(data)
        assert result[4] == WIRE_VERSION_CURRENT
    
    def test_migrate_short_data_raises(self):
        """Migrating too-short data should raise ValueError."""
        with pytest.raises(ValueError):
            migrate(b'FLU')
    
    def test_register_custom_migration(self):
        """Can register custom migration."""
        # Register migration 99 -> 100
        def migrate_99_100(data: bytes) -> bytes:
            result = bytearray(data)
            result[4] = 100
            return bytes(result)
        
        register_migration(99, 100, migrate_99_100, "Test migration")
        
        path = get_migration_path(99, 100)
        assert len(path) == 1
        assert path[0].from_version == 99
        assert path[0].to_version == 100


# ============================================================================
# INTEGRATION TESTS
# ============================================================================

class TestVersionIntegration:
    """Integration tests with crous serialization."""
    
    def test_serialized_data_compatible(self):
        """Data serialized by crous should be compatible."""
        data = {'name': 'test', 'value': 42}
        binary = crous.dumps(data)
        
        result = check_compatibility(binary)
        assert result.is_compatible
    
    def test_serialized_has_valid_header(self):
        """Serialized data should have valid FLUX header."""
        data = {'key': 'value'}
        binary = crous.dumps(data)
        
        result = check_compatibility(binary)
        assert result.header is not None
        assert result.header.is_valid
        assert result.header.magic == b'FLUX'
    
    def test_serialized_uses_current_wire_version(self):
        """Serialized data should use a readable wire version."""
        binary = crous.dumps([1, 2, 3])
        
        result = check_compatibility(binary)
        # Wire version should be readable (between min and max)
        assert WIRE_VERSION_MIN_READ <= result.header.wire_version <= WIRE_VERSION_MAX_READ
    
    def test_version_info_matches_features(self):
        """Version info should match available features."""
        info = crous.version_info()
        
        # We support tagged values
        assert info.supports_feature(Feature.TAGGED)
        
        # We support set/frozenset
        assert info.supports_feature(Feature.SET)
        assert info.supports_feature(Feature.FROZENSET)


# ============================================================================
# EDGE CASES
# ============================================================================

class TestEdgeCases:
    """Edge case tests."""
    
    def test_version_with_leading_zeros_in_string_parts(self):
        """Parse version with string identifiers (leading zeros in alphanumeric allowed)."""
        # Per SemVer spec, leading zeros are NOT allowed in numeric identifiers
        # but alphanumeric identifiers like "alpha01" are fine
        v = SemanticVersion.parse("1.0.0-alpha01")
        assert v.prerelease == "alpha01"
    
    def test_very_large_version_numbers(self):
        """Handle very large version numbers."""
        v = SemanticVersion(999, 999, 999)
        assert str(v) == "999.999.999"
    
    def test_empty_prerelease(self):
        """Version with empty prerelease should work."""
        v = SemanticVersion(1, 0, 0, "")
        assert not v.is_prerelease
    
    def test_compatibility_result_properties(self):
        """CompatibilityResult properties should work correctly."""
        result = CompatibilityResult(Compatibility.OK)
        assert result.is_compatible
        assert not result.is_warning
        assert not result.is_error
        assert result.message == Compatibility.OK.message


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
