---
sidebar_position: 1
---

# Developer Guide

## Contributing to Crous

This guide covers development setup, testing, and contributing to Crous.

## Development Setup

### Prerequisites

- Python 3.7 or later
- C compiler (gcc, clang, or MSVC)
- Build tools: gcc/clang (macOS: Command Line Tools via `xcode-select --install`)

### Clone and Build

```bash
# Clone repository
git clone https://github.com/crous/crous.git
cd crous

# Create virtual environment
python3 -m venv env
source env/bin/activate  # or `env\Scripts\activate` on Windows

# Install in development mode
pip install -e .

# Verify installation
python -c "import crous; print(crous.__version__)"
```

### Building from Source

```bash
# Clean build
python setup.py clean --all

# Build extension
python setup.py build_ext --inplace

# Run tests
pytest tests/ -v
```

## Architecture Overview

### File Structure

```
crous/
├── crous.c          # C core library (encoding/decoding)
├── crous.h          # C header (type definitions, API)
├── pycrous.c        # Python C extension (Python bindings)
├── __init__.py      # Python wrapper (high-level API)
├── setup.py         # Build configuration
└── tests/           # Test suite
    ├── test_basic.py
    ├── test_containers.py
    ├── test_error_handling.py
    ├── test_nested.py
    ├── test_scalars.py
    ├── test_custom_serializers.py
    ├── test_streaming.py
    ├── test_large_files.py
    ├── test_performance.py
    ├── test_tagged_types.py
    ├── test_atomic_write.py
    └── test_regressions.py
```

### C Core (crous.c)

The core library handles:

1. **Value representation**: `crous_value` tree structure
2. **Type system**: Type tags (NULL, BOOL, INT, etc.)
3. **Encoding**: Convert values to binary format
4. **Decoding**: Parse binary format to values
5. **Memory management**: Allocation/deallocation
6. **Error handling**: Error codes and reporting

Key functions:

```c
/* Encoding */
crous_err_t crous_encode(crous_context *ctx, const crous_value *value,
                         uint8_t **out_buf, size_t *out_size);

/* Decoding */
crous_err_t crous_decode(crous_context *ctx, const uint8_t *buf, 
                         size_t buf_size, crous_value **out_value);

/* Value constructors */
crous_value *crous_value_new_int(int64_t v);
crous_value *crous_value_new_string(const char *data, size_t len);
// ... more constructors
```

### Python Extension (pycrous.c)

The C extension provides:

1. **Type conversion**: Python ↔ C value tree
2. **Exception handling**: Error propagation to Python
3. **Reference counting**: Memory management for Python objects
4. **API functions**: Expose C functions to Python

### Python Wrapper (__init__.py)

High-level API providing:

1. **File path handling**: Convert str paths to file objects
2. **Module exports**: Define public API
3. **Docstrings**: IDE support and documentation
4. **Version info**: Module metadata

## Running Tests

### Basic Test Execution

```bash
# Run all tests
pytest tests/ -v

# Run specific test file
pytest tests/test_basic.py -v

# Run specific test class
pytest tests/test_basic.py::TestBasicDumpsLoads -v

# Run specific test method
pytest tests/test_basic.py::TestBasicDumpsLoads::test_none_roundtrip -v
```

### Test Coverage

```bash
# Run with coverage
pytest tests/ --cov=crous --cov-report=html

# View coverage report
open htmlcov/index.html  # macOS
xdg-open htmlcov/index.html  # Linux
```

### Performance Tests

```bash
# Run performance benchmarks
pytest tests/test_performance.py -v

# Profile specific operation
python -m cProfile -s cumulative -m pytest tests/test_performance.py
```

## Code Style Guidelines

### Python Code

