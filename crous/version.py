"""
CROUS Version Control Module

Production-level versioning with:
- Semantic versioning (major.minor.patch)
- Wire format versioning (for binary compatibility)
- Feature flags for optional capabilities
- Backward/forward compatibility checks
- Deprecation warnings
- Migration support

Usage:
    >>> import crous
    >>> from crous.version import VersionInfo, check_compatibility
    >>> 
    >>> # Get version info
    >>> crous.version_info()
    VersionInfo(major=2, minor=0, patch=0, ...)
    >>>
    >>> # Check if data is compatible
    >>> compat = check_compatibility(binary_data)
    >>> if compat.is_compatible:
    ...     data = crous.loads(binary_data)
"""

from __future__ import annotations

import re
import warnings
from dataclasses import dataclass, field
from enum import IntEnum, IntFlag, auto
from functools import total_ordering
from typing import (
    TYPE_CHECKING,
    Any,
    Callable,
    Dict,
    List,
    NamedTuple,
    Optional,
    Tuple,
    Union,
)

if TYPE_CHECKING:
    from typing import TypeAlias
    VersionTuple: TypeAlias = Tuple[int, int, int]


# ============================================================================
# VERSION CONSTANTS
# ============================================================================

# Library version (SemVer)
VERSION_MAJOR = 2
VERSION_MINOR = 0
VERSION_PATCH = 0
VERSION_PRERELEASE = ""  # e.g., "alpha.1", "beta.2", "rc.1"
VERSION_BUILD = ""  # e.g., git commit hash

# Computed version values
VERSION_TUPLE = (VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)
VERSION_HEX = (VERSION_MAJOR << 16) | (VERSION_MINOR << 8) | VERSION_PATCH

# Version string
if VERSION_PRERELEASE and VERSION_BUILD:
    VERSION_STRING = f"{VERSION_MAJOR}.{VERSION_MINOR}.{VERSION_PATCH}-{VERSION_PRERELEASE}+{VERSION_BUILD}"
elif VERSION_PRERELEASE:
    VERSION_STRING = f"{VERSION_MAJOR}.{VERSION_MINOR}.{VERSION_PATCH}-{VERSION_PRERELEASE}"
elif VERSION_BUILD:
    VERSION_STRING = f"{VERSION_MAJOR}.{VERSION_MINOR}.{VERSION_PATCH}+{VERSION_BUILD}"
else:
    VERSION_STRING = f"{VERSION_MAJOR}.{VERSION_MINOR}.{VERSION_PATCH}"

# Wire format versions
WIRE_VERSION_CURRENT = 2
WIRE_VERSION_MIN_READ = 1
WIRE_VERSION_MAX_READ = 2

# Wire version history
WIRE_V1 = 1  # Initial format: basic types
WIRE_V2 = 2  # Added: tagged values, tuples, set/frozenset


# ============================================================================
# FEATURE FLAGS
# ============================================================================

class Feature(IntFlag):
    """Feature flags for optional capabilities."""
    
    NONE = 0x0000
    
    # Type features
    TAGGED = 0x0001        # Tagged values (@tag: value)
    TUPLE = 0x0002         # Tuple type (ordered, immutable)
    SET = 0x0004           # Set type via tagged list
    FROZENSET = 0x0008     # Frozenset type via tagged list
    
    # Encoding features
    COMPRESSION = 0x0010   # LZ4/ZSTD compression
    STREAMING = 0x0020     # Streaming mode
    SCHEMA = 0x0040        # Schema validation
    ENCRYPTION = 0x0080    # Encrypted payload
    
    # Extended types
    DATETIME = 0x0100      # datetime/date/time types
    DECIMAL = 0x0200       # Decimal type
    UUID = 0x0400          # UUID type
    PATH = 0x0800          # Path type
    
    # Format features
    COMMENTS = 0x1000      # Embedded comments
    METADATA = 0x2000      # Header metadata
    CHECKSUMS = 0x4000     # Integrity checksums

# Features supported by this version
FEATURES_SUPPORTED = (
    Feature.TAGGED | Feature.TUPLE | Feature.SET | Feature.FROZENSET |
    Feature.DATETIME | Feature.DECIMAL | Feature.UUID
)


