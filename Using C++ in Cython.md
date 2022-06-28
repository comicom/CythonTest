# Using C++ in Cython

*3.0.0a10 version*

## 구성 요소
* pyx file
* setup.py
* h/cpp files

## How to Implement Cython (Basic)
### Cython example
#### Function
``` cython
# distutils: language = c++
# distutils: sources = Rectangle.cpp

cdef extern from "Rectangle.h" namespace "shapes":
    bool SetTrue()
    pass
```

#### Class
``` cython
# distutils: language = c++
# distutils: sources = Rectangle.cpp

cdef extern from "Rectangle.h" namespace "shapes":
    cdef cppclass Rectangle:
        Rectangle()
        void SetLength()
        pass
```

### Setup.py example
``` python
from distutils.core import setup, Extension
from Cython.Build import cythonize

ext = Extension("Rect",
                sources=["Rect.pyx"],
                language="c++")

setup(name="Rect",
      ext_modules=cythonize(ext, compiler_directives={'language_level': "3str"}))
```

### build
```shell
python3 setup.py build_ext --inplace
```

### Test
```shell
python3
import Rect
```
