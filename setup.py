from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext as _build_ext
import os
import sys
import shutil
import glob
import platform

here = os.path.abspath(os.path.dirname(__file__))


class build_ext(_build_ext):
    """Custom build extension to copy compiled .so files to package directory."""
    
    def run(self):
        super().run()
        build_lib = self.build_lib
        pattern = os.path.join(build_lib, 'crous', 'crous*.so')
        
        for so_file in glob.glob(pattern):
            dest = os.path.join(here, os.path.basename(so_file))
            print(f"Copying {so_file} to {dest}")
            shutil.copy2(so_file, dest)
    
    def copy_extensions_to_source(self):
        pass

long_description = ""
readme_path = os.path.join(here, "README.md")
if os.path.exists(readme_path):
    with open(readme_path, "r", encoding="utf-8") as f:
        long_description = f.read()

crous_extension = Extension(
    'crous.crous',                  
    sources=[
        'crous/pycrous.c',      
        'crous/crous.c',        
    ],
    include_dirs=['crous'],
    extra_compile_args=[
        '-O3',                
        '-Wall',              
        '-Wextra',            
        '-std=c99',           
    ],
    extra_link_args=[],
)

setup(
    name="crous",
    version="1.0.0",
    description="Crous: High-performance binary serialization format for Python",
    long_description=long_description,
    long_description_content_type="text/markdown",
    author="Pawan Kumar",
    author_email="aegis.invincible@gmail.com",
    url="https://github.com/axiomchronicles/crous",
    license="MIT",
    
    packages=find_packages(exclude=["tests", "tests.*", "crous-docx"]),
    
    ext_modules=[crous_extension],
    
    cmdclass={'build_ext': build_ext},
    
    python_requires=">=3.6",
    
    install_requires=[
        # No external runtime dependencies
    ],
    
    extras_require={
        "dev": [
            "pytest>=7.0.0",
            "pytest-cov>=4.0.0",
            "pytest-xdist>=3.0.0",
            "black>=22.0.0",
            "flake8>=4.0.0",
            "mypy>=0.950",
            "isort>=5.10.0",
            "sphinx>=4.5.0",
            "sphinx-rtd-theme>=1.0.0",
            "twine>=3.4.0",
        ],
    },
    
    entry_points={
        # Example: "console_scripts": ["crous-tool = crous.cli:main"],
    },
    
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Natural Language :: English",
        "Operating System :: OS Independent",
        "Programming Language :: C",
        "Programming Language :: Python",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Topic :: Utilities",
    ],
    
    keywords=[
        "serialization",
        "binary",
        "encoding",
        "decoding",
        "compression",
        "format",
    ],
    
    project_urls={
        "Bug Reports": "https://github.com/axiomchronicles/crous/issues",
        # "Documentation": "https://crous.readthedocs.io",
        "Source": "https://github.com/axiomchronicles/crous",
        "Changelog": "https://github.com/axiomchronicles/crous/blob/main/CHANGELOG.md",
    },
    
    include_package_data=True,
    zip_safe=False,
)