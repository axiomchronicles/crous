"""
crous: High-performance binary serialization for Python

This module provides complete Crous serialization with full IDE support.

Version: 2.0.0

Public API:
    Serialization:
        - dumps(obj, *, default=None, encoder=None, allow_custom=True) -> bytes
        - dump(obj, fp, *, default=None) -> None
        - loads(data, *, decoder=None, object_hook=None) -> object
        - load(fp, *, object_hook=None) -> object
    
    Classes:
        - CrousEncoder: Encoder class for custom serialization
        - CrousDecoder: Decoder class for custom deserialization
    
    Custom Serializers:
        - register_serializer(typ, func) -> None
        - unregister_serializer(typ) -> None
        - register_decoder(tag, func) -> None
        - unregister_decoder(tag) -> None
    
    Version Control:
        - version_info() -> VersionInfo
        - check_compatibility(data) -> CompatibilityResult
        - __version__: Version string
        - __version_info__: Full version information tuple

Exceptions:
    - CrousError: Base exception
    - CrousEncodeError: Encoding errors
    - CrousDecodeError: Decoding errors

Supported types:
    Built-in: None, bool, int, float, str, bytes, list, dict, tuple
    Via tagged values: set, frozenset, datetime, date, time, Decimal, UUID

File I/O:
    Both dump() and load() accept:
    - File path (str) - automatically opened and closed
    - File object (with read()/write() methods)

Examples:
    >>> import crous
    >>> 
    >>> # Bytes serialization
    >>> data = {'name': 'Alice', 'age': 30}
    >>> binary = crous.dumps(data)
    >>> crous.loads(binary)
    {'name': 'Alice', 'age': 30}
    >>>
    >>> # File I/O with path
    >>> crous.dump(data, 'output.crous')
    >>> crous.load('output.crous')
    {'name': 'Alice', 'age': 30}
    >>>
    >>> # File I/O with file object
    >>> with open('output.crous', 'wb') as f:
    ...     crous.dump(data, f)
    >>>
    >>> with open('output.crous', 'rb') as f:
    ...     result = crous.load(f)
    >>>
    >>> # Version checking
    >>> crous.version_info()
    VersionInfo(major=2, minor=0, patch=0, ...)
    >>>
    >>> # Compatibility checking
    >>> result = crous.check_compatibility(binary_data)
    >>> if result.is_compatible:
    ...     data = crous.loads(binary_data)
"""

import os
from typing import Any, Union, BinaryIO

# Import from C extension
try:
    from . import crous as _crous_ext
except ImportError:
    # Fallback for development without compiled C extension
    import crous as _crous_ext

# Import version module
from .version import (
    # Version info
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
    deprecated,
    migrate,
)

# Re-export C extension functions directly
dumps = _crous_ext.dumps
loads = _crous_ext.loads
CrousEncoder = _crous_ext.CrousEncoder
CrousDecoder = _crous_ext.CrousDecoder
register_serializer = _crous_ext.register_serializer
unregister_serializer = _crous_ext.unregister_serializer
register_decoder = _crous_ext.register_decoder
unregister_decoder = _crous_ext.unregister_decoder
CrousError = _crous_ext.CrousError
CrousEncodeError = _crous_ext.CrousEncodeError
CrousDecodeError = _crous_ext.CrousDecodeError

__all__ = [
    # Serialization
    "dumps",
    "dump",
    "loads",
    "load",
    "dumps_stream",
    "loads_stream",
    # Classes
    "CrousEncoder",
    "CrousDecoder",
    # Custom serializers
    "register_serializer",
    "unregister_serializer",
    "register_decoder",
    "unregister_decoder",
    # Exceptions
    "CrousError",
    "CrousEncodeError",
    "CrousDecodeError",
    # Version info
    "version_info",
    "check_compatibility",
    "VersionInfo",
    "SemanticVersion",
    "Feature",
    "Compatibility",
    "Header",
    "CompatibilityResult",
]

# Version information
__version__ = VERSION_STRING
__version_info__ = VERSION_TUPLE
__author__ = "Crous Contributors"
__license__ = "MIT"


def version_info() -> VersionInfo:
    """
    Get complete version information.
    
    Returns:
        VersionInfo object with all version details.
        
    Example:
        >>> import crous
        >>> info = crous.version_info()
        >>> print(f"CROUS v{info.string}")
        CROUS v2.0.0
        >>> info.supports_feature(crous.Feature.TAGGED)
        True
    """
    return get_version_info()


