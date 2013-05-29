INCLUDES = -I$(top_builddir) -I$(top_builddir)/libs
LDADD = $(top_builddir)/libs/config/libconfig.la $(top_builddir)/libs/utils/libutil.la ../runner/libcppu_runner.la 

noinst_PROGRAMS = config_unit_test
config_unit_test_SOURCES = config_test.cpp

test:
	./config_unit_test