# ============================================================================
# COMPATIBILITY
# ============================================================================

class Compatibility(IntEnum):
    """Compatibility check result codes."""
    
    OK = 0                  # Fully compatible
    WARN_FEATURES = 1       # Compatible but missing optional features
    WARN_NEWER = 2          # Newer format, may lose data
    ERR_TOO_OLD = -1        # Format too old, cannot read
    ERR_TOO_NEW = -2        # Format too new, cannot read
    ERR_FEATURES = -3       # Required features not supported
    ERR_INVALID = -4        # Invalid data format

    @property
    def is_compatible(self) -> bool:
        """Check if this result indicates compatibility."""
        return self >= 0
    
    @property
    def is_warning(self) -> bool:
        """Check if this result is a warning."""
        return self in (self.WARN_FEATURES, self.WARN_NEWER)
    
    @property
    def is_error(self) -> bool:
        """Check if this result is an error."""
        return self < 0
    
    @property
    def message(self) -> str:
        """Get human-readable message for this compatibility status."""
        messages = {
            self.OK: "Fully compatible",
            self.WARN_FEATURES: "Compatible but some optional features not supported",
            self.WARN_NEWER: "Newer format version, some data may be ignored",
            self.ERR_TOO_OLD: "Format version too old, cannot read",
            self.ERR_TOO_NEW: "Format version too new, cannot read",
            self.ERR_FEATURES: "Required features not supported by this version",
            self.ERR_INVALID: "Invalid data format",
        }
        return messages.get(self, "Unknown compatibility status")


# ============================================================================
# VERSION INFO
# ============================================================================

@dataclass(frozen=True)
class VersionInfo:
    """
    Complete version information.
    
    Attributes:
        major: Major version number (breaking changes)
        minor: Minor version number (new features)
        patch: Patch version number (bug fixes)
        prerelease: Pre-release identifier (e.g., "alpha.1")
        build: Build metadata (e.g., git hash)
        wire_version: Current wire format version
        wire_min_read: Minimum readable wire version
        wire_max_read: Maximum readable wire version
        features: Supported feature flags
    """
    
    major: int
    minor: int
    patch: int
    prerelease: str = ""
    build: str = ""
    wire_version: int = WIRE_VERSION_CURRENT
    wire_min_read: int = WIRE_VERSION_MIN_READ
    wire_max_read: int = WIRE_VERSION_MAX_READ
    features: Feature = FEATURES_SUPPORTED
    
    @property
    def tuple(self) -> Tuple[int, int, int]:
        """Get version as tuple (major, minor, patch)."""
        return (self.major, self.minor, self.patch)
    
    @property
    def hex(self) -> int:
        """Get version as hex value for comparisons."""
        return (self.major << 16) | (self.minor << 8) | self.patch
    
    @property
    def string(self) -> str:
        """Get full version string."""
        base = f"{self.major}.{self.minor}.{self.patch}"
        if self.prerelease and self.build:
            return f"{base}-{self.prerelease}+{self.build}"
        elif self.prerelease:
            return f"{base}-{self.prerelease}"
        elif self.build:
            return f"{base}+{self.build}"
        return base
    
    @property
    def short(self) -> str:
        """Get short version string (major.minor.patch)."""
        return f"{self.major}.{self.minor}.{self.patch}"
    
    def __str__(self) -> str:
        return self.string
    
    def __repr__(self) -> str:
        return f"VersionInfo({self.string!r})"
    
    def can_read_wire(self, wire_version: int) -> bool:
        """Check if this version can read the given wire format."""
        return self.wire_min_read <= wire_version <= self.wire_max_read
    
    def supports_feature(self, feature: Feature) -> bool:
        """Check if a feature is supported."""
        return (self.features & feature) == feature
    
    def supports_features(self, features: Feature) -> bool:
        """Check if all given features are supported."""
        return (self.features & features) == features


# Singleton instance
_version_info: Optional[VersionInfo] = None


