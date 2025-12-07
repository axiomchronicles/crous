# Release Checklist

Complete this checklist before releasing a new version of Crous.

## Pre-Release

- [ ] All tests pass locally
  ```bash
  pytest --cov=crous
  ```

- [ ] Code quality checks pass
  ```bash
  black --check crous tests
  isort --check crous tests
  flake8 crous tests
  mypy crous
  ```

- [ ] Documentation is up-to-date
  - [ ] README.md reviewed
  - [ ] API docs reviewed
  - [ ] Examples tested
  - [ ] Docstrings updated

- [ ] CHANGELOG.md updated
  - [ ] Version number correct
  - [ ] All changes documented
  - [ ] Upgrade guide included (if major change)

- [ ] Version numbers updated
  - [ ] `crous/__init__.py` - `__version__`
  - [ ] `crous/setup.py` - `version`
  - [ ] `pyproject.toml` - `version` (if applicable)

- [ ] Git repository clean
  ```bash
  git status
  git diff
  ```

- [ ] All changes committed and pushed
  ```bash
  git add .
  git commit -m "Release v2.x.x"
  git push origin main
  ```

## Build & Test

- [ ] Build wheel package
  ```bash
  python -m build
  ```

- [ ] Verify wheel contents
  ```bash
  twine check dist/*
  ```

- [ ] Test wheel installation
  ```bash
  pip install dist/crous-2.x.x-py3-none-any.whl
  python -c "import crous; print(crous.__version__)"
  ```

- [ ] Test PyPI upload (optional)
  ```bash
  twine upload --repository testpypi dist/*
  pip install --index-url https://test.pypi.org/simple/ crous
  ```

## Release to PyPI

- [ ] Upload to PyPI
  ```bash
  twine upload dist/*
  ```

- [ ] Verify PyPI release
  ```bash
  pip install --upgrade crous
  python -c "import crous; print(crous.__version__)"
  ```

## GitHub Release

- [ ] Create git tag
  ```bash
  git tag v2.x.x
  git push origin v2.x.x
  ```

- [ ] Create GitHub release
  - Go to: https://github.com/crous-project/crous/releases/new
  - Tag version: `v2.x.x`
  - Release title: `Crous v2.x.x`
  - Copy relevant section from CHANGELOG.md
  - Upload wheel if applicable
  - Publish release

## Post-Release

- [ ] Announce release
  - [ ] GitHub Discussions
  - [ ] Reddit (if major release)
  - [ ] Twitter/Social media (if applicable)

- [ ] Monitor for issues
  - [ ] Check GitHub Issues
  - [ ] Check GitHub Discussions
  - [ ] Email support@crous.dev

- [ ] Prepare for next development version
  - [ ] Create development branch (if needed)
  - [ ] Update version to next dev version (optional)

## Rollback Plan (If Needed)

If issues found after release:

1. **Immediate action**
   - [ ] Disable download links
   - [ ] Post notice on GitHub
   - [ ] Notify users

2. **Fix issues**
   - [ ] Create hotfix branch
   - [ ] Fix bugs
   - [ ] Run tests
   - [ ] Update CHANGELOG.md

3. **Release hotfix**
   - [ ] Follow release checklist again
   - [ ] Tag as v2.x.x-hotfix or v2.x.(x+1)
   - [ ] Upload to PyPI

4. **Document incident**
   - [ ] Post-mortem in GitHub Discussions
   - [ ] Update documentation
   - [ ] Add tests to prevent regression

## Release Notes Template

```markdown
# Crous v2.x.x

**Release Date**: YYYY-MM-DD

## What's New

- Feature 1
- Feature 2
- Bug fix 1

## Breaking Changes

None / List any breaking changes

## Migration Guide

Instructions if upgrading from previous version

## Contributors

@username1, @username2

## Download

- [PyPI](https://pypi.org/project/crous/2.x.x/)
- [GitHub](https://github.com/crous-project/crous/releases/tag/v2.x.x)

## Changelog

[Full changelog](https://github.com/crous-project/crous/blob/main/CHANGELOG.md)
```

---

**Important**: Always test releases in a clean environment before marking as production-ready.

Last Updated: 2024-12-07
