# Contributing to Crous

Thank you for your interest in contributing to Crous! We welcome contributions from everyone. This document provides guidelines and instructions for contributing to the project.

## Code of Conduct

Please be respectful and constructive in all interactions with other contributors and maintainers.

## Getting Started

### Prerequisites

- Python 3.6 or higher
- Git
- A C compiler (gcc/clang on Linux/macOS, MSVC on Windows)

### Setting Up Development Environment

1. **Fork the repository**
   ```bash
   # Click "Fork" on GitHub
   ```

2. **Clone your fork**
   ```bash
   git clone https://github.com/your-username/crous.git
   cd crous
   ```

3. **Create a virtual environment**
   ```bash
   python -m venv env
   source env/bin/activate  # On Windows: env\Scripts\activate
   ```

4. **Install development dependencies**
   ```bash
   pip install -r requirements-dev.txt
   ```

5. **Build the C extension**
   ```bash
   python setup.py build_ext --inplace
   ```

6. **Verify setup**
   ```bash
   pytest  # Run tests to ensure everything works
   ```

## How to Contribute

### Reporting Bugs

Before creating a bug report, please check the [issue tracker](https://github.com/crous-project/crous/issues) to avoid duplicates.

When creating a bug report, include:
- **Clear description** of the issue
- **Python version** (`python --version`)
- **Crous version** (`python -c "import crous; print(crous.__version__)`)
- **Operating system**
- **Minimal reproducible example**
- **Expected vs actual behavior**
- **Error traceback** (if applicable)

### Suggesting Enhancements

Feature requests are welcome! Please provide:
- **Clear description** of the feature
- **Use case** explaining why this feature would be useful
- **Possible implementation** (optional)
- **Examples** of how it would be used

### Submitting Pull Requests

1. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make your changes**
   - Follow the code style (see below)
   - Write clear, descriptive commit messages
   - Add tests for new functionality
   - Update documentation as needed

3. **Run tests and checks**
   ```bash
   # Run tests
   pytest

   # Check code formatting
   black --check crous tests

   # Check imports
   isort --check crous tests

   # Run linter
   flake8 crous tests

   # Type checking
   mypy crous
   ```

4. **Format code automatically**
   ```bash
   black crous tests
   isort crous tests
   ```

5. **Commit your changes**
   ```bash
   git add .
   git commit -m "Concise description of changes"
   ```

6. **Push to your fork**
   ```bash
   git push origin feature/your-feature-name
   ```

7. **Create a Pull Request**
   - Go to the repository on GitHub
   - Click "New Pull Request"
   - Select your branch
   - Fill in the PR template
   - Submit the PR

## Code Style

### Python Code

We follow [PEP 8](https://www.python.org/dev/peps/pep-0008/) with these tools:

- **Formatter**: [Black](https://github.com/psf/black)
- **Import sorter**: [isort](https://pycqa.github.io/isort/)
- **Linter**: [flake8](https://flake8.pycqa.org/)

### C Code

For C code, follow these guidelines:

- Use 4-space indentation
- Keep lines under 100 characters
- Use descriptive variable names
- Add comments for complex logic
- Follow the existing code style in the codebase

### Commit Messages

Write clear, descriptive commit messages:

```
Add feature X to handle Y

- Detailed explanation of what was changed
- Why the change was necessary
- Any relevant issue numbers (closes #123)
```

## Testing

### Writing Tests

- Place tests in the `tests/` directory
- Name files `test_*.py`
- Use pytest fixtures when appropriate
- Include both positive and negative test cases
- Aim for good coverage (80%+ for new code)

### Running Tests

```bash
# Run all tests
pytest

# Run with coverage
pytest --cov=crous

# Run specific test
pytest tests/test_basic.py

# Run in parallel
pytest -n auto

# Show slowest tests
pytest --durations=10
```

## Documentation

### Docstrings

Write clear docstrings for all public functions and classes:

```python
def dumps(obj, *, default=None):
    """Serialize obj to a Crous bytes object.
    
    Args:
        obj: The Python object to serialize
        default: Optional callable for handling non-serializable objects
        
    Returns:
        Bytes representation of obj
        
    Raises:
        CrousEncodeError: If obj cannot be serialized
        
    Examples:
        >>> import crous
        >>> data = {'name': 'Alice', 'age': 30}
        >>> binary = crous.dumps(data)
    """
```

### Documentation Files

- Update `README.md` for user-facing changes
- Update `CHANGELOG.md` for version changes
- Add examples in docstrings
- Update API docs if applicable

## Version Control

### Branch Naming

Use descriptive branch names:
- `feature/description` - New features
- `bugfix/description` - Bug fixes
- `docs/description` - Documentation updates
- `refactor/description` - Code refactoring

### Commit Conventions

Follow conventional commits when possible:
- `feat:` New feature
- `fix:` Bug fix
- `docs:` Documentation
- `test:` Testing
- `refactor:` Code refactoring
- `perf:` Performance improvement

## Release Process

The core team manages releases. Version updates follow [Semantic Versioning](https://semver.org/):

1. Update `__version__` in `crous/__init__.py`
2. Update `version` in `setup.py`
3. Update `CHANGELOG.md`
4. Create a release PR
5. After merge, create a git tag
6. Build and upload to PyPI

## Getting Help

- **GitHub Discussions**: Ask questions and discuss ideas
- **GitHub Issues**: Report bugs and request features
- **Documentation**: Check [crous.readthedocs.io](https://crous.readthedocs.io)
- **Email**: support@crous.dev

## Recognition

Contributors will be:
- Listed in the CONTRIBUTORS file (coming soon)
- Credited in release notes for significant contributions
- Added as maintainers for substantial ongoing contributions

## Legal

By submitting a pull request, you agree that your contribution is licensed under the MIT License, the same license as Crous.

---

Thank you for contributing to Crous! 🎉
