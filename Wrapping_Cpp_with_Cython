ref. https://azhpushkin.me/posts/cython-cpp-intro

Wrapping C++ with Cython: intro
This is the beginning of a small series showcasing the implementation of a Cython wrapper over a C++ library. Each article will be a step forward towards having a Python module that is fast, convenient, extendable, and so on. Some will probably be dedicated to not so useful but interesting exotic features of Cython or Python C API.

In this introductory chapter, we are going to create the simplest version of our wrapper. This includes:

presenting a C++ project that is going to be wrapped
creating a Python API that is similar to the C++ API of the underlying library
packaging a Python module that can be installed with pip
I consider this article more of a recipe and findings collection than a complete step by step tutorial. In fact, I expect the whole series to go that way.

If you are a total stranger to Cython, I highly recommend reading Cython "Basic Tutorial" first.

Meet the specimen: yaacrl
Yet Another Audio Recognition Library is a small Shazam-like library, which can recognize songs using a small recorded fragment. Knowledge of audio-recognition algorithms is not required at all for this article. However, for those of you who are interested, I welcome you to read Audio Fingerprinting with Python and Numpy. Basically, this is one of the articles I used to implement yaacrl.

Audio recognition consists of two major parts. The first one is decoding an audio track and generating a unique fingerprint of the track. The second part is searching for the best match among a set of fingerprints. Yaacrl features both parts:

namespace yaacrl {
// Helper structs for storing fingerprint internals
struct Peak;
struct Hash;

// Helper structs for opening different files
struct WAVFile {
    WAVFile(std::string path);
    WAVFile(std::string path, std::string name);
}

struct MP3File {
    MP3File(std::string path);
    MP3File(std::string path, std::string name);
}

// Primary classes that do most of the job

class Fingerprint {
public:
    Fingerprint(const WAVFile& file);
    Fingerprint(const MP3File& file);
    std::vector<Peak> peaks;   // peaks of frequencies
    std::vector<Hash> hashes;  // actual fingerprint
};

class Storage {
public:
    Storage(std::string dbpath);  // path to SQLite database
    ~Storage();

    // Store fingerprint for future matching, both rvalue and lvalue
    void store_fingerprint(Fingerprint& fp);
    void store_fingerprint(Fingerprint&& fp);

    // Find matches among stored fingerprints, return match probability
    std::map<std::string, float> get_matches(Fingerprint& fp);
};
} // end of namespace
As of the time of writing, MP3File constructor is not implemented yet. It is used only for demo purposes. Calling one in a real program will throw an exception.

Fingerprint class is used for both original tracks added to the library and samples that need to be recognized. Here is an example:

// stdlib includes omitted
#include "yaacrl.h"

int main() {
    // Fill database with some songs
    yaacrl::WAVFile song_1 = WAVFile("song_1.wav");
    yaacrl::MP3File song_2 = MP3File("song_2.mp3");
  
    yaacrl::Storage storage;
    storage.store_fingerprint(yaacrl::Fingerprint(song_1));
    storage.store_fingerprint(yaacrl::Fingerprint(song_2));

    // Take a test recording and lookup matches
    yaacrl::WAVFile test_sample = WAVFile("recording.wav");
    auto matches = storage.get_matches(yaacrl::Fingerprint(test_sample));

    for (auto& pair: matches) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    // Output will look like this:
    // song_1.wav: 0.7801492786407471
    // song_2.mp3: 0.1068292784690857

    return 0;
}
You can find a source code for yaacrl in this GitHub repository. Just make sure to switch to intro branch, as newer versions may arrive by the time of reading.

Looking at the header file and the example above, we can already foresee how a similar Python API could look. In general, we want it to be as close as possible to the original C++ API — same classes, same methods, dict instead of std::map, and so on.

As yaacrl contains a leading Y letter, a Python wrapper gets a spot-on name pyaacrl — Python Yet Another Audio Recognition Library.

Getting Cython to know the library
Unfortunately, Cython cannot parse a header file itself and cannot understand what methods and classes are available. For Cython to consume yaacrl correctly, we need to "teach" it about the API using cdef extern. It is convenient to put such declarations in *.pxd files. I am no different and defined it all in lib.pxd:

# Cython offers handly STL declarations
from libcpp.map cimport map
from libcpp.string cimport string