def get_version_info() -> VersionInfo:
    """Get version information singleton."""
    global _version_info
    if _version_info is None:
        _version_info = VersionInfo(
            major=VERSION_MAJOR,
            minor=VERSION_MINOR,
            patch=VERSION_PATCH,
            prerelease=VERSION_PRERELEASE,
            build=VERSION_BUILD,
            wire_version=WIRE_VERSION_CURRENT,
            wire_min_read=WIRE_VERSION_MIN_READ,
            wire_max_read=WIRE_VERSION_MAX_READ,
            features=FEATURES_SUPPORTED,
        )
    return _version_info


# ============================================================================
# SEMANTIC VERSION
# ============================================================================

# SemVer regex pattern (https://semver.org/)
SEMVER_PATTERN = re.compile(
    r'^(?P<major>0|[1-9]\d*)'
    r'\.(?P<minor>0|[1-9]\d*)'
    r'\.(?P<patch>0|[1-9]\d*)'
    r'(?:-(?P<prerelease>(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)'
    r'(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?'
    r'(?:\+(?P<build>[0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$'
)


@total_ordering
class SemanticVersion:
    """
    Semantic version with full SemVer 2.0.0 support.
    
    Examples:
        >>> v1 = SemanticVersion.parse("1.0.0")
        >>> v2 = SemanticVersion.parse("2.0.0-alpha.1")
        >>> v1 < v2
        True
        >>> v2.is_prerelease
        True
    """
    
    __slots__ = ('major', 'minor', 'patch', 'prerelease', 'build')
    
    def __init__(
        self,
        major: int,
        minor: int,
        patch: int,
        prerelease: str = "",
        build: str = ""
    ):
        if major < 0 or minor < 0 or patch < 0:
            raise ValueError("Version numbers must be non-negative")
        self.major = major
        self.minor = minor
        self.patch = patch
        self.prerelease = prerelease
        self.build = build
    
    @classmethod
    def parse(cls, version: str) -> 'SemanticVersion':
        """Parse a SemVer string."""
        match = SEMVER_PATTERN.match(version)
        if not match:
            raise ValueError(f"Invalid semantic version: {version}")
        
        return cls(
            major=int(match.group('major')),
            minor=int(match.group('minor')),
            patch=int(match.group('patch')),
            prerelease=match.group('prerelease') or "",
            build=match.group('build') or "",
        )
    
    @classmethod
    def current(cls) -> 'SemanticVersion':
        """Get current library version."""
        return cls(
            VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
            VERSION_PRERELEASE, VERSION_BUILD
        )
    
    @property
    def tuple(self) -> Tuple[int, int, int]:
        """Get version as tuple."""
        return (self.major, self.minor, self.patch)
    
    @property
    def is_prerelease(self) -> bool:
        """Check if this is a pre-release version."""
        return bool(self.prerelease)
    
    @property
    def is_stable(self) -> bool:
        """Check if this is a stable release (major >= 1, no prerelease)."""
        return self.major >= 1 and not self.prerelease
    
    def __str__(self) -> str:
        result = f"{self.major}.{self.minor}.{self.patch}"
        if self.prerelease:
            result += f"-{self.prerelease}"
        if self.build:
            result += f"+{self.build}"
        return result
    
    def __repr__(self) -> str:
        return f"SemanticVersion({str(self)!r})"
    
    def __hash__(self) -> int:
        return hash((self.major, self.minor, self.patch, self.prerelease))
    
    def __eq__(self, other: object) -> bool:
        if isinstance(other, str):
            try:
                other = self.parse(other)
            except ValueError:
                return NotImplemented
        if not isinstance(other, SemanticVersion):
            return NotImplemented
        return self._comparison_key() == other._comparison_key()
    
    def __lt__(self, other: object) -> bool:
        if isinstance(other, str):
            try:
                other = self.parse(other)
            except ValueError:
                return NotImplemented
        if not isinstance(other, SemanticVersion):
            return NotImplemented
        return self._comparison_key() < other._comparison_key()
    
    def _comparison_key(self) -> tuple:
        """Generate comparison key per SemVer spec."""
        # Pre-release versions have lower precedence
        if self.prerelease:
            # Split into identifiers
            pre_parts = []
            for part in self.prerelease.split('.'):
                if part.isdigit():
                    pre_parts.append((0, int(part)))
                else:
                    pre_parts.append((1, part))
            return (self.major, self.minor, self.patch, 0, tuple(pre_parts))
        else:
            # Non-prerelease has higher precedence
            return (self.major, self.minor, self.patch, 1, ())
    
    def bump_major(self) -> 'SemanticVersion':
        """Return new version with bumped major."""
        return SemanticVersion(self.major + 1, 0, 0)
    
    def bump_minor(self) -> 'SemanticVersion':
        """Return new version with bumped minor."""
        return SemanticVersion(self.major, self.minor + 1, 0)
    
    def bump_patch(self) -> 'SemanticVersion':
        """Return new version with bumped patch."""
        return SemanticVersion(self.major, self.minor, self.patch + 1)
    
    def satisfies(self, requirement: str) -> bool:
        """
        Check if this version satisfies a requirement.
        
        Supported operators:
            >=, >, <=, <, ==, !=
            ^  (compatible with - same major)
            ~  (approximately - same major.minor)
        
        Examples:
            >>> v = SemanticVersion.parse("2.1.0")
            >>> v.satisfies(">=2.0.0")
            True
            >>> v.satisfies("^2.0.0")
            True
            >>> v.satisfies("~2.0.0")
            False
        """
        requirement = requirement.strip()
        
        # Parse operator
        if requirement.startswith('>='):
            op, ver = '>=', requirement[2:].strip()
        elif requirement.startswith('>'):
            op, ver = '>', requirement[1:].strip()
        elif requirement.startswith('<='):
            op, ver = '<=', requirement[2:].strip()
        elif requirement.startswith('<'):
            op, ver = '<', requirement[1:].strip()
        elif requirement.startswith('=='):
            op, ver = '==', requirement[2:].strip()
        elif requirement.startswith('!='):
            op, ver = '!=', requirement[2:].strip()
        elif requirement.startswith('^'):
            op, ver = '^', requirement[1:].strip()
        elif requirement.startswith('~'):
            op, ver = '~', requirement[1:].strip()
        else:
            op, ver = '==', requirement
        
        other = self.parse(ver)
        
        if op == '>=':
            return self >= other
        elif op == '>':
            return self > other
        elif op == '<=':
            return self <= other
        elif op == '<':
            return self < other
        elif op == '==':
            return self == other
        elif op == '!=':
            return self != other
        elif op == '^':
            # Same major version and >= requirement
            return self.major == other.major and self >= other
        elif op == '~':
            # Same major.minor and >= requirement
            return (self.major == other.major and 
                    self.minor == other.minor and 
                    self >= other)
        
        return False


