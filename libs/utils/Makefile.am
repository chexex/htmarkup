INCLUDES = -I$(top_builddir) -I$(top_builddir)/libs
noinst_LTLIBRARIES = libutil.la
libutil_la_SOURCES = defs.hpp hash_array.hpp hashes.hpp memfile.cpp memfile.hpp \
                     memio.hpp ptr_array.hpp stringutils.hpp bits/escape_tbl.hpp \
                     syserror.hpp fileutils.cpp fileutils.hpp \
                     base64.cpp str_escape.cpp stringutils.cpp conv.hpp conv.cpp
libutil_la_LIBADD  = @ICU_LIBS@