cdef extern from "yaacrl/yaacrl.h" namespace "yaacrl":
    # CppMP3File definition is literally the same as the CppWAVFile one 
    cdef cppclass CppWAVFile "yaacrl::WAVFile":
        string name
        CppWAVFile (string path)
        CppWAVFile (string path, string name)

    cdef cppclass CppFingerprint "yaacrl::Fingerprint":
        CppFingerprint(CppWAVFile file)
        CppFingerprint(CppMP3File file)
        string name
    
    cdef cppclass CppStorage "yaacrl::Storage":
        CppStorage(string)

        void store_fingerprint(CppFingerprint f)
        map[string, float] get_matches(CppFingerprint f)
Here we stumble upon one of the first features of Cython that I find extremely useful — aliasing. Aliasing allows us to refer to methods, classes, and objects in code using a custom name rather than sticking with a real one. However, it does not affect the generated C/C++ code. With aliasing, we can use names like Storage or Fingerprint for Python classes without shadowing original C++ classes. Also, with the Cpp prefix, it is now effortless to differentiate between Python and C++ objects.

Another thing to note is the lack of details about referencing. The original store_fingerprint method is overloaded and accepts either lvalue (&) or rvalue (&&). However, code-generation works in the same way for both methods, so there is no need to reflect this overload in Cython declaration. C++ compiler is the one to take care of it during the compilation.

Aside from that, lib.pxd is very simple, just like all the other .pxd files. Now we can proceed to the pyaacrl_cy.pyx, which contains the implementation of pyaacrl.

Implementing a wrapper: pyaacrl
The most common way to wrap a C++ class is to use Extension types. As an extension type a just a C struct, it can have an underlying C++ class as a field and act as a proxy to it.

However, there are some complications. C structs are created as plain memory chunks, but C++ classes are always created with a constructor. So in order to be stack-allocated in a struct, the class has to implement a nullary constructor.

In our case, CppStorage accepts std::string in a constructor and thus cannot be used as a member. Cython detects this and raises an exception if we try it:

cdef class Storage:
    cdef CppStorage this
    ...

# This code fails with a message:
# "C++ class must have a nullary constructor to be stack allocated"
Official documentation proposes a solid way around this by using a pointer that is manually allocated and deallocated. Custom Cython methods __cinit__ and __dealloc__ is what we need:

cdef class Storage:
    cdef CppStorage* thisptr

    def __cinit__(self, filepath):
        self.thisptr = new CppStorage(filepath)

    def __dealloc__(self):
        del self.thisptr
This method is perfectly fine, and it also has the benefit of automatic dereferencing (so thisptr.get_matches will be automatically translated to thisptr->get_matches).

I decided to go a little further and to use a smart pointer:

from cython.operator cimport dereference as deref
from libcpp.memory cimport unique_ptr, make_unique

cdef class Storage:
    cdef unique_ptr[CppStorage] thisptr

    def __cinit__(self, filepath):
        # make_unique can't deduce conversion of <str> to std::string
        # but doing one manually works fine
        self.thisptr = make_unique[CppStorage](<string>dbfile) 
                
        # or use reset(), then autoconversion works
        # self.thisptr.reset(new CppStorage(dbfile))

    def get_matches(self, ...):
        return deref(self.thisptr).get_matches(...)
In this way, we are obliged to dereference a pointer manually when using it because unique_ptr has methods on its own (reset(), get(), etc.) On the other side, it is easier to reason about memory usage with smart pointers. For instance, we don't need to perform manual deallocation now. Or, if some tricky copy semantics arrive, we can also switch to shared_ptr in no time.

The more interesting conversation relates to the C++ overloading. There is no such concept in Python, so we have no way to keep the libraries' API 100% similar. What we should do — is to find a Pythonic way to reflect C++ API as close as possible.

There are two such occasions in pyaacrl:

CppWAVFile and CppMP3File have two constructors with a different amount of arguments,
CppFingerprint constructors have the same arity, but an argument type could vary.
There are a lot of different ways to deal with the first issue. If we are dealing with the methods overloading, we can create several Python methods, where each one corresponds to a certain overload. If dealing with constructors, we can rely on optional arguments and pick a correct constructor depending on arguments passed:

cdef class WAVFile:
    cdef unique_ptr[CppWAVFile] thisptr

    def __cinit__(self, path, name=None):
        if name is None:
            self.thisptr.reset(new CppWAVFile(path))
        else:
            self.thisptr.reset(new CppWAVFile(path, name))
