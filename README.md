[Lemmatizer library](https://github.com/dkrotx/gogo_lemmatizer) is required for successful build.

### Building
``` bash
$ autoreconf
$ ./configure
$ make && make test
```

As you can see, there is no `install' for these libs/binaries.
They doesn't attented to be installed system-wide.

### Perl module installation

To install perl module of qclassify:

``` bash
$ cd perllib/QClassify
$ perl Makefile.PL
$ make && make test
$ sudo make install
```

Perl module and all it's dependencies will be installed system-wide

### Python module installation

To install python module you need to install patched version of [SWIG](https://github.com/swig/swig).

SWIG build depends of bison, so install it first
 ``` bash
yum install bison
```

Then build SWIG (patch adds support for unicode strings)
``` bash
$ git clone https://github.com/swig/swig.git
$ cd swig && git checkout rel-3.0.2
$ curl https://raw.githubusercontent.com/sataloger/htmarkup/swig/pythonlib/pyQClassify/swig_unicode_string.patch | git apply -
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
```

And finally build module by itself:

``` bash
$ cd pythonlib/pyQClassify
$ python setup.py build
$ sudo python setup.py install
```

