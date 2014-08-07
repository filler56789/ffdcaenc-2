ACLOCAL_AMFLAGS = -I m4

AUTOMAKE_OPTIONS=foreign
EXTRA_DIST = ffdcaenc.sym ffdca.conf

bin_PROGRAMS = ffdcaenc
noinst_PROGRAMS = gentables
lib_LTLIBRARIES = libffdcaenc.la

include_HEADERS = ffdcaenc.h
libffdcaenc_la_SOURCES = ffdcaenc.c \
    ffdcaenc_private.h int_data.h math_tables.h softfloat.h
nodist_libffdcaenc_la_SOURCES = math_tables.c
libffdcaenc_la_LDFLAGS = -version-info 0:0:0 \
    -export-symbols $(top_srcdir)/ffdcaenc.sym

#if HAVE_ALSA
#plugin_LTLIBRARIES = libasound_module_pcm_dca.la
#plugindir = $(libdir)/alsa-lib
#
#libasound_module_pcm_dca_la_SOURCES = alsaplugin.c ffdcaenc.h
#libasound_module_pcm_dca_la_LIBADD = libffdcaenc.la @ALSA_LIBS@
#libasound_module_pcm_dca_la_LDFLAGS = -no-undefined -avoid-version
#
#alsaconfdir = $(datadir)/alsa/pcm
#alsaconf_DATA = ffdca.conf
#endif

ffdcaenc_SOURCES = main.c wavfile.c xgetopt.c ffdcaenc.h wavfile.h xgetopt.h
ffdcaenc_LDADD = libffdcaenc.la

gentables_SOURCES = gentables.c \
    int_data.h float_data.h softfloat.h

gentables_LDADD = @LIBM@

CLEANFILES = math_tables.c
math_tables.c: Makefile gentables$(EXEEXT)
	./gentables$(EXEEXT) >$@
