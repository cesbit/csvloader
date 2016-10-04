from distutils.core import setup, Extension

module = Extension(
    'csvloader',
    define_macros = [],
    include_dirs = ['.'],
    libraries = [],
    sources = ['./csvloader/csvloader.c']
)

VERSION = '0.0.1'

setup(
    name='csvloader',
    packages=['csvloader'],
    version=VERSION,
    description='Fast C-implementation for reading CSV data into Python',
    author='Jeroen van der Heijden',
    author_email='jeroen@transceptor.technology',
    url='https://github.com/transceptor-technology/csvloader',
    ext_modules = [module],
    download_url=
        'https://github.com/transceptor-technology/'
        'csvloader/tarball/{}'.format(VERSION),
    keywords=['deserializer', 'csv'],
    classifiers=[
        'Development Status :: 4 - Beta',
        'Environment :: Other Environment',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 3',
        'Topic :: Software Development'
    ],
)
