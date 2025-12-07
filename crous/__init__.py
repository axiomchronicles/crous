"""
crous: High-performance binary serialization for Python

This module provides complete Crous serialization with full IDE support.

Public API:
    - dumps(obj, *, default=None, encoder=None, allow_custom=True) -> bytes
    - dump(obj, fp, *, default=None) -> None
    - loads(data, *, decoder=None, object_hook=None) -> object
    - load(fp, *, object_hook=None) -> object
    - CrousEncoder: Encoder class
    - CrousDecoder: Decoder class
    - register_serializer(typ, func) -> None
    - unregister_serializer(typ) -> None
    - register_decoder(tag, func) -> None
    - unregister_decoder(tag) -> None

Exceptions:
    - CrousError: Base exception
    - CrousEncodeError: Encoding errors
    - CrousDecodeError: Decoding errors

Supported types:
    None, bool, int, float, str, bytes, list, dict

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
"""

import os
from typing import Any, Union, BinaryIO

# Import from C extension
try:
    from . import crous as _crous_ext
except ImportError:
    # Fallback for development without compiled C extension
    import crous as _crous_ext

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
    "dumps",
    "dump",
    "loads",
    "load",
    "dumps_stream",
    "loads_stream",
    "CrousEncoder",
    "CrousDecoder",
    "register_serializer",
    "unregister_serializer",
    "register_decoder",
    "unregister_decoder",
    "CrousError",
    "CrousEncodeError",
    "CrousDecodeError",
]

__version__ = "2.0.0"
__author__ = "Crous Contributors"
__license__ = "MIT"


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