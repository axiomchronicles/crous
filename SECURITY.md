# Security Policy

## Supported Versions

Security updates are provided for the following versions:

| Version | Supported          | Until      |
|---------|------------------|------------|
| 2.0.x   | ✅ Yes            | 2025-12-07 |
| 1.0.x   | ❌ No             | 2024-12-07 |

## Reporting a Vulnerability

**⚠️ IMPORTANT: Do not open public GitHub issues for security vulnerabilities.**

If you discover a security vulnerability in Crous, please report it confidentially:

### Email

Send a detailed report to: `security@crous.dev`

Include:
- Description of the vulnerability
- Affected versions
- Steps to reproduce (if applicable)
- Proof of concept code (if safe)
- Suggested fix (if you have one)

### Response Time

We aim to:
- **Acknowledge** reports within 24 hours
- **Provide initial assessment** within 48 hours
- **Release patch** within 7 days (if critical)
- **Publish advisory** after fix is released

## Security Update Process

1. **Report received** - We acknowledge the vulnerability
2. **Assessment** - We determine severity and scope
3. **Fix development** - We develop and test the fix
4. **Pre-release notification** - We notify key users
5. **Release patch** - We release the security update
6. **Publication** - We publish security advisory

## Security Best Practices

When using Crous:

1. **Keep updated** - Always use the latest version
2. **Validate input** - Validate deserialized data
3. **Size limits** - Set maximum sizes for serialized data
4. **Type restrictions** - Use `object_hook` to restrict types
5. **Error handling** - Properly handle serialization errors

Example secure usage:

```python
import crous

def safe_deserialize(data, max_size=10*1024*1024):
    """Safely deserialize Crous data with validation."""
    
    # Check size
    if len(data) > max_size:
        raise ValueError("Data exceeds maximum size")
    
    # Deserialize with error handling
    try:
        return crous.loads(data)
    except crous.CrousDecodeError as e:
        raise ValueError(f"Invalid data: {e}")

# Validate deserialized objects
def validate_user_data(data):
    """Validate expected structure."""
    required = {'username', 'email'}
    if not isinstance(data, dict) or not required.issubset(data.keys()):
        raise ValueError("Invalid user data structure")
    return data
```

## Security Contact

- **Email**: security@crous.dev
- **GPG Key**: Available upon request
- **Response**: 24-48 hours

## Acknowledgments

We appreciate responsible disclosure of security issues. Researchers who report vulnerabilities responsibly may be acknowledged (with permission).

## Security Resources

- [OWASP Top 10](https://owasp.org/www-project-top-ten/)
- [Python Security Guide](https://python.readthedocs.io/en/latest/library/security_warnings.html)
- [Binary Format Security](https://owasp.org/www-community/attacks/Binary_Serialization)

---

Last Updated: 2024-12-07
