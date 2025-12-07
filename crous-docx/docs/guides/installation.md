---
sidebar_position: 1
description: Install Crous on Linux, macOS, Windows, or in Docker with step-by-step instructions
---

import Tabs from '@theme/Tabs';
import TabItem from '@theme/TabItem';

# Installation Guide

Get Crous up and running in minutes with our comprehensive installation guide.

## ⚡ Quick Install

<Tabs groupId="install-method">
  <TabItem value="pip" label="PyPI (Recommended)">

```bash
pip install crous
```

  </TabItem>
  <TabItem value="conda" label="Conda">

```bash
conda install -c conda-forge crous
```

  </TabItem>
  <TabItem value="source" label="From Source">

```bash
git clone https://github.com/crous/crous.git
cd crous
pip install -e .
```

  </TabItem>
</Tabs>

:::info **Verify Installation**
```bash
python -c "import crous; print(f'✓ Crous {crous.__version__} installed!')"
```
:::

## System Requirements

:::note **Before You Install**
Crous requires a C compiler to build from source. Most systems have this, but if not, follow the troubleshooting steps below.
:::

### Minimum Requirements

| Requirement | Version |
|-------------|---------|
| **Python** | 3.7 or later |
| **C Compiler** | gcc, clang, or MSVC |
| **Memory** | 100 MB free |
| **Disk** | 10 MB |

### Platform Support

| Platform | Status | Notes |
|----------|--------|-------|
| 🐧 **Linux** (x86_64) | ✅ Full | gcc/clang required |
| 🍎 **macOS** | ✅ Full | Xcode Command Line Tools required |
| 🪟 **Windows** | ✅ Full | Visual Studio Build Tools required |
| 💪 **ARM64** | ⚠️ Experimental | Linux only |

## Installation Methods

### Method 1: PyPI (Recommended) 🎯

The easiest way to install Crous:

```bash
pip install crous
```

This will download and install:
- Pre-built wheel (if available for your platform)
- Or compile from source automatically

**Upgrade to latest version:**
```bash
pip install --upgrade crous
```

### Method 2: From Source 🔧

For development or if pre-built wheels aren't available:

```bash
# Clone the repository
git clone https://github.com/crous/crous.git
cd crous

# Install in development mode (editable)
pip install -e .

# Or install normally
pip install .
```

### Method 3: Conda 📦

If you're using Conda:

```bash
conda install -c conda-forge crous
```

## Platform-Specific Setup

<Tabs groupId="platform">
  <TabItem value="linux" label="🐧 Linux">

### Ubuntu/Debian
```bash
# Update package list
sudo apt-get update

# Install build tools
sudo apt-get install build-essential python3-dev

# Install Crous
pip install crous
```

### Fedora/RHEL
```bash
sudo dnf install gcc python3-devel
pip install crous
```

### Arch Linux
```bash
sudo pacman -S gcc python
pip install crous
```

  </TabItem>
  <TabItem value="macos" label="🍎 macOS">

### Option 1: Xcode Command Line Tools (Recommended)
```bash
xcode-select --install
pip install crous
```

### Option 2: Full Xcode
Download from App Store, then:
```bash
pip install crous
```

### Option 3: Homebrew
```bash
brew install python3
pip install crous
```

  </TabItem>
  <TabItem value="windows" label="🪟 Windows">

