AC_PREREQ(2.53)
AC_INIT([htmarkup], [0.2], [kisel@corp.mail.ru])
AM_INIT_AUTOMAKE([foreign])

LT_INIT([pic-only])
AC_DISABLE_SHARED
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX

AC_CHECK_FUNCS([localeconv])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([munmap])
AC_CHECK_FUNCS([strcasecmp])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([strspn])
AC_CHECK_FUNCS([strtol])
AC_CHECK_FUNCS([mmap])
AC_CHECK_FUNCS([mlock])
AC_CHECK_FUNCS([pread])
AC_CHECK_FUNCS([readahead])
AC_CHECK_FUNCS([iconv_open])

AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([iconv.h])

AC_C_INLINE
AC_FUNC_MMAP

m4_include([m4.incl/cppunit.m4])
m4_include([m4.incl/expat.m4])
m4_include([m4.incl/ax_check_icu.m4])
m4_include([m4.incl/ax_path_lib_pcre.m4])

AM_WITH_EXPAT
AX_PATH_LIB_PCRE([], AC_MSG_ERROR([Could not find libpcre]))
AX_CHECK_ICU([3.6], [], AC_MSG_ERROR([Could not find libicu/libicu-dev >= 3.6]))
AM_PATH_CPPUNIT([1.9.0], [], AC_MSG_ERROR([Could not find cppunit >= 1.9.0]))

PKG_CHECK_MODULES([lemmatizer], [lemmatizer], [], \
                  AC_MSG_ERROR([Lemmatizer can't be found by pkg-config. Have you installed gogo_lemmatizer?]))

# for pkg-config <= 0.23
AC_SUBST([lematizer_CFLAGS])
AC_SUBST([lemmatizer_LIBS])

AC_CONFIG_FILES([Makefile \
                 \
                 libs/config/Makefile \
                 libs/hashes/Makefile \
                 libs/utils/Makefile \
                 libs/qclassify/Makefile \
                 \
                 qclassify/Makefile \
                 \
                 tests/unit/Makefile \
                 tests/unit/runner/Makefile \
                 tests/unit/utils/Makefile \
                 tests/unit/config/Makefile \
                 tests/unit/qclassify/Makefile \
                 \
                 perllib/QClassify/Makefile.PL \
               ])
AC_OUTPUT
