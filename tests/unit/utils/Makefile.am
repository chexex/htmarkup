INCLUDES = -I$(top_builddir) -I$(top_builddir)/libs @lemmatizer_CFLAGS@
LDADD = @lemmatizer_LIBS@ $(top_builddir)/libs/utils/libutil.la ../runner/libcppu_runner.la 

noinst_PROGRAMS = utils_unit_test
utils_unit_test_SOURCES = utils_test.cpp

test:
	./utils_unit_test