Follow [PEP 8](https://www.python.org/dev/peps/pep-0008/):

```python
# Good
def dumps(obj, *, default=None):
    """Serialize object to bytes."""
    ...

# Bad
def dumps(obj,default=None):
    """Serialize object to bytes"""
    ...
```

Use type hints where helpful:

```python
def dumps(obj: Any, *, default: Optional[Callable] = None) -> bytes:
    """Serialize object to bytes."""
    ...
```

### C Code

- Use consistent indentation (2 or 4 spaces, choose one)
- Follow kernel-like naming: `crous_value_new_int()`
- Comment complex logic
- Use meaningful variable names
- Keep functions focused

```c
/* Good: Clear naming and organization */
crous_value *crous_value_new_string(const char *data, size_t len) {
    crous_value *v = malloc(sizeof(crous_value));
    if (!v) return NULL;
    
    v->type = CROUS_TYPE_STRING;
    v->data.s.data = malloc(len);
    if (!v->data.s.data) {
        free(v);
        return NULL;
    }
    
    memcpy(v->data.s.data, data, len);
    v->data.s.len = len;
    return v;
}
```

## Adding New Features

### Example: Adding a New Built-in Type

Suppose you want to add support for `set` type:

1. **Define type code** in `crous.h`:
```c
#define CROUS_TYPE_SET 0x0B
```

2. **Add value structure** in `crous.c`:
```c
struct {
    crous_value **items;
    size_t len;
} set;
```

3. **Implement encoding** in `crous.c`:
```c
case CROUS_TYPE_SET:
    // Write set-specific encoding
    break;
```

4. **Implement decoding** in `crous.c`:
```c
case CROUS_TYPE_SET:
    // Read set-specific encoding
    break;
```

5. **Add Python conversion** in `pycrous.c`:
```c
// Python set to C value
case PySet_Check(obj):
    // Convert Python set to crous_value
    break;
```

6. **Write tests** in `tests/test_sets.py`:
```python
def test_set_roundtrip():
    data = {'items': {1, 2, 3}}
    binary = crous.dumps(data)
    result = crous.loads(binary)
    assert result['items'] == {1, 2, 3}
```

## Performance Profiling

### Profiling Python Code

```python
import cProfile
import crous

def benchmark():
    data = {'items': list(range(10000))}
    for _ in range(100):
        crous.dumps(data)

cProfile.run('benchmark()')
```

### Profiling C Code

```bash
# macOS
instruments -t "System Trace" -o trace.trace python -m pytest tests/test_performance.py

# Linux (with perf-tools)
perf record -g python -m pytest tests/test_performance.py
perf report
```

## Debugging Strategies

### Python Debugging

```python
import pdb
import crous

def debug_serialization():
    data = {'complex': {'nested': [1, 2, 3]}}
    pdb.set_trace()  # Debugger stops here
    binary = crous.dumps(data)

debug_serialization()
```

### C Debugging

```bash
# macOS/Linux with GDB
gdb --args python -m pytest tests/test_basic.py
(gdb) break crous_encode
(gdb) run

# Or use LLDB on macOS
lldb -- python -m pytest tests/test_basic.py
(lldb) breakpoint set --name crous_encode
(lldb) run
```

### Common Issues

**ImportError when running tests:**
```bash
# Rebuild extension
python setup.py build_ext --inplace
pytest tests/
```

**Segmentation fault:**
- Enable address sanitizer: `CFLAGS="-fsanitize=address"`
- Use GDB/LLDB for stack trace

## PR Workflow

1. **Fork repository** on GitHub
2. **Create feature branch**: `git checkout -b feature/my-feature`
3. **Make changes** and commit with clear messages
4. **Run tests**: `pytest tests/ -v`
5. **Check coverage**: `pytest --cov=crous`
6. **Push branch**: `git push origin feature/my-feature`
7. **Open PR** on GitHub with description
8. **Address review comments**
9. **Merge when approved**

## PR Checklist

- [ ] Tests pass: `pytest tests/ -v`
- [ ] Coverage maintained or improved
- [ ] Code follows style guidelines
- [ ] Docstrings added/updated
- [ ] CHANGELOG entry added (if applicable)
- [ ] No breaking changes (or noted in PR)

## Release Process

1. **Update version** in `setup.py` and `__init__.py`
2. **Update CHANGELOG** with changes
3. **Create release branch**: `git checkout -b release/v2.1.0`
4. **Tag commit**: `git tag v2.1.0`
5. **Build distribution**: `python setup.py sdist bdist_wheel`
6. **Upload to PyPI**: `twine upload dist/*`
7. **Push tag**: `git push origin v2.1.0`
8. **Create GitHub Release** with changelog

## Resources and Support

- **Architecture Details**: See [Architecture](../internals/architecture.md)
- **Binary Format**: See [API Reference](../api/reference.md)
- **Test Examples**: Browse `tests/` directory
- **Issues**: [GitHub Issues](https://github.com/crous/crous/issues)
- **Discussions**: [GitHub Discussions](https://github.com/crous/crous/discussions)

## Questions?

- Check existing GitHub issues and discussions
- Review test suite for usage examples
- Read code comments for implementation details
- Ask in GitHub Discussions

Thank you for contributing to Crous!