# ============================================================================
# HEADER PARSING
# ============================================================================

@dataclass
class Header:
    """Parsed binary header information."""
    
    magic: bytes
    wire_version: int
    flags: int
    features: Feature = Feature.NONE
    reserved: int = 0
    
    @property
    def is_valid(self) -> bool:
        """Check if magic bytes are valid."""
        return self.magic == b'FLUX'
    
    @property
    def has_extended_header(self) -> bool:
        """Check if extended header is present."""
        return bool(self.flags & 0x80)
    
    @classmethod
    def parse(cls, data: bytes) -> 'Header':
        """Parse header from binary data."""
        if len(data) < 6:
            raise ValueError("Data too short for header")
        
        header = cls(
            magic=data[:4],
            wire_version=data[4],
            flags=data[5],
        )
        
        # Parse extended header if present
        if header.has_extended_header and len(data) >= 10:
            header.features = Feature((data[6] << 8) | data[7])
            header.reserved = (data[8] << 24) | (data[9] << 16)
            if len(data) >= 12:
                header.reserved |= (data[10] << 8) | data[11]
        
        return header


@dataclass
class CompatibilityResult:
    """Result of compatibility check."""
    
    status: Compatibility
    header: Optional[Header] = None
    unsupported_features: Feature = Feature.NONE
    
    @property
    def is_compatible(self) -> bool:
        """Check if data is compatible."""
        return self.status.is_compatible
    
    @property
    def is_warning(self) -> bool:
        """Check if there are warnings."""
        return self.status.is_warning
    
    @property
    def is_error(self) -> bool:
        """Check if there's an error."""
        return self.status.is_error
    
    @property
    def message(self) -> str:
        """Get human-readable message."""
        return self.status.message


