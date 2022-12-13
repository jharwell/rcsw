# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html
import os
import sys
import pathlib
import subprocess
import textwrap

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#

sys.path.insert(0, os.path.abspath('..'))


# -- Project information -----------------------------------------------------

project = 'RCSW'
copyright = '2022, John Harwell'
author = 'John Harwell'


version_major = subprocess.run(("grep "
                                "PROJECT_VERSION_MAJOR "
                                "../cmake/project-local.cmake |"
                                "grep -Eo [0-9]+"),
                               shell=True,
                               check=True,
                               stdout=subprocess.PIPE).stdout.decode()
version_minor = subprocess.run(("grep "
                                "PROJECT_VERSION_MINOR "
                                "../cmake/project-local.cmake |"
                                "grep -Eo [0-9]+"),
                               shell=True,
                               check=True,
                               stdout=subprocess.PIPE).stdout.decode()
version_patch = subprocess.run(("grep "
                                "PROJECT_VERSION_PATCH "
                                "../cmake/project-local.cmake |"
                                "grep -Eo [0-9]+"),
                               shell=True,
                               check=True,
                               stdout=subprocess.PIPE).stdout.decode()

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = f'v{version_major}.{version_minor}.{version_patch}'

# The full version, including alpha/beta/rc tags.
git_branch = subprocess.run(['git', 'rev-parse', '--abbrev-ref', 'HEAD'],
                            stdout=subprocess.PIPE,
                            check=True)
if git_branch == 'devel':
    release = f'v{version_major}.{version_minor}.{version_patch}.beta'
else:
    release = version

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ['sphinx.ext.intersphinx',
              'sphinx.ext.todo',
              'sphinx.ext.coverage',
              'sphinx.ext.mathjax',
              'sphinx.ext.ifconfig',
              'breathe',
              'exhale']

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

cpp_id_attributes = ['RCSW_PURE', 'RCSW_CONST',
                     '__restrict__', 'BEGIN_C_DECLS', 'END_C_DECLS']

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

rcsw_root = pathlib.Path(os.path.abspath(".."))
breathe_projects = {
    "RCSW": str(rcsw_root / "build/docs/rcsw/xml")
}
breathe_default_project = "RCSW"
paths = []
for root, subs, files in os.walk(str(rcsw_root / "include/")):
    for f in files:
        paths.append(os.path.abspath(os.path.join(root, f)))

breathe_projects_source = {"RCSW": (str(rcsw_root / "include"), paths)}

exhale_args = {
    # These arguments are required
    "containmentFolder":     "./_api",
    "rootFileName":          "api.rst",
    "rootFileTitle":         "RCSW API",
    "afterTitleDescription": textwrap.dedent('''
       .. note::

          Some functions declared+defined using variadic macros might not be
          included, due to limits in the doxygen+breathe+exhale toolchain.

    '''),

    "doxygenStripFromPath": "..",
    "verboseBuild": False,
    'exhaleExecutesDoxygen': False,
    "createTreeView": True
}
