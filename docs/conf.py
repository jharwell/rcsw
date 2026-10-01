# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html
import os
import sys
import pathlib
import subprocess
import datetime
import urllib.request

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#

sys.path.insert(0, os.path.abspath(".."))


# -- Project information -----------------------------------------------------
# sphinx uses python by default, so set to c
primary_domain = "c"
highlight_language = "c"

today = datetime.date.today()
project = "rcsw"
copyright = f"{today.year}, John Harwell"
author = "John Harwell"

opener = urllib.request.build_opener()
opener.addheaders = [('User-Agent', 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36')]
urllib.request.install_opener(opener)

repo_root = pathlib.Path(__file__).parent.parent
version = subprocess.run(
    ["clibra", "version", "--full", "--preset", "debug"],
    capture_output=True,
    text=True,
    cwd=repo_root,
).stdout.strip()

breathe_projects = {"rcsw": str(repo_root / "build/docs/rcsw/xml")}
breathe_default_project = "rcsw"
c_id_attributes = ["BEGIN_C_DECLS", "END_C_DECLS", "__restrict__"]

breathe_domain_by_extension = {"h": "c"}

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    "sphinx.ext.intersphinx",
    "sphinx.ext.todo",
    "sphinx.ext.coverage",
    "sphinx.ext.mathjax",
    "sphinx.ext.ifconfig",
    "sphinx_design",
    "breathe",
    "sphinxcontrib.moderncmakedomain",
    "sphinx_last_updated_by_git",
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = "pydata_sphinx_theme"

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ["_static"]

html_css_files = []
html_theme_options = {
    "navbar_start": ["navbar-logo"],
    "navbar_center": ["navbar-nav"],
    "navbar_end": ["theme-switcher", "navbar-icon-links"],
    "navigation_depth": 3,
    "show_toc_level": 2,
    "header_links_before_dropdown": 8,
    "logo": {
        "image_light": "_static/logo-only-light.png",
        "image_dark": "_static/logo-only-dark.png",
        "text": f"RCSW {version}",
    },
}

# Example configuration for intersphinx: refer to the Python standard library.
intersphinx_mapping = {"libra": ("https://jharwell.github.io/libra", None)}
