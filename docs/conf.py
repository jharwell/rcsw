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
import datetime

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#

sys.path.insert(0, os.path.abspath('..'))


# -- Project information -----------------------------------------------------
# sphinx uses python by default, so set to cpp
primary_domain = 'cpp'
highlight_language = 'cpp'

today = datetime.date.today()
project = 'RCSW'
copyright = f'{today.year}, John Harwell'
author = 'John Harwell'


version_major = subprocess.run(("grep "
                                "PROJECT_VERSION_MAJOR "
                                "../cmake/project-local.cmake |"
                                "grep -Eo [0-9]+"),
                               shell=True,
                               check=True,
                               stdout=subprocess.PIPE).stdout.decode().strip('\n')
version_minor = subprocess.run(("grep "
                                "PROJECT_VERSION_MINOR "
                                "../cmake/project-local.cmake |"
                                "grep -Eo [0-9]+"),
                               shell=True,
                               check=True,
                               stdout=subprocess.PIPE).stdout.decode().strip('\n')
version_patch = subprocess.run(("grep "
                                "PROJECT_VERSION_PATCH "
                                "../cmake/project-local.cmake |"
                                "grep -Eo [0-9]+"),
                               shell=True,
                               check=True,
                               stdout=subprocess.PIPE).stdout.decode().strip('\n')

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = f'{version_major}.{version_minor}.{version_patch}'

# The full version, including alpha/beta/rc tags.
git_branch = subprocess.run(['git', 'rev-parse', '--abbrev-ref', 'HEAD'],
                            stdout=subprocess.PIPE,
                            check=True).stdout.decode().strip('\n')
if git_branch == 'devel':
    version = f'{version}.beta'

release = version

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
              'exhale',
              'sphinx_last_updated_by_git']

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

# Custom sidebar templates, must be a dictionary that maps document names
# to template names.
#
# This is required for the alabaster theme
# refs: http://alabaster.readthedocs.io/en/latest/installation.html#sidebars
html_sidebars = {
    '**': [
        'relations.html',  # needs 'show_related': True theme option to display
        'searchbox.html',
    ]
}

# Example configuration for intersphinx: refer to the Python standard library.
intersphinx_mapping = {'libra': ('https://libra2.readthedocs.io/en/master/',
                                 None)}
