"""
test_atomic_write.py - Atomic write and consistency tests

Tests atomic write behavior and data consistency guarantees.
"""

import pytest
import crous
import tempfile
import os


class TestAtomicWriteBehavior:
    """Test atomic write characteristics."""

    def test_dump_creates_valid_file(self, tmp_path):
        """Test that dump creates a valid file."""
        data = {'key': 'value', 'list': [1, 2, 3]}
        file_path = tmp_path / "test.crous"
        
        crous.dump(data, str(file_path))
        
        # Verify file exists and has content
        assert file_path.exists()
        assert file_path.stat().st_size > 0

    def test_load_written_file(self, tmp_path):
        """Test that we can load a file written with dump."""
        data = {'key': 'value'}
        file_path = tmp_path / "test.crous"
        
        crous.dump(data, str(file_path))
        result = crous.load(str(file_path))
        
        assert result == data

    def test_partial_write_not_readable(self, tmp_path):
        """Test that partially written file cannot be read."""
        file_path = tmp_path / "partial.crous"
        
        # Write incomplete data
        with open(file_path, 'wb') as f:
            f.write(b'\x00\x01\x02')  # Incomplete/invalid data
        
        # Should fail to load
        with pytest.raises(crous.CrousDecodeError):
            crous.load(str(file_path))

    def test_overwrite_existing_file(self, tmp_path):
        """Test that dump overwrites existing file."""
        file_path = tmp_path / "test.crous"
        
        # Write first version
        crous.dump({'version': 1}, str(file_path))
        first_size = file_path.stat().st_size
        
        # Write second version
        crous.dump({'version': 2, 'extra': 'data'}, str(file_path))
        second_size = file_path.stat().st_size
        
        # Verify it's overwritten
        result = crous.load(str(file_path))
        assert result['version'] == 2

    def test_multiple_rapid_writes(self, tmp_path):
        """Test multiple rapid writes don't corrupt data."""
        for i in range(10):
            file_path = tmp_path / f"test_{i}.crous"
            data = {'iteration': i, 'data': 'x' * 1000}
            crous.dump(data, str(file_path))
            result = crous.load(str(file_path))
            assert result == data
