ASSEMBLY = LongoMatch
TARGET = exe
LINK = $(REF_DEP_LONGOMATCH)

SOURCES = \
	AssemblyInfo.cs \
	Main.cs

IMAGES = $(top_srcdir)/images/longomatch.png \
         $(top_srcdir)/images/background.png

LOGO = $(top_srcdir)/images/logo_48x48.png

RESOURCES = \
	$(top_srcdir)/images/longomatch.png\
	$(top_srcdir)/images/stock_draw-line-45.png\
	$(top_srcdir)/images/stock_draw-circle-unfilled.png\
	$(top_srcdir)/images/stock_draw-line-ends-with-arrow.png\
	$(top_srcdir)/images/stock_draw-rectangle-unfilled.png\
	$(top_srcdir)/images/stock_draw-freeform-line.png\
	$(top_srcdir)/images/camera-video.png\
	$(top_srcdir)/images/video.png

bin_SCRIPTS = longomatch

DESKTOP_FILE = longomatch.desktop.in

include $(top_srcdir)/build/build.mk

EXTRA_DIST += \
	longomatch.in\
	AssemblyInfo.cs.in

