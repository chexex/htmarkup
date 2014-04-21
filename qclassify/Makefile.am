INCLUDES = -I$(top_builddir) -I$(top_builddir)/libs @lemmatizer_CFLAGS@
LDADD = @lemmatizer_LIBS@ $(top_builddir)/libs/qclassify/libqclassify.la

bin_PROGRAMS = cphrase idx_phrases qcmarker
cphrase_SOURCES = cphrase.cpp
idx_phrases_SOURCES = idx_phrases.cpp
qcmarker_SOURCES = qcmarker.cpp

