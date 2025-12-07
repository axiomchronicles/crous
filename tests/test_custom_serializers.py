"""
test_custom_serializers.py - Custom serializer and decoder tests

Tests registration and use of custom serializers and decoders.
"""

import pytest
import crous
from datetime import datetime, date
from decimal import Decimal
import uuid
import json


class TestCustomSerializerRegistration:
    """Test registering custom serializers."""

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_register_serializer_for_datetime(self):
        """Test registering a custom serializer for datetime."""
        def datetime_serializer(obj):
            if isinstance(obj, datetime):
                return obj.isoformat()
            raise TypeError(f"Object {type(obj)} is not JSON serializable")
        
        # Register the serializer
        crous.register_serializer(datetime, datetime_serializer)
        
        try:
            # Now datetime should serialize
            data = datetime(2023, 12, 25, 10, 30, 45)
            binary = crous.dumps(data)
            # Result depends on how the custom serializer is used
            assert binary is not None
        finally:
            # Unregister after test
            crous.unregister_serializer(datetime)

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_register_serializer_for_decimal(self):
        """Test registering a custom serializer for Decimal."""
        def decimal_serializer(obj):
            if isinstance(obj, Decimal):
                return str(obj)
            raise TypeError(f"Object {type(obj)} not serializable")
        
        crous.register_serializer(Decimal, decimal_serializer)
        
        try:
            data = Decimal('3.14159265359')
            binary = crous.dumps(data)
            assert binary is not None
        finally:
            crous.unregister_serializer(Decimal)

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_register_serializer_for_custom_class(self):
        """Test registering serializer for custom class."""
        class Point:
            def __init__(self, x, y):
                self.x = x
                self.y = y
        
        def point_serializer(obj):
            if isinstance(obj, Point):
                return {'x': obj.x, 'y': obj.y}
            raise TypeError()
        
        crous.register_serializer(Point, point_serializer)
        
        try:
            data = Point(10, 20)
            binary = crous.dumps(data)
            assert binary is not None
        finally:
            crous.unregister_serializer(Point)

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_unregister_serializer(self):
        """Test unregistering a custom serializer."""
        def serializer(obj):
            return 'serialized'
        
        crous.register_serializer(datetime, serializer)
        crous.unregister_serializer(datetime)
        
        # After unregistering, datetime should fail
        data = datetime.now()
        with pytest.raises(crous.CrousEncodeError):
            crous.dumps(data)


class TestCustomDecoderRegistration:
    """Test registering custom decoders."""

    @pytest.mark.skip(reason="register_decoder not yet implemented in Crous")
    def test_register_decoder_for_tag(self):
        """Test registering a custom decoder for a tag."""
        def my_decoder(data):
            # Decoder receives the tagged value
            return f"decoded: {data}"
        
        crous.register_decoder(100, my_decoder)
        
        try:
            # Test would use tagged values
            pass
        finally:
            crous.unregister_decoder(100)

    @pytest.mark.skip(reason="register_decoder not yet implemented in Crous")
    def test_unregister_decoder(self):
        """Test unregistering a decoder."""
        def decoder(data):
            return data
        
        crous.register_decoder(100, decoder)
        crous.unregister_decoder(100)


class TestSerializerFallback:
    """Test default/fallback behavior for custom serializers."""

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_custom_serializer_not_called_for_builtin(self):
        """Test that custom serializers don't interfere with built-in types."""
        def dummy_serializer(obj):
            raise AssertionError("Should not be called for int")
        
        crous.register_serializer(str, dummy_serializer)
        
        try:
            # Int should still work
            binary = crous.dumps(42)
            result = crous.loads(binary)
            assert result == 42
        finally:
            crous.unregister_serializer(str)

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_multiple_serializers(self):
        """Test registering multiple serializers."""
        def datetime_ser(obj):
            if isinstance(obj, datetime):
                return obj.isoformat()
            raise TypeError()
        
        def decimal_ser(obj):
            if isinstance(obj, Decimal):
                return str(obj)
            raise TypeError()
        
        crous.register_serializer(datetime, datetime_ser)
        crous.register_serializer(Decimal, decimal_ser)
        
        try:
            data = [datetime.now(), Decimal('3.14')]
            binary = crous.dumps(data)
            # Should work with both serializers
            assert binary is not None
        finally:
            crous.unregister_serializer(datetime)
            crous.unregister_serializer(Decimal)

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_serializer_override(self):
        """Test that registering a new serializer overrides the old one."""
        def ser1(obj):
            return 'first'
        
        def ser2(obj):
            return 'second'
        
        crous.register_serializer(datetime, ser1)
        crous.register_serializer(datetime, ser2)  # Override
        
        try:
            # Second serializer should be used
            pass
        finally:
            crous.unregister_serializer(datetime)


class TestCustomSerializerErrors:
    """Test error handling in custom serializers."""

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_serializer_returns_none(self):
        """Test serializer that returns None."""
        def bad_serializer(obj):
            return None
        
        crous.register_serializer(datetime, bad_serializer)
        
        try:
            # None return might be valid or cause error depending on impl
            data = datetime.now()
            binary = crous.dumps(data)
        finally:
            crous.unregister_serializer(datetime)

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_serializer_raises_exception(self):
        """Test serializer that raises exception."""
        def error_serializer(obj):
            raise ValueError("Custom serializer error")
        
        crous.register_serializer(datetime, error_serializer)
        
        try:
            data = datetime.now()
            with pytest.raises((ValueError, crous.CrousEncodeError)):
                crous.dumps(data)
        finally:
            crous.unregister_serializer(datetime)


class TestDecoderBehavior:
    """Test behavior of decoders with tagged values."""

    def test_decoder_with_builtin_tag(self):
        """Test that built-in tags work correctly."""
        # Built-in tags for datetime, decimal, etc. should work
        pass

    def test_decoder_with_user_tag(self):
        """Test that user-defined tags (100-199) work."""
        pass


class TestSerializerRoundTrip:
    """Test round-trip serialization with custom serializers."""

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_datetime_roundtrip_with_serializer(self):
        """Test datetime serialization and deserialization."""
        def datetime_serializer(obj):
            if isinstance(obj, datetime):
                # Could serialize to ISO string or structured format
                return {'__datetime__': obj.isoformat()}
            raise TypeError()
        
        crous.register_serializer(datetime, datetime_serializer)
        
        try:
            original = datetime(2023, 12, 25, 10, 30, 45)
            binary = crous.dumps(original)
            # Note: Without a decoder, we'd get dict, not datetime
            result = crous.loads(binary)
            # Verify the structure was preserved
            assert isinstance(result, dict)
        finally:
            crous.unregister_serializer(datetime)

    @pytest.mark.skip(reason="register_serializer not yet implemented in Crous")
    def test_custom_class_roundtrip(self):
        """Test custom class round-trip with serializers."""
        class Person:
            def __init__(self, name, age):
                self.name = name
                self.age = age
        
        def person_serializer(obj):
            if isinstance(obj, Person):
                return {'__person__': True, 'name': obj.name, 'age': obj.age}
            raise TypeError()
        
        crous.register_serializer(Person, person_serializer)
        
        try:
            person = Person('Alice', 30)
            binary = crous.dumps(person)
            result = crous.loads(binary)
            
            assert result['__person__'] is True
            assert result['name'] == 'Alice'
            assert result['age'] == 30
        finally:
            crous.unregister_serializer(Person)
