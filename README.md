# CythonTest

Cython 은 CPython 확장 모듈을 생성하고 이를 이용하여 외부 함수 인터페이스와 실행 속도 향상과 외부 라이브러리의 연동을 보다 향상 시킬 수 있도록 고안된 컴파일 언어이다
Cython 은 pyx 확장명을 사용하고, 컴파일 과정을 통하여 파이썬에서 import 형태로 사용될 수 있다.
컴파일은 다음 setup.py 를 이용하여 컴파일할 수 있다

## install cython

``` shell
pip install cython
```

## create pyx file 

``` python
# test_cython.pyx

def ret_list(n):
    return [i for i in range(n)]
```

## create setup.py

``` python
# setup.py# -*- coding: utf-8 -*-
from distutils.core import setup
from Cython.Build import cythonize
 
setup(ext_modules=cythonize("test_cython.pyx")) # 어떤 파일을 변환할지 지정
```

## run cython

``` shell
python3 setup.py build_ext --inplace
```

## compare

``` python
import timeit
 
print("test_cython", timeit.timeit('sum(test_cython.ret_list(100000))', "import test_cython", number=1000))
 
def ret_list(n):
    return [i for i in range(n)]
 
print("test default", timeit.timeit('sum(ret_list(100000))', "from __main__ import ret_list", number=1000))
```
