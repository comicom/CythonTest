from distutils.core import setup, Extension
from Cython.Build import cythonize

ext = Extension("CONF",
                sources=["CONF.pyx", "config.cpp"],
                language="c++")

setup(name="CONF",
      ext_modules=cythonize(ext))