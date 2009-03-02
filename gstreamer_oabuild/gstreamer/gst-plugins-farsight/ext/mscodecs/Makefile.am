SUBDIRS = wine

#bin_PROGRAMS = simple-test

#simple_test_SOURCES = simple-test.c mscodec2.S
#simple_test_CFLAGS = $(GST_CFLAGS) -I./wine
#simple_test_LDADD = $(GST_LIBS) $(GST_PLUGINS_BASE_LIBS) -lgstrtp-@GST_MAJORMINOR@ $(srcdir)/wine/libloader.la

plugin_LTLIBRARIES = libgstmscodecs.la

libgstmscodecs_la_SOURCES = \
			    gstmscodecs.c gstmsdec.c gstmsenc.c gstmsrtahealer.c \
			    gstrtprtaudiodepay.c gstrtprtaudiopay.c msaudiohealer.c \
			    msaudiohealer.S msbufferbase.c msbufferbase.S msbuffercustom.c \
			    msbufferrtpextheader.c msbufferrtpextheader.S msbufferstream.c \
			    msbufferstream.S mscodec.c mscodec.S msrtaudiohealer.c \
			    msrtaudiohealer.S msutils.S

libgstmscodecs_la_CFLAGS =  $(GST_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) \
			    $(ERROR_CFLAGS) -I./wine
libgstmscodecs_la_LIBADD = $(GST_LIBS) $(GST_PLUGINS_BASE_LIBS) \
			   -lgstrtp-@GST_MAJORMINOR@ \
			   $(srcdir)/wine/libloader.la
libgstmscodecs_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = \
		 gstmsdec.h gstmsenc.h gstmsrtahealer.h gstrtprtaudiodepay.h \
		 gstrtprtaudiopay.h msaudiohealer.h msbufferbase.h msbuffercustom.h \
		 msbufferrtpextheader.h msbufferstream.h mscodec.h msrtaudiohealer.h \
		 msutils.h

