---------------------------------------------------------------------
INSTALLATION:
---------------------------------------------------------------------

If you have no ./configure file (git clone), perform autoreconf first:
$ autoreconf -fiv

$ ./configure
$ make && make test

---------------------------------------------------------------------
perllib/QClassify INSTALLATION:
---------------------------------------------------------------------

To install perl module of qclasify, do the following:
$ cd perllib/QClassify
$ perl Makefile.PL
$ make && make test
$ sudo make install
