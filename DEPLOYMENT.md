# Crous Deployment Guide

This guide covers deploying Crous to various environments and platforms.

## Table of Contents

1. [Pre-Deployment Checklist](#pre-deployment-checklist)
2. [PyPI Release](#pypi-release)
3. [Docker Deployment](#docker-deployment)
4. [Production Deployment](#production-deployment)
5. [Monitoring & Troubleshooting](#monitoring--troubleshooting)

## Pre-Deployment Checklist

Before deploying Crous, ensure:

- [ ] All tests pass: `pytest`
- [ ] Code coverage is acceptable: `pytest --cov=crous`
- [ ] Type checking passes: `mypy crous`
- [ ] Linting passes: `flake8 crous`
- [ ] Code formatting is correct: `black --check crous`
- [ ] Documentation is up-to-date
- [ ] CHANGELOG.md is updated
- [ ] Version numbers are incremented
- [ ] Git repository is clean: `git status`

## PyPI Release

### 1. Prepare the Release

```bash
# Create a release branch
git checkout -b release/v2.1.0

# Update version in setup.py
# Update version in crous/__init__.py
# Update CHANGELOG.md

# Commit changes
git add setup.py crous/__init__.py CHANGELOG.md
git commit -m "Bump version to 2.1.0"
```

### 2. Build Distribution Packages

```bash
# Install build dependencies
pip install build twine wheel

# Build distributions
python -m build

# Verify distributions
twine check dist/*
```

### 3. Test Upload (Optional but Recommended)

```bash
# Create a PyPI test account at test.pypi.org
# Upload to test PyPI
twine upload --repository testpypi dist/*

# Test installation
pip install --index-url https://test.pypi.org/simple/ crous
```

### 4. Release to PyPI

```bash
# Upload to production PyPI
twine upload dist/*

# Verify release
pip install --upgrade crous
python -c "import crous; print(crous.__version__)"
```

### 5. Create Git Tag and Release

```bash
# Create git tag
git tag v2.1.0

# Push branch and tag
git push origin release/v2.1.0
git push origin v2.1.0

# Create GitHub release with changelog
# Go to https://github.com/crous-project/crous/releases/new
```

## Docker Deployment

### 1. Create Dockerfile

```dockerfile
FROM python:3.11-slim

WORKDIR /app

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Copy application
COPY . .

# Install Crous
RUN pip install --no-cache-dir -e .

# Run application
CMD ["python", "-c", "import crous; print('Crous ready')"]
```

### 2. Build Docker Image

```bash
docker build -t crous:2.0.0 .
docker tag crous:2.0.0 crous:latest
```

### 3. Push to Registry

```bash
# Docker Hub
docker tag crous:latest yourname/crous:latest
docker push yourname/crous:latest

# AWS ECR
aws ecr get-login-password --region us-east-1 | docker login --username AWS --password-stdin 123456789.dkr.ecr.us-east-1.amazonaws.com
docker tag crous:latest 123456789.dkr.ecr.us-east-1.amazonaws.com/crous:latest
docker push 123456789.dkr.ecr.us-east-1.amazonaws.com/crous:latest
```

## Production Deployment

### Environment Setup

```bash
# Create virtual environment
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install Crous from PyPI
pip install crous

# Or install with specific version
pip install crous==2.0.0
```

### Performance Tuning

For production environments:

```python
# Use connection pooling if needed
import crous

# Pre-compile encoders for frequently used types
encoder = crous.CrousEncoder()
decoder = crous.CrousDecoder()
```

### Logging and Monitoring

```python
import logging
import crous

# Enable logging
logging.basicConfig(level=logging.INFO)

try:
    data = crous.dumps({'key': 'value'})
except crous.CrousError as e:
    logging.error(f"Serialization error: {e}")
```

### Database Integration

Example with SQLAlchemy:

```python
import crous
from sqlalchemy import TypeDecorator, LargeBinary

class CrousBinary(TypeDecorator):
    impl = LargeBinary
    cache_ok = True

    def process_bind_param(self, value, dialect):
        if value is None:
            return None
        return crous.dumps(value)

    def process_result_value(self, value, dialect):
        if value is None:
            return None
        return crous.loads(value)

# Usage in models
from sqlalchemy.orm import declarative_base

Base = declarative_base()

class MyModel(Base):
    __tablename__ = 'my_table'
    data = Column(CrousBinary)
```

### API Server Integration

Example with Flask:

```python
from flask import Flask, request, jsonify
import crous

app = Flask(__name__)

@app.route('/api/data', methods=['POST'])
def serialize_data():
    try:
        data = request.get_json()
        binary = crous.dumps(data)
        return binary, 200, {'Content-Type': 'application/octet-stream'}
    except crous.CrousError as e:
        return jsonify({'error': str(e)}), 400

@app.route('/api/data', methods=['GET'])
def deserialize_data():
    try:
        binary = request.get_data()
        data = crous.loads(binary)
        return jsonify(data)
    except crous.CrousError as e:
        return jsonify({'error': str(e)}), 400
```

## Monitoring & Troubleshooting

### Health Checks

```python
def health_check():
    """Simple health check for Crous."""
    try:
        test_data = {'status': 'ok'}
        encoded = crous.dumps(test_data)
        decoded = crous.loads(encoded)
        return decoded == test_data
    except Exception as e:
        logging.error(f"Health check failed: {e}")
        return False
```

### Performance Monitoring

```python
import time
import crous

def benchmark_serialization(data, iterations=1000):
    """Benchmark serialization performance."""
    start = time.time()
    for _ in range(iterations):
        crous.dumps(data)
    encode_time = (time.time() - start) / iterations * 1000
    
    binary = crous.dumps(data)
    
    start = time.time()
    for _ in range(iterations):
        crous.loads(binary)
    decode_time = (time.time() - start) / iterations * 1000
    
    print(f"Encode: {encode_time:.2f}ms, Decode: {decode_time:.2f}ms")
    print(f"Binary size: {len(binary)} bytes")
```

### Common Issues

#### Import Error

```python
# If import fails, ensure C extension is built
python setup.py build_ext --inplace
```

#### Version Mismatch

```bash
# Verify installed version
python -c "import crous; print(crous.__version__)"

# Upgrade if needed
pip install --upgrade crous
```

#### Memory Issues

For large datasets, use streaming:

```python
# Instead of loading entire file into memory
with open('large_file.crous', 'rb') as f:
    while True:
        chunk = f.read(8192)
        if not chunk:
            break
        data = crous.loads(chunk)
        # Process data
```

### Rollback Procedure

```bash
# If deployment fails, rollback to previous version
pip install crous==2.0.0  # Previous stable version

# Or uninstall and reinstall
pip uninstall crous -y
pip install crous
```

## Scaling Considerations

### Distributed Systems

For use in distributed systems:

1. **Consistency**: Ensure all nodes use the same Crous version
2. **Serialization**: Use tagged values for type preservation across services
3. **Compression**: Consider compression for network transmission

```python
import gzip
import crous

# Compress serialized data
data = {'large': 'payload'}
serialized = crous.dumps(data)
compressed = gzip.compress(serialized)

# On receiver end
decompressed = gzip.decompress(compressed)
deserialized = crous.loads(decompressed)
```

### Load Balancing

When deploying behind a load balancer:

1. Ensure all servers have identical Crous versions
2. Use sticky sessions if maintaining per-connection state
3. Monitor serialization errors across all instances

## Security Considerations

1. **Input Validation**: Always validate deserialized data
2. **Size Limits**: Set maximum sizes for serialized data
3. **Type Restrictions**: Use `object_hook` to restrict deserialized types
4. **Updates**: Keep Crous updated for security patches

```python
import crous

# Size check
MAX_SIZE = 10 * 1024 * 1024  # 10MB

def safe_loads(data):
    if len(data) > MAX_SIZE:
        raise ValueError("Payload too large")
    return crous.loads(data)
```

---

For more information, see [Documentation](https://crous.readthedocs.io)