def check_compatibility(data: bytes) -> CompatibilityResult:
    """
    Check if binary data is compatible with this version.
    
    Args:
        data: Binary CROUS/FLUX data
        
    Returns:
        CompatibilityResult with status and details
        
    Example:
        >>> result = check_compatibility(binary_data)
        >>> if result.is_compatible:
        ...     loaded = crous.loads(binary_data)
        >>> else:
        ...     print(f"Cannot load: {result.message}")
    """
    if not data or len(data) < 6:
        return CompatibilityResult(Compatibility.ERR_INVALID)
    
    try:
        header = Header.parse(data)
    except (ValueError, IndexError):
        return CompatibilityResult(Compatibility.ERR_INVALID)
    
    if not header.is_valid:
        return CompatibilityResult(Compatibility.ERR_INVALID, header)
    
    # Check wire version
    info = get_version_info()
    
    if header.wire_version < info.wire_min_read:
        return CompatibilityResult(Compatibility.ERR_TOO_OLD, header)
    
    if header.wire_version > info.wire_max_read:
        return CompatibilityResult(Compatibility.ERR_TOO_NEW, header)
    
    # Check features
    unsupported = header.features & ~info.features
    if unsupported:
        # Check if unsupported features are required
        required_mask = Feature(0x8000)
        if unsupported & required_mask:
            return CompatibilityResult(
                Compatibility.ERR_FEATURES, header, unsupported
            )
        return CompatibilityResult(
            Compatibility.WARN_FEATURES, header, unsupported
        )
    
    # Check if newer than current
    if header.wire_version > info.wire_version:
        return CompatibilityResult(Compatibility.WARN_NEWER, header)
    
    return CompatibilityResult(Compatibility.OK, header)


# ============================================================================
# DEPRECATION SYSTEM
# ============================================================================

@dataclass
class DeprecationInfo:
    """Information about a deprecated feature."""
    
    name: str
    deprecated_in: str
    removed_in: Optional[str] = None
    replacement: Optional[str] = None
    reason: Optional[str] = None
    
    @property
    def message(self) -> str:
        """Generate deprecation warning message."""
        msg = f"{self.name} is deprecated since version {self.deprecated_in}"
        if self.removed_in:
            msg += f" and will be removed in version {self.removed_in}"
        if self.replacement:
            msg += f". Use {self.replacement} instead"
        if self.reason:
            msg += f". {self.reason}"
        return msg


# Registry of deprecated features
_deprecations: Dict[str, DeprecationInfo] = {}


def register_deprecation(
    name: str,
    deprecated_in: str,
    removed_in: Optional[str] = None,
    replacement: Optional[str] = None,
    reason: Optional[str] = None,
) -> None:
    """Register a deprecated feature."""
    _deprecations[name] = DeprecationInfo(
        name=name,
        deprecated_in=deprecated_in,
        removed_in=removed_in,
        replacement=replacement,
        reason=reason,
    )


def get_deprecation(name: str) -> Optional[DeprecationInfo]:
    """Get deprecation info for a feature."""
    return _deprecations.get(name)


def warn_deprecated(name: str, stacklevel: int = 2) -> None:
    """Issue a deprecation warning if feature is deprecated."""
    info = get_deprecation(name)
    if info:
        current = SemanticVersion.current()
        
        # Check if already removed
        if info.removed_in:
            removed = SemanticVersion.parse(info.removed_in)
            if current >= removed:
                raise NotImplementedError(
                    f"{info.name} was removed in version {info.removed_in}"
                )
        
        warnings.warn(info.message, DeprecationWarning, stacklevel=stacklevel + 1)


