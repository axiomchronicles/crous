"""
crous type stubs for Pylance, MyPy, and other LSP engines

This .pyi file provides complete type information for the crous module.
It enables full IDE autocomplete, type checking, and hover help.

Stubs follow PEP 561 conventions.
"""

from typing import Any, Callable, Optional, Dict, List, Union, overload, TypeVar

# Type variables for generic support
_T = TypeVar("_T")
_SupportsWrite = Any  # File-like object with write() method
_SupportsRead = Any   # File-like object with read() method

# Supported serializable types
CrousSerializable = Union[None, bool, int, float, str, bytes, list, dict]

# ============================================================================
# EXCEPTION HIERARCHY
# ============================================================================

class CrousError(Exception):
    """
    Base exception for all Crous errors.
    
    Raised when an error occurs during serialization or deserialization.
    """
    pass

class CrousEncodeError(CrousError):
    """
    Raised when serialization (dumps/dump) fails.
    
    Common causes:
        - Object type is not serializable
        - Circular references
        - Unsupported type in nested structure
    """
    pass

class CrousDecodeError(CrousError):
    """
    Raised when deserialization (loads/load) fails.
    
    Common causes:
        - Invalid magic number
        - Corrupted or truncated data
        - Unsupported version
        - Malformed binary structure
    """
    pass

# ============================================================================
# CORE SERIALIZATION FUNCTIONS
# ============================================================================

@overload
def dumps(
    obj: CrousSerializable,
    *,
    default: None = None,
    encoder: None = None,
    allow_custom: bool = True,
) -> bytes:
    """
    Serialize obj to Crous binary format.
    
    Converts a Python object to compact binary representation.
    
    Supported types:
        - None
        - bool (True, False)
        - int (any size)
        - float (64-bit IEEE 754)
        - str (UTF-8 encoded)
        - bytes
        - list (heterogeneous)
        - dict (string keys required, heterogeneous values)
    
    Args:
        obj: Python object to serialize.
        default: Optional callable for custom types (not yet implemented).
        encoder: Optional encoder instance (not yet implemented).
        allow_custom: Whether to allow custom types (default True).
    
    Returns:
        Binary bytes in Crous format.
    
    Raises:
        CrousEncodeError: If object cannot be serialized.
        TypeError: If arguments are invalid.
    
    Example:
        >>> import crous
        >>> data = {'name': 'Alice', 'age': 30, 'active': True}
        >>> binary = crous.dumps(data)
        >>> type(binary)
        <class 'bytes'>
    """
    ...

@overload
def dumps(
    obj: Any,
    *,
    default: Optional[Callable[[Any], CrousSerializable]] = None,
    encoder: Optional[Any] = None,
    allow_custom: bool = True,
) -> bytes:
    """Overload for custom default handler."""
    ...

@overload
def loads(
    data: Union[bytes, bytearray, memoryview],
    *,
    object_hook: None = None,
    decoder: None = None,
) -> CrousSerializable:
    """
    Deserialize Crous binary data to Python object.
    
    Decodes binary data in Crous format back to Python objects.
    Restores the exact structure that was serialized with dumps().
    
    Args:
        data: Bytes-like object containing Crous-encoded data.
        object_hook: Optional callable for dict post-processing (not yet implemented).
        decoder: Optional decoder instance (not yet implemented).
    
    Returns:
        Deserialized Python object (one of the supported types).
    
    Raises:
        CrousDecodeError: If data is malformed, truncated, or invalid.
        TypeError: If data is not bytes-like.
    
    Example:
        >>> import crous
        >>> binary = b'CR\\x01...'  # Crous-encoded data
        >>> obj = crous.loads(binary)
        >>> obj
        {'name': 'Alice', 'age': 30, 'active': True}
    """
    ...

@overload
def loads(
    data: Union[bytes, bytearray, memoryview],
    *,
    object_hook: Optional[Callable[[Dict[str, Any]], Any]] = None,
    decoder: Optional[Any] = None,
) -> Any:
    """Overload for custom object_hook."""
    ...

def dump(
    obj: CrousSerializable,
    fp: _SupportsWrite,
    *,
    default: Optional[Callable[[Any], CrousSerializable]] = None,
) -> None:
    """
    Serialize obj to a file-like object.
    
    Encodes obj to Crous binary and writes the bytes via fp.write().
    Equivalent to: fp.write(dumps(obj))
    
    Args:
        obj: Python object to serialize.
        fp: File-like object with write(bytes) -> int method.
        default: Optional callable for custom types (not yet implemented).
    
    Returns:
        None
    
    Raises:
        CrousEncodeError: If object cannot be serialized.
        IOError: If write to fp fails.
        AttributeError: If fp has no write() method.
    
    Example:
        >>> import crous
        >>> with open('data.crous', 'wb') as f:
        ...     crous.dump({'key': 'value'}, f)
    """
    ...

