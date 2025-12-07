"""
test_regressions.py - Regression and API surface tests

Tests for specific regressions, API completeness, and autocomplete.
"""

import pytest
import crous


class TestAPICompleteness:
    """Test that all public API is available."""

    def test_dumps_exists(self):
        """Test dumps function exists and is callable."""
        assert hasattr(crous, 'dumps')
        assert callable(crous.dumps)

    def test_loads_exists(self):
        """Test loads function exists and is callable."""
        assert hasattr(crous, 'loads')
        assert callable(crous.loads)

    def test_dump_exists(self):
        """Test dump function exists and is callable."""
        assert hasattr(crous, 'dump')
        assert callable(crous.dump)

    def test_load_exists(self):
        """Test load function exists and is callable."""
        assert hasattr(crous, 'load')
        assert callable(crous.load)

    def test_register_serializer_exists(self):
        """Test register_serializer exists."""
        assert hasattr(crous, 'register_serializer')
        assert callable(crous.register_serializer)

    def test_unregister_serializer_exists(self):
        """Test unregister_serializer exists."""
        assert hasattr(crous, 'unregister_serializer')
        assert callable(crous.unregister_serializer)

    def test_register_decoder_exists(self):
        """Test register_decoder exists."""
        assert hasattr(crous, 'register_decoder')
        assert callable(crous.register_decoder)

    def test_unregister_decoder_exists(self):
        """Test unregister_decoder exists."""
        assert hasattr(crous, 'unregister_decoder')
        assert callable(crous.unregister_decoder)

    def test_crous_error_exists(self):
        """Test CrousError exception class exists."""
        assert hasattr(crous, 'CrousError')
        assert issubclass(crous.CrousError, Exception)

    def test_crous_encode_error_exists(self):
        """Test CrousEncodeError exception class exists."""
        assert hasattr(crous, 'CrousEncodeError')
        assert issubclass(crous.CrousEncodeError, crous.CrousError)

    def test_crous_decode_error_exists(self):
        """Test CrousDecodeError exception class exists."""
        assert hasattr(crous, 'CrousDecodeError')
        assert issubclass(crous.CrousDecodeError, crous.CrousError)

    def test_crous_encoder_exists(self):
        """Test CrousEncoder class exists."""
        assert hasattr(crous, 'CrousEncoder')

    def test_crous_decoder_exists(self):
        """Test CrousDecoder class exists."""
        assert hasattr(crous, 'CrousDecoder')


class TestAutoCompleteExportedNames:
    """Test that autocomplete gets correct exported names."""

    def test_dir_crous_completeness(self):
        """Test that dir(crous) includes all public names."""
        exported = dir(crous)
        
        required = [
            'dumps', 'loads', 'dump', 'load',
            'CrousEncoder', 'CrousDecoder',
            'register_serializer', 'unregister_serializer',
            'register_decoder', 'unregister_decoder',
            'CrousError', 'CrousEncodeError', 'CrousDecodeError',
        ]
        
        for name in required:
            assert name in exported, f"{name} not in dir(crous)"

    def test_all_attribute(self):
        """Test that __all__ is properly defined."""
        if hasattr(crous, '__all__'):
            assert isinstance(crous.__all__, list)
            # Should contain at least the main functions
            assert 'dumps' in crous.__all__
            assert 'loads' in crous.__all__

    def test_public_functions_callable(self):
        """Test that all public functions are callable."""
        public_funcs = [
            'dumps', 'loads', 'dump', 'load',
            'register_serializer', 'unregister_serializer',
            'register_decoder', 'unregister_decoder',
        ]
        
        for func_name in public_funcs:
            func = getattr(crous, func_name)
            assert callable(func), f"{func_name} is not callable"


class TestVersionInfo:
    """Test version information."""

    def test_version_exists(self):
        """Test that __version__ is defined."""
        assert hasattr(crous, '__version__')
        assert isinstance(crous.__version__, str)

    def test_version_format(self):
        """Test that version follows semantic versioning."""
        version = crous.__version__
        parts = version.split('.')
        assert len(parts) >= 2, "Version should be X.Y.Z format"


class TestRegressionSpecific:
    """Test specific regression cases."""

    def test_regression_int_vs_float_in_list(self):
        """Regression: int vs float type preserved in list."""
        data = [1, 1.0]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        assert isinstance(result[0], int)
        assert isinstance(result[1], float)

    def test_regression_empty_string_vs_none(self):
        """Regression: empty string not confused with None."""
        data = [None, '', 'text']
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        assert result[0] is None
        assert result[1] == ''
        assert result[2] == 'text'

    def test_regression_false_vs_zero(self):
        """Regression: False not confused with 0."""
        data = [False, 0, True, 1]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        assert result[0] is False
        assert isinstance(result[0], bool)
        assert result[1] == 0
        assert isinstance(result[1], int)
        assert result[2] is True
        assert result[3] == 1

    def test_regression_dict_with_numeric_values(self):
        """Regression: dict values maintain types."""
        data = {'int': 1, 'float': 1.0, 'bool': True}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        assert isinstance(result['int'], int)
        assert isinstance(result['float'], float)
        assert isinstance(result['bool'], bool)

    def test_regression_unicode_key_preservation(self):
        """Regression: Unicode dict keys preserved."""
        data = {'键': 1, '鍵': 2, 'key': 3}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        assert result == data
        assert '键' in result
        assert '鍵' in result
