CSV Loader
==========

This is a fast C-implementation for parsing CSV data to Python.
Only one way parsing, from csv -> python, is possible at this moment but
maybe in some future we will also create an 'export-to-CSV' method.

>Note:
>-----
>This module is not a replacement for the Pyton CSV module and should only
>be used if you want to parse (rather strict) csv strings. We do not
>support reading from file directly or different dialects etc. Go for the
>[csv module](https://docs.python.org/3/library/csv.html) if you need more
>options.

Installation
------------

From PyPI (recommend)

```
pip install csvloader
```

From source code

```
python setup.py install
```

Example
-------

```python
import csvloader
data = csvloader.loads(
'''Name,Age
"vd Heijden, Iris",3''')

print(data)  # [['Name', 'Age'], ['vd Heijden, Iris', 3]]
```
