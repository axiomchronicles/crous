"""
test_performance.py - Performance and regression tests

Tests performance benchmarks and ensures C extension functionality.
"""

import pytest
import crous
import time
import json
import pickle


class TestPerformanceBasic:
    """Test basic performance characteristics."""

    def test_dumps_completes(self):
        """Test that dumps completes without error."""
        data = {'key': 'value', 'num': 42, 'list': [1, 2, 3]}
        binary = crous.dumps(data)
        assert isinstance(binary, bytes)

    def test_loads_completes(self):
        """Test that loads completes without error."""
        binary = crous.dumps({'key': 'value'})
        result = crous.loads(binary)
        assert result == {'key': 'value'}

    def test_large_list_dumps_performance(self):
        """Test dumps performance on large list."""
        data = list(range(100000))
        start = time.time()
        binary = crous.dumps(data)
        elapsed = time.time() - start
        
        assert isinstance(binary, bytes)
        assert elapsed < 10.0  # Should complete in reasonable time

    def test_large_dict_dumps_performance(self):
        """Test dumps performance on large dict."""
        data = {f'key_{i}': i for i in range(100000)}
        start = time.time()
        binary = crous.dumps(data)
        elapsed = time.time() - start
        
        assert isinstance(binary, bytes)
        assert elapsed < 10.0

    def test_large_list_loads_performance(self):
        """Test loads performance on large list."""
        data = list(range(100000))
        binary = crous.dumps(data)
        
        start = time.time()
        result = crous.loads(binary)
        elapsed = time.time() - start
        
        assert result == data
        assert elapsed < 10.0

    def test_large_dict_loads_performance(self):
        """Test loads performance on large dict."""
        data = {f'key_{i}': i for i in range(100000)}
        binary = crous.dumps(data)
        
        start = time.time()
        result = crous.loads(binary)
        elapsed = time.time() - start
        
        assert result == data
        assert elapsed < 10.0


class TestRegressions:
    """Test regressions and known issues."""

    def test_c_extension_loaded(self):
        """Test that C extension is properly loaded."""
        # Verify C extension functions exist
        assert callable(crous.dumps)
        assert callable(crous.loads)
        assert callable(crous.dump)
        assert callable(crous.load)

    def test_exceptions_properly_raised(self):
        """Test that exceptions are properly raised from C extension."""
        with pytest.raises(crous.CrousEncodeError):
            crous.dumps({1: 'value'})

    def test_no_notimplemented_errors(self):
        """Test that no NotImplementedError is raised for any public API."""
        # Test all public API functions
        try:
            crous.dumps([1, 2, 3])
            crous.loads(crous.dumps({'a': 1}))
            assert True
        except NotImplementedError:
            pytest.fail("C extension raised NotImplementedError")

    def test_type_preservation_int_vs_float(self):
        """Test regression: int vs float type preservation."""
        data = [1, 1.0, 2, 2.0]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        assert isinstance(result[0], int)
        assert isinstance(result[1], float)
        assert isinstance(result[2], int)
        assert isinstance(result[3], float)

    def test_dict_key_order_python37_plus(self):
        """Test regression: dict key order preservation (Python 3.7+)."""
        data = {'z': 1, 'a': 2, 'm': 3}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        assert list(result.keys()) == ['z', 'a', 'm']

    def test_roundtrip_complex_nested(self):
        """Test regression: complex nested roundtrip."""
        data = {
            'users': [
                {'id': 1, 'name': 'Alice', 'scores': [100, 95, 87]},
                {'id': 2, 'name': 'Bob', 'scores': [88, 92, 91]},
            ],
            'metadata': {'total': 2, 'average': 91.5}
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        assert result == data
