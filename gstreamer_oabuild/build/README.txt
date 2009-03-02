OABuild
=======
Ole Andre Ravnas <ole.andre.ravnas@tandberg.com>
:Author Initials: OAVR

'OABuild' is a simplistic suite that provides Windows developers with an easy
and relatively fast way to build a selection of cool F/OSS projects in a
familiar environment using only native compilers.


What is included
----------------

OABuild v1 (obsolete/unmaintained)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 - zlib
 - libjpeg
 - libpng
 - libxml2
 - glib (w/.NET bindings)
 - cairo (w/.NET bindings)
 - atk (w/.NET bindings)
 - pango (w/.NET bindings)
 - gtk+ (w/.NET bindings)
 - libglade (w/.NET bindings)
 - glade
 - gstreamer (w/.NET bindings)
 - gst-plugins-base
 - gst-plugins-good
 - gst-plugins-bad
 - gst-ffmpeg
 - D-Bus (w/GLib and .NET bindings)
 - Telepathy (telepathy-glib, telepathy-gabble and .NET bindings)

OABuild v2 (work in progress)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 - glib 2.16.1
 - ffmpeg r11247 (the revision currently suggested by gst-ffmpeg)
 - GStreamer
  * -core: 0.10.20.1 (HEAD snapshot taken 2008-08-03 17:35)
  * -base: 0.10.20.1 (HEAD snapshot taken 2008-08-03 17:35)
  * -good: 0.10.9.1 (HEAD snapshot taken 2008-08-03 17:35)
  * -bad: 0.10.8.1 (HEAD snapshot taken 2008-08-03 17:35)
  * -ffmpeg: 0.10.3.1 (HEAD snapshot taken 2008-03-23 21:12)
  * gst-plugins-farsight: 0.12.7
 - Farsight 2 HEAD (Courtesy of Haakon Sporsheim!)
 - GTK+ (HEAD, r19938)
 - Clutter (Courtesy of Haakon Sporsheim!)
 - Sofia-SIP (Courtesy of Haakon Sporsheim!)
 - libjpeg
 - libmms

This list will most likely grow in the future. Contributions are very welcome.


Pre-requisities
---------------

 * MS Visual Studio 2008 (2005 for v1)
 * MS Windows SDK (latest version, added to MSVS' VC++ directories)
 * MS DirectX SDK (-----------------------""----------------------)
 * MS DirectShow BaseClasses Release_MBCS and Debug_MBCS compiled
   and with the directories added to MSVS' VC++ directories.

  - For example:
   * Include files:
    - C:\Program Files\Microsoft SDKs\Windows\v6.0\Samples\Multimedia\DirectShow\BaseClasses
   * Library files:
    - C:\Program Files\Microsoft SDKs\Windows\v6.0\Samples\Multimedia\DirectShow\BaseClasses\Debug_MBCS
    - C:\Program Files\Microsoft SDKs\Windows\v6.0\Samples\Multimedia\DirectShow\BaseClasses\Release_MBCS

[IMPORTANT]
=====================================================================
Make sure you put the BaseClasses include directory first (at the
top), followed by the DirectX SDK and finally the Windows SDK.
=====================================================================

 * In the system-wide PATH:
   * bzr: http://bazaar-vcs.org/releases/win32/bzr-setup-latest.exe[]
   * [v1 only] cvs: http://march-hare.com/archive/cvsnt-2.5.03.2382.msi[]
   * [v1 only] svn: http://subversion.tigris.org/files/documents/15/38369/svn-1.4.4-setup.exe[]
   * python: http://python.org/ftp/python/2.5.1/python-2.5.1.msi[]
   * perl: http://downloads.activestate.com/ActivePerl/Windows/5.8/ActivePerl-5.8.8.820-MSWin32-x86-274739.msi[]


Getting started
---------------

Start a Command Prompt as 'Administrator' and change to a suitable
location, for example 'c:\'. Then do:

OABuild v2
~~~~~~~~~~
--------------------------------------
md OABuild
cd OABuild
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/build
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/libintl-proxy
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/glib
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/ffmpeg
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/zlib
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/libjpeg
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/libmms
md gstreamer
cd gstreamer
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/gst-builddeps builddeps
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/gstreamer
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/gst-plugins-base
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/gst-plugins-good
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/gst-plugins-bad
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/gst-ffmpeg
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/gst-plugins-farsight
cd ..
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/farsight2
md gtk+
cd gtk+
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/pixman
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/cairo
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/atk
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/pango
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/gtk+
cd ..
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/clutter
bzr branch http://bazaar.launchpad.net/~oleavr/oabuild/sofia-sip
--------------------------------------

Open the appropriate solution files (.sln) in the 'build' subdirectory and
build them one by one. Previously there was only one huge solution, but this
monolithic approach was abandoned for two reasons:

- MSVS becomes awfully slow and eats lots of memory when you have a massive
  amount of projects in one solution.
- Having all sorts of unrelated projects in one solution makes it harder for
  people wanting to hack on, play around with or package only individual
  projects or groups of projects.
  E.g. people hacking on Telepathy might not want to be bothered with 100+
  GStreamer modules and vice versa.

The output ends up in build/windows/{Debug,Release,ReleaseWdkCrt} and will
be similar to the UNIX layout.

Note that binaries built by the Debug and Release configurations depend on
msvcr90[d].dll, whilst those built by ReleaseWdkCrt depend on msvcrt.dll.
The latter does however require the http://www.microsoft.com/whdc/devtools/WDK/default.mspx[Windows Driver Kit].
There's some background info on how this is achieved http://kobyk.wordpress.com/2007/07/20/dynamically-linking-with-msvcrtdll-using-visual-c-2005/[here].

OABuild v1 (obsolete/unmaintained)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
--------------------------------------
bzr branch http://projects.collabora.co.uk/~oleavr/branches/OABuild/
cd OABuild
bootstrap
--------------------------------------

If all went well you should now be able to open oabuild.sln, go
'Build Solution' and wait a few minutes.

The output ends up in the 'Debug' subdirectory for debug builds
and 'Release' for release builds. The output will be similar to
the UNIX layout, header-files in 'include', .lib files for
linking in 'lib', and executables, runtime libraries and .NET
assemblies in 'bin'.
