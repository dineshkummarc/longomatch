plugin_LTLIBRARIES = libgstrtppayloads.la

libgstrtppayloads_la_SOURCES = \
    gstmsgsmpayload.c \
    gstmsgsmpayload.h \
    gstmsgsmdepayload.c \
    gstmsgsmdepayload.h \
    gstdvpayload.c \
    gstdvpayload.h \
    gstdvdepayload.c \
    gstdvdepayload.h \
    gstrtpcnpayload.c \
    gstrtpcnpayload.h \
    gstrtpcndepayload.c \
    gstrtpcndepayload.h \
    gstrtppayloads.c

if ENABLE_G729
libgstrtppayloads_la_SOURCES += \
    gstrtpg729depay.c \
    gstrtpg729depay.h \
    gstrtpg729pay.c \
    gstrtpg729pay.h
endif

libgstrtppayloads_la_CFLAGS = $(GST_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(ERROR_CFLAGS) -DEXTERN_BUF -DRTP_SUPPORT
libgstrtppayloads_la_LIBADD = $(GST_LIBS_LIBS)
libgstrtppayloads_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS) -lgstrtp-@GST_MAJORMINOR@
