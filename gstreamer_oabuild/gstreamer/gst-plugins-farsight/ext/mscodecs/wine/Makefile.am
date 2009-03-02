noinst_LTLIBRARIES = libloader.la

CCAS = $(CC)
CCASFLAGS = 

%.o: %.s
	$(CC) -c $< -o $@

libloader_la_SOURCES = \
	ldt_keeper.c \
	pe_image.c \
	module.c \
	ext.c \
	win32.c \
	driver.c \
	pe_resource.c \
	resource.c \
	registry.c \
	elfdll.c \
	afl.c \
	vfl.c \
	stubs.s \
	wrapper.S

noinst_HEADERS = \
	avifmt.h \
	basetsd.h \
	com.h \
	debugtools.h \
	driver.h \
	elfdll.h \
	ext.h \
	heap.h \
	ldt.h \
	ldt_keeper.h \
	loader.h \
	mmreg.h \
	module.h \
	msacmdrv.h \
	msacm.h \
	ntdef.h \
	pe_image.h \
	poppack.h \
	pshpack1.h \
	pshpack2.h \
	pshpack4.h \
	pshpack8.h \
	registry.h \
	resource.h \
	vfw.h \
	win32.h \
	winbase.h \
	windef.h \
	windows.h \
	wineacm.h \
	winerror.h \
	winestring.h \
	winnt.h \
	winreg.h \
	winuser.h \
	wrapper.h

# gcc-3.0 produces buggy code for acmStreamOpen() with
# "-O3 -fomit-frame-pointer" or "-O2 -fomit-frame-pointer
# -finline-functions -frename-registers" (code is OK with sole -O2),
# the bad code accesses parameters via %ebp without setting up a
# propper %ebp first!
# -fno-omit-frame-pointer works around this gcc-3.0 bug.  gcc-2.95.2 is OK.
# Note: -D_FILE_OFFSET_BITS=32 is required to disable using mmap64(),
# as it's broken in glibc 2.1.2 (bad header) and 2.1.3 (bad code)
INCLUDES = \
	-DHAVE_VSSCANF -DHAVE_SYS_MMAN_H -D__WINE__ \
	-DWIN32_PATH=\"/usr/lib/win32\" \
	-fno-omit-frame-pointer \
	-mno-omit-leaf-frame-pointer
