from distutils.core import setup, Extension

c_ext = Extension("pcf8523", ["_pcf8523.c", "pcf8523.c"], libraries=['rt'])

setup(
    ext_modules=[c_ext],
)