CppFingerprint case is more complicated. In C++, it's often crucial to have a constructor as an initialization method due to RAII and methods like emplace_back or make_unique. In Python, we don't have such restrictions, which means that we can use staticmethods or classmethods for object creation.

With staticmethods, we don't even need to define Python extension types for CppMP3File and CppWAVFile classes. All their usage is encapsulated like this:

cdef class Fingerprint:
    cdef unique_ptr[CppFingerprint] thisptr
    
    @staticmethod
    def from_wav(path, name = None):
        self = Fingerprint()
        if name:
            self.thisptr = make_unique[CppFingerprint](CppWAVFile(path, name))
        else:
            self.thisptr = make_unique[CppFingerprint](CppWAVFile(path))
        
        return self

    @staticmethod
    def from_mp3(path, name = None):
        self = Fingerprint()
        if name:
            self.thisptr = make_unique[CppFingerprint](CppMP3File(path, name))
        else:
            self.thisptr = make_unique[CppFingerprint](CppMP3File(path))
        
        return self

"""
Usage example:
>>> unnamed = Fingerprint.from_wav("/path/to/file")
>>> named   = Fingerprint.from_mp3("/path/to/file", "name")
"""
And again, option arguments are used as a way to reflect C++ API.

I'm sure that there are certain scenarios where there are just too many overloads, and optional arguments (or named arguments) are not enough. In such cases, just define multiple Python methods with slightly different names. In the end, different languages are not always able to be translated in an exact way.

The last thing to add now is to proxy C++ fields via Python @property decorator. It is straightforward, thanks to the autoconversion that Cython offers. Even setters are doable.

# cython: c_string_type=unicode, c_string_encoding=utf8
# Note: this declaration ensures correct encoding during autoconversion

cdef class Fingerprint:
    ...
    # std::string converts to <str> and vice-versa
    @property
    def name(self):
        return deref(self.thisptr).name

    @name.setter
    def name(self, new_name: str):
        deref(self.thisptr).name = new_name


cdef class Storage:
    ...
    def store_fingerprint(self, fp: Fingerprint):
        # Note: CppFingerprint is extracted from `fp` object
        deref(self.thisptr).store_fingerprint(deref(fp.thisptr))

    def get_matches(self, fp: Fingerprint):
        # std::map<std::string, float> converts to dict[str, float] 
        return deref(self.thisptr).get_matches(deref(fp.thisptr))
Worth mentioning, every automatic conversion makes a copy of the data. Why so? Python objects and C\C++ objects live a different life and are managed differently. Copying prevents them from messing with each other.

That's it for the implementation!

Make sure to take a look at the resulting pyaacrl_cy.pyx and the test program, that consumes it.

Also, Cython documentation has a whole page dedicated to the pitfalls of "Using C++ in Cython." I highly recommend reading it for ones interested in the details of autoconversion, exceptions handling, STL, and some other topics I haven't covered.

Packaging a Cython code
As Cython is a code-generation tool, there is no way to write import pyaacrl_cy.pyx. All the Cython programs have to be compiled and packaged as a Python module.

