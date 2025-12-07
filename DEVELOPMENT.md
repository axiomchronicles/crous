# Crous Development Setup

Complete guide for setting up Crous for development.

## System Requirements

- Python 3.6 or higher
- Git
- C compiler (gcc/clang on Unix, MSVC on Windows)
- 500MB free disk space

### Installation by OS

#### macOS

```bash
# Using Homebrew
brew install python3 git

# Verify installation
python3 --version
gcc --version
```

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install python3 python3-venv git build-essential
```

#### Windows

1. Download Python from https://www.python.org/downloads/
2. Download Git from https://git-scm.com/
3. Download Visual Studio Build Tools or MinGW
4. Add all to PATH

## Setup Steps

### 1. Clone Repository

```bash
git clone https://github.com/crous-project/crous.git
cd crous
```

### 2. Create Virtual Environment

```bash
# macOS/Linux
python3 -m venv env
source env/bin/activate

# Windows
python -m venv env
env\Scripts\activate
```

### 3. Upgrade pip

```bash
pip install --upgrade pip
```

### 4. Install Dependencies

```bash
# Install development dependencies
pip install -r requirements-dev.txt

# Install runtime (for testing)
pip install -e .
```

### 5. Build C Extension

```bash
# Build in-place
python setup.py build_ext --inplace

# Verify build
python -c "import crous; print(crous.__version__)"
```

### 6. Verify Installation

```bash
# Run quick test
python -m pytest tests/test_basic.py -v

# Run full test suite
pytest

# Check coverage
pytest --cov=crous
```

## Development Workflow

### Running Tests

```bash
# Run all tests
pytest

# Run specific test file
pytest tests/test_basic.py

# Run with verbose output
pytest -v

# Run with coverage
pytest --cov=crous --cov-report=html

# Run in parallel
pytest -n auto
```

### Code Quality Checks

```bash
# Format code
black crous tests

# Sort imports
isort crous tests

# Lint code
flake8 crous tests

# Type check
mypy crous

# All checks (shorthand)
pre-commit run --all-files  # if pre-commit is set up
```

### Building C Extension

```bash
# Clean previous build
python setup.py clean --all

# Build extension
python setup.py build_ext --inplace

# Build and install for testing
pip install -e .
```

### Documentation

```bash
# Build documentation locally
cd docs/crous-docx
npm install
npm start

# Open http://localhost:3000 in browser
```

## Common Tasks

### Create Feature Branch

```bash
git checkout -b feature/my-feature
```

### Commit Changes

```bash
git add .
git commit -m "Brief description of changes"
```

### Push Changes

```bash
git push origin feature/my-feature
```

### Create Pull Request

1. Go to GitHub repository
2. Click "Compare & pull request"
3. Fill in PR template
4. Submit PR

### Update from Main

```bash
git fetch origin
git rebase origin/main
# or
git merge origin/main
```

### Stash Changes

```bash
# Save changes
git stash

# List stashes
git stash list

# Restore stash
git stash pop
```

## Debugging

### Enable Debug Output

```python
import logging
logging.basicConfig(level=logging.DEBUG)

import crous
# Now all debug messages will be printed
```

### Run with Debugger

```bash
# Using pdb
python -m pdb -c "from tests import test_basic; test_basic.test_basic()"

# Using pytest with pdb
pytest --pdb

# Drop to debugger on failure
pytest --pdb-trace
```

### Memory Profiling

```bash
pip install memory-profiler

python -m memory_profiler script.py
```

### Performance Profiling

```bash
pip install py-spy

py-spy record -o profile.svg python script.py
```

## Troubleshooting

### Build Fails

```bash
# Clean and rebuild
python setup.py clean --all
python setup.py build_ext --inplace

# Check compiler
gcc --version  # macOS/Linux
cl.exe /?     # Windows
```

### Tests Fail

```bash
# Run with verbose output
pytest -v

# Run specific failing test
pytest tests/test_file.py::TestClass::test_method -v

# Check Python version
python --version

# Verify dependencies
pip list
```

### Import Errors

```bash
# Verify C extension built
ls crous/*.so  # macOS/Linux
dir crous\*.pyd  # Windows

# Rebuild if missing
python setup.py build_ext --inplace
```

### Virtual Environment Issues

```bash
# Recreate virtual environment
deactivate
rm -rf env
python3 -m venv env
source env/bin/activate
pip install -r requirements-dev.txt
```

## Environment Variables

```bash
# Set Python path
export PYTHONPATH=/path/to/crous:$PYTHONPATH

# Enable C extension debug info
export CFLAGS="-g -O0"

# Disable bytecode
export PYTHONDONTWRITEBYTECODE=1
```

## Editor Configuration

### VSCode

Create `.vscode/settings.json`:

```json
{
    "python.defaultInterpreterPath": "${workspaceFolder}/env/bin/python",
    "python.linting.enabled": true,
    "python.linting.pylintEnabled": false,
    "python.linting.flake8Enabled": true,
    "python.formatting.provider": "black",
    "[python]": {
        "editor.defaultFormatter": "ms-python.python",
        "editor.formatOnSave": true
    }
}
```

### PyCharm

1. Open Project Settings
2. Go to Project → Python Interpreter
3. Click "Add" and select the `env/bin/python` path
4. Go to Tools → Python Integrated Tools
5. Set Default test runner to pytest

## Pre-commit Hooks (Optional)

```bash
# Install pre-commit
pip install pre-commit

# Create .pre-commit-config.yaml
cat > .pre-commit-config.yaml << 'EOF'
repos:
  - repo: https://github.com/psf/black
    rev: 23.1.0
    hooks:
      - id: black
  - repo: https://github.com/PyCQA/isort
    rev: 5.12.0
    hooks:
      - id: isort
  - repo: https://github.com/PyCQA/flake8
    rev: 6.0.0
    hooks:
      - id: flake8
EOF

# Install git hooks
pre-commit install

# Run manually
pre-commit run --all-files
```

## Useful Resources

- [Python Development Guide](https://devguide.python.org/)
- [Pytest Documentation](https://docs.pytest.org/)
- [Black Code Formatter](https://black.readthedocs.io/)
- [Building C Extensions](https://docs.python.org/3/extending/building.html)

## Getting Help

- GitHub Issues: https://github.com/crous-project/crous/issues
- GitHub Discussions: https://github.com/crous-project/crous/discussions
- Email: support@crous.dev

---

Happy coding! 🚀
