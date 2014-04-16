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

To install python package:

``` bash
$ cd pythonlib/pyQClassify
$ python setup.py build
$ sudo python setup.py install
```