UPD: turns out Cython provides a tool pyximport that can import .pyx files in runtime. However, it has some quirks and limitations (e.g. it can't compile to C++), because of which it is not recommended to be used on the user side. More details at "Compiling with pyximport (Cython docs)."

The simplest way to create a module is to use setuptools along with the Cython build functions:

from setuptools import setup, Extension
from Cython.Build import cythonize

# in fact, cythonize() call can be omitted because
# setuptools.setup calls it automatically if Cython is installed
setup(
    name='pyaacrl',
    packages=['pyaacrl'],
    ext_modules=cythonize(
        Extension(
            'pyaacrl',
            sources=['pyaacrl.pyx'],
            language='c++',
            libraries=['yaacrl']
        )
    )
)
This solution works, and it is convenient in many ways. However, it is full of pitfalls, if you look close enough:

user is responsible for ensuring that the library is installed in the system; pip install is not enough for the package to work
if the library is installed in the system, we need to make sure that Python module and actual shared library have compatible API versions (e.g. system library can be outdated)
as we use C++, we also need to ensure ABI compatibility (you can read more about this in my previous article)
First, let's focus on the first two issues, as they occur more frequently and relate to both C and C++ code.

The easiest way to avoid both of them is to distribute the source code of the encased library along with the Cython code. With the source code in the project, we can compile and link against it in the [setup.py](http://setup.py).

For pyaacrl, I've added an original yaacrl library as a git submodule to the /vendor/yaacrl path. Git submodule is a great choice because it also pins a certain commit, thus resolves versioning issues too.

Let's update the [setup.py](http://setup.py) with a yaacrl compilation step:

import subprocess
from pathlib import Path, PurePath

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext as build_ext

CPPFLAGS = ['-O2', '-std=c++17', ]
FILE_PATH = Path(__file__).parent.resolve()
YAACRL_DIR: PurePath = FILE_PATH / 'vendor' / 'yaacrl'
YAACRL_BUILD_DIR: PurePath = FILE_PATH / 'build' / 'yaacrl'

class pyaacrl_build_ext(build_ext):
    def _build_yaacrl(self):
        # Make sure build directory is ready, create one if not
        YAACRL_BUILD_DIR.mkdir(parents=True, exist_ok=True)

                # Create build commands via CMake
        subprocess.run([
            'cmake', '-S', YAACRL_DIR, '-B', YAACRL_BUILD_DIR
        ], check=True)

        # Compile the project using makefile, created by CMake
        subprocess.run(['make'], cwd=YAACRL_BUILD_DIR, check=True)

    def build_extension(self, ext):            
        self._build_yaacrl()

        # Add objects and headers to the scope of Extension compiler
        ext.extra_objects.extend([str(YAACRL_BUILD_DIR / 'libyaacrl-static.a')])
        self.compiler.add_include_dir(str(YAACRL_DIR / 'include'))
        
        super().build_extension(ext)

setup(
    name='pyaacrl',
    packages=['pyaacrl'],
    cmdclass={'build_ext': pyaacrl_build_ext},
    ext_modules=[Extension(
        'pyaacrl',
         sources=['pyaacrl/pyaacrl.pyx'],
         extra_compile_args=CPPFLAGS,
         language='c++'
    )]
)
Unfortunately, in the real world, there are often more issues to solve when compiling something in setup.py. You can inspect such real-world examples among MagicStack's projects. They use Cython heavily and have a few projects that wrap C libraries: uvloop wraps libuv and httptools wraps http-parser from node-js. I recommend taking a look at both uvloop/setup.py and httptools/setup.py, as they contain a lot of interesting ideas.

One such idea is the option to use a system library, instead of compiling a bundled one. Using system libraries has some benefits. For instance, if there some security of performance patches arrive, Python extension will automatically pick them up. Also, less disk space is used. In fact, bundled-vs-system library is the same debate as shared-vs-static libraries. Every option has its own benefits, and the ultimate decision is to give users a choice.

To make this choice possible, we need to update our pyaacrl_build_ext command:

...

class pyaacrl_build_ext(build_ext):
    user_options = build_ext.user_options + [(
        'use-system-yaacrl',
        None,
        'If set, use system yaacrl library, instead of the bundled one'
    )]

    boolean_options = build_ext.boolean_options + ['use-system-yaacrl']

    def initialize_options(self):
        if getattr(self, '_initialized', False):
            return

        super().initialize_options()
        self.use_system_yaacrl = False
    
    def _build_yaacrl(self): ...   # exactly the same

    def build_extension(self, ext):
        if self.use_system_yaacrl:
            self.compiler.add_library('yaacrl')  # this will use system library
        else:
            self._build_yaacrl()
            ext.extra_objects.extend([str(YAACRL_BUILD_DIR / 'libyaacrl-static.a')])
            self.compiler.add_include_dir(str(YAACRL_DIR / 'include'))
        
        super().build_extension(ext)
Freshly added build_ext option is available in both python setup.py or pip install, even though pip install variant looks a bit ugly.

$ python setup.py build_ext --use-system-pyaacrl
$ pip install /path/to/pyaacrl --global-option="build_ext" \
                               --global-option="--use-system-yaacrl"
By now, the first two issues I addressed at the start of the chapter are resolved. However, the C++ ABI issue is still not gone.

In setup.py, we compile yaacrl library independently of the pyaacrl extension. Theoretically, these two compilation steps may happen to use a different compiler, different flags, or some other quirks that mess an ABI. To ensure compatibility, a single cohesive compilation pipeline is needed. Fortunately, there is a tool that is designed specifically for such needs.

Meet scikit-build, aka skbuild
The official documentation says: "The scikit-build package is fundamentally just glue between the setuptools Python module and CMake." This definition pretty much describes it all.

skbuild offers several CMake modules and functions that perform a compilation of Python extension modules in the same way that setuptools does. Basically, it allows us to define our compilation rules in CMakeLists.txt rather than in the code of setup.py. That gives us a whole power of CMake, which means we can define our targets, dependencies, and configurations in a much more expressive and natural to C\C++ way.

With skbuild, setup.py looks as easy as it gets.

from skbuild import setup

setup(
    name='pyaacrl',
    packages=['pyaacrl'],
)
The real magic happens in the CMakeLists.txt files. Usually, there are two of them — one is in the root directory, and another one is right near the source code.

For pyaacrl, here is the root-level CMakeLists:

# /CMakeLists.txt
cmake_minimum_required(VERSION 3.3.0)
project(pyaacrl)

# Import yaacrl, only compile if neeeded
add_subdirectory(vendor/yaacrl EXCLUDE_FROM_ALL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/yaacrl/include)
  
# import CMakeLists.txt that lays near the source code
# subdirectory will inherit yaacrl targets from current scope
add_subdirectory(pyaacrl/cython_src)
And here is the one located along the Cython code:

# /pyaacrl/cython_src/CMakeLists.txt
find_package(PythonExtensions REQUIRED)
find_package(Cython REQUIRED)

add_cython_target(pyaacrl_cy CXX PY3)  # cythonize as C++ for Python3
add_library(pyaacrl_cy MODULE ${pyaacrl_cy})  # add module as a target
target_link_libraries(pyaacrl_cy yaacrl-statuc)  # link to original library

python_extension_module(pyaacrl_cy)  # add Python-specific arguments to target 
install(TARGETS pyaacrl_cy LIBRARY DESTINATION pyaacrl)  # copy compiled file to site-packages
This works especially great as the original yaacrl library uses CMake itself, so linking and compiling looks extremely simple. Now the whole installation is really a single pipeline, and the same compiler is used among all the targets.

We can add a choice of whether to use a system or a bundled library to CMake as well. This time, we'll use environment variables, as I find them fancier than passing --global-options to pip.

I found a pretty way to implement this using a dynamic target. In top-level CMakeLists.txt we populate yaacrl_dependency accordingly to env variable with either bundled or system library. After that, pyaacrl/cython_src/CMakeLists.txt consumes the target without bothering about actual choice.

if ($ENV{USE_SYSTEM_YAACRL})
    message("Using system yaacrl library")

    # INTERFACE IMPORTED forwards linking of system yaacrl shared object
    add_library(yaacrl_dependency INTERFACE IMPORTED)
    target_link_libraries(yaacrl_dependency INTERFACE yaacrl)
    # NOTE: assume incude headers are installed in system paths
    
else()
    # Otherwise, yaacrl_dependency is just an alias of the bundled target
    add_subdirectory(vendor/yaacrl EXCLUDE_FROM_ALL)
    add_library(yaacrl_dependency ALIAS yaacrl-static)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/yaacrl/include)

endif()

# In pyaacrl/cython_src/CMakeLists.txt, change linking to
# target_link_libraries(pyaacrl_cy yaacrl_dependency)
Now opting in system dependency is as simple as

$ USE_SYSTEM_YAACRL=1 pip install /path/to/pyaacrl
$ # This works with pip install -r reqs.txt as well
scikit-build is an attractive tool for projects with some heavy mixing of Python, C, and C++. It is easy to use, handy and powerful, and I'm very grateful to Anthony Shaw for recommending me to take a look at it.

Unfortunately, no well-known real-world projects are using it right now. I did some digging into Uber's h3-py library, but it has the simplest skbuild setup possible. I also found pyutils/line_profiler using "ghtopdep" repos search CLI tool. line_profiler has more magic in its CMakeLists.txt and setup.py files, so check it out for a more complex example.

pyaacrl will return...
Well, the introduction turned out to be bigger than I anticipated. At least, now we have small yet fully operational wrapper!

You can inspect the final setup in the GitHub pyaacrl repository. Clone it, install it, see it by yourself. Make sure to use intro git branch to play with the code from this article, as I plan to continue some experiments with the project. And don't hesitate to speak to me if something does not work, we'll sort it out.
