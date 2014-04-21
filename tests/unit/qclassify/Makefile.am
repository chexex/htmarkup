INCLUDES = -I$(top_builddir) -I$(top_builddir)/libs @lemmatizer_CFLAGS@
LDADD = @lemmatizer_LIBS@ $(top_builddir)/libs/qclassify/libqclassify.la ../runner/libcppu_runner.la

noinst_PROGRAMS = qclassify_unit_test
qclassify_unit_test_SOURCES = qclassify_test.cpp qchtml_test.cpp

test:
	./qclassify_unit_test