def deprecated(
    since: str,
    removed_in: Optional[str] = None,
    replacement: Optional[str] = None,
    reason: Optional[str] = None,
) -> Callable:
    """
    Decorator to mark a function as deprecated.
    
    Example:
        @deprecated("2.0.0", removed_in="3.0.0", replacement="new_func()")
        def old_func():
            pass
    """
    def decorator(func: Callable) -> Callable:
        name = func.__qualname__
        register_deprecation(name, since, removed_in, replacement, reason)
        
        def wrapper(*args, **kwargs):
            warn_deprecated(name, stacklevel=2)
            return func(*args, **kwargs)
        
        wrapper.__name__ = func.__name__
        wrapper.__doc__ = func.__doc__
        wrapper.__wrapped__ = func
        
        return wrapper
    
    return decorator


# ============================================================================
# MIGRATION SUPPORT
# ============================================================================

# Migration function type
MigrationFunc = Callable[[bytes], bytes]


@dataclass
class Migration:
    """Migration definition."""
    
    from_version: int
    to_version: int
    func: MigrationFunc
    description: str = ""


# Registry of migrations
_migrations: List[Migration] = []


def register_migration(
    from_version: int,
    to_version: int,
    func: MigrationFunc,
    description: str = "",
) -> None:
    """Register a migration function."""
    _migrations.append(Migration(from_version, to_version, func, description))


def get_migration_path(from_version: int, to_version: int) -> List[Migration]:
    """Find migration path between versions."""
    if from_version == to_version:
        return []
    
    if from_version > to_version:
        # Downgrade not supported
        return []
    
    # Simple linear path for incrementing versions
    path = []
    current = from_version
    
    while current < to_version:
        # Find migration for current -> current+1
        migration = next(
            (m for m in _migrations 
             if m.from_version == current and m.to_version == current + 1),
            None
        )
        
        if not migration:
            # Try direct migration
            migration = next(
                (m for m in _migrations 
                 if m.from_version == current and m.to_version == to_version),
                None
            )
            if migration:
                path.append(migration)
                break
            return []  # No path found
        
        path.append(migration)
        current += 1
    
    return path


def migrate(
    data: bytes,
    target_version: Optional[int] = None
) -> bytes:
    """
    Migrate binary data to target version.
    
    Args:
        data: Binary CROUS data
        target_version: Target wire version (default: current)
        
    Returns:
        Migrated binary data
        
    Raises:
        ValueError: If migration path not found
    """
    if len(data) < 6:
        raise ValueError("Data too short")
    
    if target_version is None:
        target_version = WIRE_VERSION_CURRENT
    
    current_version = data[4]
    
    if current_version == target_version:
        return data
    
    path = get_migration_path(current_version, target_version)
    if not path:
        raise ValueError(
            f"No migration path from version {current_version} to {target_version}"
        )
    
    result = data
    for migration in path:
        result = migration.func(result)
    
    return result


# Built-in migration: v1 -> v2
def _migrate_v1_to_v2(data: bytes) -> bytes:
    """Migrate from wire v1 to v2."""
    result = bytearray(data)
    result[4] = 2  # Update version byte
    return bytes(result)


# Register built-in migrations
register_migration(1, 2, _migrate_v1_to_v2, "Update version byte")


# ============================================================================
# PUBLIC API
# ============================================================================

__all__ = [
    # Constants
    'VERSION_MAJOR',
    'VERSION_MINOR', 
    'VERSION_PATCH',
    'VERSION_STRING',
    'VERSION_TUPLE',
    'VERSION_HEX',
    'WIRE_VERSION_CURRENT',
    'WIRE_VERSION_MIN_READ',
    'WIRE_VERSION_MAX_READ',
    
    # Classes
    'VersionInfo',
    'SemanticVersion',
    'Feature',
    'Compatibility',
    'Header',
    'CompatibilityResult',
    'DeprecationInfo',
    'Migration',
    
    # Functions
    'get_version_info',
    'check_compatibility',
    'register_deprecation',
    'get_deprecation',
    'warn_deprecated',
    'deprecated',
    'register_migration',
    'get_migration_path',
    'migrate',
]
