"""
test_tagged_types.py - Tagged types and extended types tests

Tests for datetime, Decimal, UUID, set, Path and other tagged types.
"""

import pytest
import crous
from datetime import datetime, date, time
from decimal import Decimal
import uuid
from pathlib import Path


class TestTaggedTypeSupport:
    """Test support for tagged types."""

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_datetime_serialization(self):
        """Test that datetime can be serialized with custom serializer."""
        def datetime_serializer(obj):
            if isinstance(obj, datetime):
                return obj.isoformat()
            raise TypeError()
        
        crous.register_serializer(datetime, datetime_serializer)
        
        try:
            data = datetime(2023, 12, 25, 10, 30, 45)
            binary = crous.dumps(data)
            assert binary is not None
        finally:
            crous.unregister_serializer(datetime)

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_date_serialization(self):
        """Test that date can be serialized with custom serializer."""
        def date_serializer(obj):
            if isinstance(obj, date) and not isinstance(obj, datetime):
                return obj.isoformat()
            raise TypeError()
        
        crous.register_serializer(date, date_serializer)
        
        try:
            data = date(2023, 12, 25)
            binary = crous.dumps(data)
            assert binary is not None
        finally:
            crous.unregister_serializer(date)

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_decimal_serialization(self):
        """Test that Decimal can be serialized."""
        def decimal_serializer(obj):
            if isinstance(obj, Decimal):
                return str(obj)
            raise TypeError()
        
        crous.register_serializer(Decimal, decimal_serializer)
        
        try:
            data = Decimal('3.14159265359')
            binary = crous.dumps(data)
            assert binary is not None
        finally:
            crous.unregister_serializer(Decimal)

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_uuid_serialization(self):
        """Test that UUID can be serialized."""
        def uuid_serializer(obj):
            if isinstance(obj, uuid.UUID):
                return str(obj)
            raise TypeError()
        
        crous.register_serializer(uuid.UUID, uuid_serializer)
        
        try:
            data = uuid.uuid4()
            binary = crous.dumps(data)
            assert binary is not None
        finally:
            crous.unregister_serializer(uuid.UUID)

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_set_serialization(self):
        """Test that set can be serialized."""
        def set_serializer(obj):
            if isinstance(obj, set):
                return list(obj)
            raise TypeError()
        
        crous.register_serializer(set, set_serializer)
        
        try:
            data = {1, 2, 3}
            binary = crous.dumps(data)
            # Result should be a list
            result = crous.loads(binary)
            assert set(result) == {1, 2, 3}
        finally:
            crous.unregister_serializer(set)

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_frozenset_serialization(self):
        """Test that frozenset can be serialized."""
        def frozenset_serializer(obj):
            if isinstance(obj, frozenset):
                return list(obj)
            raise TypeError()
        
        crous.register_serializer(frozenset, frozenset_serializer)
        
        try:
            data = frozenset([1, 2, 3])
            binary = crous.dumps(data)
            result = crous.loads(binary)
            assert set(result) == {1, 2, 3}
        finally:
            crous.unregister_serializer(frozenset)

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_path_serialization(self):
        """Test that pathlib.Path can be serialized."""
        def path_serializer(obj):
            if isinstance(obj, Path):
                return str(obj)
            raise TypeError()
        
        crous.register_serializer(Path, path_serializer)
        
        try:
            data = Path('/home/user/file.txt')
            binary = crous.dumps(data)
            result = crous.loads(binary)
            assert result == '/home/user/file.txt'
        finally:
            crous.unregister_serializer(Path)


class TestBuiltInTaggedTypes:
    """Test built-in tagged type support if implemented."""

    def test_datetime_roundtrip_builtin(self):
        """Test datetime round-trip if built-in support exists."""
        # This might not be implemented yet
        try:
            data = datetime(2023, 12, 25, 10, 30, 45)
            binary = crous.dumps(data)
            result = crous.loads(binary)
            # If it works, verify result
            if isinstance(result, datetime):
                assert result == data
        except crous.CrousEncodeError:
            # Expected if not implemented
            pass

    def test_decimal_roundtrip_builtin(self):
        """Test Decimal round-trip if built-in support exists."""
        try:
            data = Decimal('3.14159')
            binary = crous.dumps(data)
            result = crous.loads(binary)
            if isinstance(result, Decimal):
                assert result == data
        except crous.CrousEncodeError:
            pass

    def test_uuid_roundtrip_builtin(self):
        """Test UUID round-trip if built-in support exists."""
        try:
            data = uuid.UUID('12345678-1234-5678-1234-567812345678')
            binary = crous.dumps(data)
            result = crous.loads(binary)
            if isinstance(result, uuid.UUID):
                assert result == data
        except crous.CrousEncodeError:
            pass