### Visual Studio Build Tools
1. Download [Visual Studio Community](https://visualstudio.microsoft.com/downloads/)
2. Run installer and select "Desktop development with C++"
3. Ensure "MSVC v142+" is checked
4. Complete installation
5. Open PowerShell and install:
```powershell
pip install crous
```

### Alternative: MinGW
```powershell
# If you have MinGW installed
pip install crous
```

  </TabItem>
</Tabs>

## Virtual Environment Setup

:::tip **Best Practice**
Always use a virtual environment to avoid conflicts with system packages.
:::

<Tabs groupId="shell">
  <TabItem value="bash" label="Linux/macOS">

```bash
# Create virtual environment
python3 -m venv venv

# Activate it
source venv/bin/activate

# Install Crous
pip install crous

# Verify
python -c "import crous; print(crous.__version__)"

# Deactivate when done
deactivate
```

  </TabItem>
  <TabItem value="powershell" label="Windows PowerShell">

```powershell
# Create virtual environment
python -m venv venv

# Activate it
.\venv\Scripts\Activate.ps1

# Install Crous
pip install crous

# Verify
python -c "import crous; print(crous.__version__)"

# Deactivate when done
deactivate
```

  </TabItem>
</Tabs>

## Docker Setup

### Docker Image

Create a `Dockerfile`:

```dockerfile
FROM python:3.11-slim

WORKDIR /app

# Install Crous
RUN pip install --no-cache-dir crous

# Copy your application
COPY . .

# Run your app
CMD ["python", "app.py"]
```

Build and run:

```bash
# Build the image
docker build -t my-crous-app .

# Run the container
docker run -it my-crous-app

# Or with volume mount
docker run -it -v $(pwd):/app my-crous-app
```

### Docker Compose

`docker-compose.yml`:

```yaml
version: '3.8'
services:
  app:
    build: .
    volumes:
      - .:/app
    command: python app.py
```

## Verification

### Quick Test

```python
import crous

# Create test data
data = {'message': 'Hello, Crous!', 'values': [1, 2, 3]}

# Serialize
binary = crous.dumps(data)
print(f"✓ Serialized to {len(binary)} bytes")

# Deserialize
result = crous.loads(binary)
assert result == data
print("✓ Deserialization successful!")
print(f"✓ Crous {crous.__version__} is working!")
```

### Check Installation

```bash
# Show version
python -c "import crous; print(crous.__version__)"

# Show installation path
python -c "import crous; print(crous.__file__)"

# Show installed package info
pip show crous
```

## Troubleshooting

:::danger **Build Fails: No C Compiler**

If you see: `error: Microsoft Visual C++ 14.0 or greater is required`

**Solution depends on your platform** - see Platform-Specific Setup section above.
:::

### Common Issues

<Tabs groupId="issue">
<TabItem value="compiler" label="No C Compiler">

**Linux:**
```bash
# Ubuntu/Debian
sudo apt-get install build-essential python3-dev

# Fedora/RHEL
sudo dnf install gcc python3-devel

# Arch
sudo pacman -S gcc python
```

**macOS:**
```bash
xcode-select --install
```

**Windows:**
- Download Visual Studio Community
- Install "Desktop development with C++"

</TabItem>
<TabItem value="python-h" label="Python.h Not Found">

**Error:** `fatal error: Python.h: No such file or directory`

**Solution:**
```bash
# Ubuntu/Debian
sudo apt-get install python3-dev

# Fedora/RHEL
sudo dnf install python3-devel

# macOS
brew install python3
```

</TabItem>
<TabItem value="setuptools" label="setuptools Error">

**Error:** `No module named setuptools`

**Solution:**
```bash
pip install --upgrade pip setuptools wheel
pip install crous
```

</TabItem>
<TabItem value="import" label="Import Error">

**Error:** `ModuleNotFoundError: No module named crous`

**Solution:**
```bash
# Verify Python path
python -c "import sys; print(sys.executable)"

# Reinstall
pip uninstall crous
pip install crous
```

</TabItem>
<TabItem value="naming" label="Naming Conflict">

**Error:** `module 'crous' has no attribute 'dumps'`

**Solution:**

You might have a local file named `crous.py` that conflicts:

```bash
# Find the conflict
python -c "import crous; print(crous.__file__)"

# If it points to your local directory, rename your file
mv crous.py crous_local.py

# Then try importing again
python -c "import crous; print(crous.__version__)"
```

</TabItem>
</Tabs>

## Upgrade & Uninstall

### Upgrade to Latest Version

```bash
# Check current version
pip show crous

# Upgrade
pip install --upgrade crous

# Upgrade to specific version
pip install crous==2.1.0
```

### Uninstall

```bash
pip uninstall crous

# Remove all Crous-related files
pip uninstall crous -y
```

## Development Installation

To contribute to Crous development:

```bash
# Clone repository
git clone https://github.com/crous/crous.git
cd crous

# Create virtual environment
python3 -m venv env
source env/bin/activate

# Install in editable mode with dev dependencies
pip install -e ".[dev]"

# Install test dependencies
pip install pytest pytest-cov

# Run tests
pytest tests/ -v

# Build documentation
pip install sphinx
make docs
```

## Next Steps

🎉 **Installation complete!** Now:

1. 📚 **Read the [User Guide](user-guide.md)** for tutorials and examples
2. 🔌 **Check the [API Reference](../api/reference.md)** for complete documentation
3. 🛠️ **Explore [Custom Serializers](custom-serializers.md)** to extend Crous
4. ⚙️ **Learn the [Architecture](../internals/architecture.md)** for implementation details

## Getting Help

:::info **Need Help?**
- 🐛 [Report bugs on GitHub Issues](https://github.com/crous-project/crous/issues)
- 💬 [Ask questions on GitHub Discussions](https://github.com/crous-project/crous/discussions)
- 📖 [Check the full documentation](/docs)
:::

