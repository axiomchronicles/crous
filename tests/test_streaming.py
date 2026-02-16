"""
test_streaming.py - Streaming I/O tests

Tests streaming encode/decode functionality including dumps_stream/loads_stream.
Validates that parsed arguments are used directly (fix for C-1 audit issue).
"""

import pytest
import crous
import io


class TestDumpsStreamBasic:
    """Test dumps_stream / loads_stream basic functionality."""

    def test_dumps_stream_basic_roundtrip(self):
        """Test encoding and decoding single record via stream (BytesIO roundtrip)."""
        data = {'record': 1, 'value': 'first'}
        buf = io.BytesIO()
        crous.dumps_stream(data, buf)
        buf.seek(0)
        result = crous.loads_stream(buf)
        assert result == data

    def test_dumps_stream_empty_dict(self):
        """Test streaming an empty dict."""
        data = {}
        buf = io.BytesIO()
        crous.dumps_stream(data, buf)
        buf.seek(0)
        result = crous.loads_stream(buf)
        assert result == data

    def test_dumps_stream_complex_data(self):
        """Test streaming complex nested data."""
        data = {
            'name': 'alice',
            'age': 30,
            'tags': ['dev', 'crous'],
            'active': True,
            'score': 3.14,
        }
        buf = io.BytesIO()
        crous.dumps_stream(data, buf)
        buf.seek(0)
        result = crous.loads_stream(buf)
        assert result == data

    def test_dumps_stream_nested_structures(self):
        """Test streaming deeply nested structures."""
        data = {'outer': {'inner': [1, 2, 3], 'deep': {'key': 'value'}}}
        buf = io.BytesIO()
        crous.dumps_stream(data, buf)
        buf.seek(0)
        result = crous.loads_stream(buf)
        assert result == data

    def test_dumps_stream_list_root(self):
        """Test streaming a list as root object."""
        data = [1, 'two', 3.0, True, None]
        buf = io.BytesIO()
        crous.dumps_stream(data, buf)
        buf.seek(0)
        result = crous.loads_stream(buf)
        assert result == data

    def test_dumps_stream_string_root(self):
        """Test streaming a string as root object."""
        data = 'hello world'
        buf = io.BytesIO()
        crous.dumps_stream(data, buf)
        buf.seek(0)
        result = crous.loads_stream(buf)
        assert result == data

    def test_dumps_stream_int_root(self):
        """Test streaming an integer as root object."""
        data = 42
        buf = io.BytesIO()
        crous.dumps_stream(data, buf)
        buf.seek(0)
        result = crous.loads_stream(buf)
        assert result == data

    def test_dumps_stream_none_root(self):
        """Test streaming None as root object."""
        buf = io.BytesIO()
        crous.dumps_stream(None, buf)
        buf.seek(0)
        result = crous.loads_stream(buf)
        assert result is None

    def test_dumps_stream_bytes_root(self):
        """Test streaming bytes as root object."""
        data = b'\x00\x01\x02\xff'
        buf = io.BytesIO()
        crous.dumps_stream(data, buf)
        buf.seek(0)
        result = crous.loads_stream(buf)
        assert result == data