def dump(
    obj: Any,
    fp: Union[str, BinaryIO],
    *,
    default=None,
) -> None:
    """
    Serialize obj to a file-like object or file path.
    
    This is a Python wrapper around the C dump() function that provides
    enhanced error handling and convenience features.
    
    Args:
        obj: Python object to serialize.
        fp: Either:
            - A file path (str): Automatically opened/closed
            - A file object: Must have write() method (open in 'wb' mode)
        default: Optional callable for custom types (not yet implemented).
    
    Returns:
        None
    
    Raises:
        CrousEncodeError: If object cannot be serialized.
        IOError: If file operation fails.
        TypeError: If fp is not str or file-like.
    
    Examples:
        >>> import crous
        >>> data = {'key': 'value'}
        >>> 
        >>> # With file path
        >>> crous.dump(data, 'output.crous')
        >>> 
        >>> # With file object
        >>> with open('output.crous', 'wb') as f:
        ...     crous.dump(data, f)
    """
    # Handle string file path
    if isinstance(fp, str):
        try:
            with open(fp, 'wb') as f:
                _crous_ext.dump(obj, f, default=default)
        except IOError as e:
            raise IOError(f"Failed to write to {fp}: {e}") from e
    else:
        # Assume file-like object
        if not hasattr(fp, 'write'):
            raise TypeError(f"fp must be str or have write() method, got {type(fp)}")
        _crous_ext.dump(obj, fp, default=default)


def load(
    fp: Union[str, BinaryIO],
    *,
    object_hook=None,
) -> Any:
    """
    Deserialize from a file-like object or file path.
    
    This is a Python wrapper around the C load() function that provides
    enhanced error handling and convenience features.
    
    Args:
        fp: Either:
            - A file path (str): Automatically opened/closed
            - A file object: Must have read() method (open in 'rb' mode)
        object_hook: Optional callable for dict post-processing (not yet implemented).
    
    Returns:
        Deserialized Python object.
    
    Raises:
        CrousDecodeError: If data is malformed or truncated.
        IOError: If file operation fails.
        TypeError: If fp is not str or file-like.
    
    Examples:
        >>> import crous
        >>> 
        >>> # With file path
        >>> obj = crous.load('output.crous')
        >>> 
        >>> # With file object
        >>> with open('output.crous', 'rb') as f:
        ...     obj = crous.load(f)
    """
    # Handle string file path
    if isinstance(fp, str):
        if not os.path.exists(fp):
            raise FileNotFoundError(f"File not found: {fp}")
        
        try:
            with open(fp, 'rb') as f:
                return _crous_ext.load(f, object_hook=object_hook)
        except IOError as e:
            raise IOError(f"Failed to read from {fp}: {e}") from e
    else:
        # Assume file-like object
        if not hasattr(fp, 'read'):
            raise TypeError(f"fp must be str or have read() method, got {type(fp)}")
        return _crous_ext.load(fp, object_hook=object_hook)


def dumps_stream(
    obj: Any,
    fp: BinaryIO,
    *,
    default=None,
) -> None:
    """
    Stream-based serialization (currently identical to dump for file objects).
    
    This function serializes an object to a file-like object with stream semantics.
    It's designed for use with file objects and custom stream implementations.
    
    Args:
        obj: Python object to serialize.
        fp: File-like object with write() method (must be opened in 'wb' mode).
        default: Optional callable for custom types (not yet implemented).
    
    Returns:
        None
    
    Raises:
        CrousEncodeError: If object cannot be serialized.
        IOError: If write fails.
        TypeError: If fp doesn't have write() method.
    
    Examples:
        >>> import crous
        >>> 
        >>> # With file object
        >>> with open('output.crous', 'wb') as f:
        ...     crous.dumps_stream({'key': 'value'}, f)
    """
    # Assume file-like object
    if not hasattr(fp, 'write'):
        raise TypeError(f"fp must have write() method, got {type(fp)}")
    _crous_ext.dumps_stream(obj, fp, default=default)


def loads_stream(
    fp: BinaryIO,
    *,
    object_hook=None,
) -> Any:
    """
    Stream-based deserialization (currently identical to load for file objects).
    
    This function deserializes an object from a file-like object with stream semantics.
    It's designed for use with file objects and custom stream implementations.
    
    Args:
        fp: File-like object with read() method (must be opened in 'rb' mode).
        object_hook: Optional callable for dict post-processing (not yet implemented).
    
    Returns:
        Deserialized Python object.
    
    Raises:
        CrousDecodeError: If data is malformed or truncated.
        IOError: If read fails.
        TypeError: If fp doesn't have read() method.
    
    Examples:
        >>> import crous
        >>> 
        >>> # With file object
        >>> with open('output.crous', 'rb') as f:
        ...     obj = crous.loads_stream(f)
    """
    # Assume file-like object
    if not hasattr(fp, 'read'):
        raise TypeError(f"fp must have read() method, got {type(fp)}")
    return _crous_ext.loads_stream(fp, object_hook=object_hook)


def _ensure_api_compatibility() -> None:
    """
    Validate that all exported functions exist in the C extension.
    Called at import time to ensure API completeness.
    """
    required = [
        "dumps", "loads", "dump", "load", "dumps_stream", "loads_stream",
        "CrousEncoder", "CrousDecoder",
        "register_serializer", "unregister_serializer",
        "register_decoder", "unregister_decoder",
        "CrousError", "CrousEncodeError", "CrousDecodeError",
    ]
    
    for name in required:
        if not hasattr(_crous_ext, name):
            raise ImportError(f"C extension missing required attribute: {name}")


# Validate on import
_ensure_api_compatibility()