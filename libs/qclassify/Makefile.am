INCLUDES = -I$(top_builddir) -I$(top_builddir)/libs @PCRE_CFLAGS@ @lemmatizer_CFLAGS@

noinst_LTLIBRARIES = libqclassify.la

libqclassify_la_LIBADD = $(top_srcdir)/libs/config/libconfig.la \
                         $(top_srcdir)/libs/utils/libutil.la    \
                         $(top_srcdir)/libs/hashes/libhashes.la \
                         @lemmatizer_LIBS@ \
                         @PCRE_LIBS@

libqclassify_la_SOURCES = \
      basic_phrase_storage.cpp \
      collection_indexer.cpp \
      collection_loader.cpp \
      htmlmark.hpp \
      phrase_indexer.cpp \
      phrase_searcher.cpp \
      ptr_array.hpp \
      qchtmlmark.cpp \
      qcindex_reader.cpp \
      qcindex_writer.cpp \
      qclassify.hpp \
      qclassify_impl.hpp \
      qphrase_splitter.cpp \
      qphrase_splitter_pcre.cpp \
      qphrase_splitter_plain.cpp \
      regexp_aux.cpp \
      regexp_reader.cpp \
      regexp_writer.cpp \
      scatter_storage.cpp \
      searcher_aux.cpp
