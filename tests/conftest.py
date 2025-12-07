"""
Shared test fixtures and configuration for Crous test suite.
"""

import pytest
import tempfile
import os
from pathlib import Path


@pytest.fixture
def temp_file():
    """Create a temporary file for testing."""
    fd, path = tempfile.mkstemp(suffix='.crous')
    os.close(fd)
    yield path
    if os.path.exists(path):
        os.unlink(path)


@pytest.fixture
def temp_dir():
    """Create a temporary directory for testing."""
    path = tempfile.mkdtemp()
    yield path
    import shutil
    if os.path.exists(path):
        shutil.rmtree(path)


@pytest.fixture
def sample_data():
    """Provide various sample data for round-trip testing."""
    return {
        'null': None,
        'bool_true': True,
        'bool_false': False,
        'int_zero': 0,
        'int_positive': 42,
        'int_negative': -100,
        'int_large': 9223372036854775807,  # max int64
        'float_zero': 0.0,
        'float_positive': 3.14159,
        'float_negative': -2.71828,
        'string_empty': '',
        'string_ascii': 'hello world',
        'string_unicode': '你好世界🌍',
        'bytes_empty': b'',
        'bytes_data': b'\x00\x01\x02\x03\xff',
        'list_empty': [],
        'list_ints': [1, 2, 3, 4, 5],
        'list_mixed': [1, 'two', 3.0, None, True],
        'dict_empty': {},
        'dict_simple': {'a': 1, 'b': 2},
        'dict_nested': {'outer': {'inner': 'value'}},
    }
