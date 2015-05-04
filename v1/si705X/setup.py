from distutils.core import setup, Extension

c_ext = Extension("si705X", ["_si705X.c", "si705X.c"], libraries=['rt'])

setup(
    ext_modules=[c_ext],
)