def load(
    fp: _SupportsRead,
    *,
    object_hook: Optional[Callable[[Dict[str, Any]], Any]] = None,
) -> CrousSerializable:
    """
    Deserialize from a file-like object.
    
    Reads Crous binary data via fp.read() and decodes to Python object.
    Equivalent to: loads(fp.read())
    
    Args:
        fp: File-like object with read() -> bytes method.
        object_hook: Optional callable for dict post-processing (not yet implemented).
    
    Returns:
        Deserialized Python object.
    
    Raises:
        CrousDecodeError: If data is malformed or truncated.
        IOError: If read from fp fails.
        AttributeError: If fp has no read() method.
    
    Example:
        >>> import crous
        >>> with open('data.crous', 'rb') as f:
        ...     obj = crous.load(f)
        >>> obj
        {'key': 'value'}
    """
    ...

# ============================================================================
# STREAMING / LOWLEVEL FUNCTIONS (PLACEHOLDERS)
# ============================================================================

def dumps_stream(
    obj: CrousSerializable,
    fp: _SupportsWrite,
    *,
    default: Optional[Callable[[Any], CrousSerializable]] = None,
) -> None:
    """
    Stream-based serialization (currently same as dump).
    
    Args:
        obj: Object to serialize.
        fp: Output file-like object.
        default: Optional serializer for custom types.
    
    Raises:
        CrousEncodeError: If serialization fails.
        IOError: If write fails.
    """
    ...

def loads_stream(
    fp: _SupportsRead,
    *,
    object_hook: Optional[Callable[[Dict[str, Any]], Any]] = None,
) -> CrousSerializable:
    """
    Stream-based deserialization (currently same as load).
    
    Args:
        fp: Input file-like object.
        object_hook: Optional hook for dict post-processing.
    
    Returns:
        Deserialized object.
    
    Raises:
        CrousDecodeError: If deserialization fails.
        IOError: If read fails.
    """
    ...

# ============================================================================
# ENCODER / DECODER CLASSES
# ============================================================================

class CrousEncoder:
    """
    Encoder class for Crous serialization.
    
    Currently a stub class for API compatibility with json module.
    In future versions, this may support incremental encoding and
    custom type handlers.
    
    Methods:
        (To be extended in future versions)
    
    Example:
        >>> import crous
        >>> encoder = crous.CrousEncoder()
        >>> # Future: encoder.encode(obj)
    """
    
    def __init__(self) -> None:
        """Initialize a Crous encoder."""
        ...

class CrousDecoder:
    """
    Decoder class for Crous deserialization.
    
    Currently a stub class for API compatibility with json module.
    In future versions, this may support incremental decoding and
    custom type handlers.
    
    Methods:
        (To be extended in future versions)
    
    Example:
        >>> import crous
        >>> decoder = crous.CrousDecoder()
        >>> # Future: decoder.decode(binary_data)
    """
    
    def __init__(self) -> None:
        """Initialize a Crous decoder."""
        ...

# ============================================================================
# CUSTOM TYPE REGISTRATION
# ============================================================================

def register_serializer(
    typ: type,
    func: Callable[[Any], CrousSerializable],
) -> None:
    """
    Register a custom serializer for a Python type.
    
    Args:
        typ: The Python type to register.
        func: Callable(obj: typ) -> CrousSerializable.
    
    Returns:
        None
    
    Raises:
        NotImplementedError: Not yet implemented.
    
    Example (future):
        >>> import crous
        >>> from datetime import datetime
        >>> def serialize_datetime(dt: datetime) -> str:
        ...     return dt.isoformat()
        >>> crous.register_serializer(datetime, serialize_datetime)
    """
    ...

def unregister_serializer(typ: type) -> None:
    """
    Unregister a custom serializer for a type.
    
    Args:
        typ: The type to unregister.
    
    Returns:
        None
    
    Raises:
        NotImplementedError: Not yet implemented.
    """
    ...

def register_decoder(
    tag: int,
    func: Callable[[CrousSerializable], Any],
) -> None:
    """
    Register a custom decoder for a tagged value.
    
    Args:
        tag: Tag identifier (typically 100-199 for user-defined).
        func: Callable(value: CrousSerializable) -> Any.
    
    Returns:
        None
    
    Raises:
        NotImplementedError: Not yet implemented.
    
    Example (future):
        >>> import crous
        >>> from datetime import datetime
        >>> def decode_datetime(value: str) -> datetime:
        ...     return datetime.fromisoformat(value)
        >>> crous.register_decoder(100, decode_datetime)
    """
    ...

def unregister_decoder(tag: int) -> None:
    """
    Unregister a custom decoder for a tag.
    
    Args:
        tag: Tag identifier to unregister.
    
    Returns:
        None
    
    Raises:
        NotImplementedError: Not yet implemented.
    """
    ...

# ============================================================================
# MODULE METADATA
# ============================================================================

__version__: str
__author__: str
__license__: str

__all__: List[str]