class TestDumpsStreamKwargs:
    """Test that keyword argument passing works correctly (C-1 fix validation)."""

    def test_dumps_stream_all_kwargs(self):
        """Test dumps_stream with all keyword arguments."""
        data = {'a': 1}
        buf = io.BytesIO()
        crous.dumps_stream(obj=data, fp=buf, default=None)
        buf.seek(0)
        result = crous.loads_stream(fp=buf, object_hook=None)
        assert result == data

    def test_dumps_stream_obj_kwarg_fp_positional(self):
        """Test mixed positional/keyword args."""
        data = {'key': 'value'}
        buf = io.BytesIO()
        # obj as positional, fp as keyword - should work
        crous.dumps_stream(data, fp=buf)
        buf.seek(0)
        result = crous.loads_stream(buf)
        assert result == data

    def test_dumps_stream_with_default_kwarg(self):
        """Test that the default= kwarg is properly forwarded."""
        data = {'key': 'value'}
        buf = io.BytesIO()
        crous.dumps_stream(data, buf, default=None)
        buf.seek(0)
        result = crous.loads_stream(buf)
        assert result == data

    def test_dumps_stream_default_func_called(self):
        """Test that default function is called for unsupported types."""
        class CustomObj:
            def __init__(self, val):
                self.val = val

        def custom_default(obj):
            if isinstance(obj, CustomObj):
                return {'__custom__': obj.val}
            raise TypeError(f"Cannot serialize {type(obj)}")

        data = {'item': CustomObj(42)}
        buf = io.BytesIO()
        crous.dumps_stream(data, buf, default=custom_default)
        buf.seek(0)
        result = crous.loads_stream(buf)
        assert result == {'item': {'__custom__': 42}}

    def test_loads_stream_object_hook_kwarg(self):
        """Test loads_stream with object_hook keyword argument."""
        data = {'x': 1, 'y': 2}
        buf = io.BytesIO()
        crous.dumps_stream(data, buf)
        buf.seek(0)

        hook_called = []
        def my_hook(d):
            hook_called.append(d)
            return d

        result = crous.loads_stream(buf, object_hook=my_hook)
        assert result == data
        # object_hook should have been called for the dict
        assert len(hook_called) > 0


class TestDumpsStreamErrorHandling:
    """Test error handling in dumps_stream / loads_stream."""

    def test_dumps_stream_no_write_method(self):
        """Test that dumps_stream raises TypeError for non-writable fp."""
        with pytest.raises(TypeError):
            crous.dumps_stream({'a': 1}, "not_a_file")

    def test_loads_stream_no_read_method(self):
        """Test that loads_stream raises TypeError for non-readable fp."""
        with pytest.raises(TypeError):
            crous.loads_stream("not_a_file")

    def test_loads_stream_empty_buffer(self):
        """Test that loads_stream raises error on empty buffer."""
        buf = io.BytesIO(b'')
        with pytest.raises((crous.CrousDecodeError, crous.CrousError)):
            crous.loads_stream(buf)

    def test_loads_stream_corrupted_data(self):
        """Test that loads_stream raises error on corrupted data."""
        buf = io.BytesIO(b'\x00\x01\x02\x03\x04\x05')
        with pytest.raises((crous.CrousDecodeError, crous.CrousError)):
            crous.loads_stream(buf)

    def test_dumps_stream_produces_same_as_dumps(self):
        """Test that dumps_stream output matches dumps output."""
        data = {'key': 'value', 'num': 42, 'list': [1, 2, 3]}
        
        # Get bytes from dumps
        expected = crous.dumps(data)
        
        # Get bytes from dumps_stream
        buf = io.BytesIO()
        crous.dumps_stream(data, buf)
        actual = buf.getvalue()
        
        assert actual == expected


class TestStreamingMultipleRecords:
    """Test encoding and decoding multiple records in sequence."""

    def test_multiple_records_sequential(self):
        """Test encoding multiple records to same stream."""
        records = [
            {'id': 1, 'value': 'first'},
            {'id': 2, 'value': 'second'},
            {'id': 3, 'value': 'third'},
        ]
        buf = io.BytesIO()
        
        # Write each record
        encoded_parts = []
        for record in records:
            data = crous.dumps(record)
            encoded_parts.append(data)
            buf.write(data)
        
        # Read back and decode each
        buf.seek(0)
        for i, expected_data in enumerate(encoded_parts):
            chunk = buf.read(len(expected_data))
            result = crous.loads(chunk)
            assert result == records[i], f"Record {i} mismatch"

    def test_streaming_with_file_object(self, tmp_path):
        """Test streaming with actual file objects."""
        filepath = tmp_path / "test.crous"
        data = {'key': 'value', 'nums': [1, 2, 3]}
        
        with open(filepath, 'wb') as f:
            crous.dumps_stream(data, f)
        
        with open(filepath, 'rb') as f:
            result = crous.loads_stream(f)
        
        assert result == data

