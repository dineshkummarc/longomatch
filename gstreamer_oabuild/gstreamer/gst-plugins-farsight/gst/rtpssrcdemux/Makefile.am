plugin_LTLIBRARIES = libgstrtpssrcdemux.la

#noinst_PROGRAMS = test_rtpssrcdemux
#test_rtpssrcdemux_SOURCES = test_rtpssrcdemux.c
#test_rtpssrcdemux_CFLAGS = $(GST_CFLAGS)
#test_rtpssrcdemux_LDADD = $(GST_LIBS)

libgstrtpssrcdemux_la_SOURCES = gstrtpssrcdemux.c

libgstrtpssrcdemux_la_CFLAGS = $(GST_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(ERROR_CFLAGS) -DEXTERN_BUF -DRTP_SUPPORT
libgstrtpssrcdemux_la_LIBADD = $(GST_LIBS_LIBS)
libgstrtpssrcdemux_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS) -lgstrtp-@GST_MAJORMINOR@

#noinst_HEADERS = gstrrtpssrcdemux.h
