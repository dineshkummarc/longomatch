#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (C) 2008  Ole André Vadla Ravnås <oleavr@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#

import urllib
import os
import os.path
import sys
import pefile

class MSCodecsHelper:
    def fetch_dll(self):
        cache_dir = 'cache'
        exe_url = 'http://www.download.windowsupdate.com/msdownload/update/v3-19990518/cabpool/mu_wlmessenger_20b5a60d1230496a7933f3962a5886d1f41dfc06.exe'
        exe_filename = os.path.join(cache_dir, 'mu_wlmessenger.exe')
        msi_filename = os.path.join(cache_dir, 'MSNMSGS.MSI')
        cab_filename = os.path.join(cache_dir, 'MSNMSGS.cab')

        if not os.path.exists(cache_dir):
            os.mkdir(cache_dir)

        if not os.path.exists(exe_filename):
            print 'Downloading %s' % exe_url
            self._last_feedback = 0
            urllib.urlretrieve(exe_url, exe_filename, self._progress_cb)

        if not os.path.exists(msi_filename):
            print '%s: Extracting MSI' % exe_filename
            data = self._extract_msi_from_image(exe_filename)
            if data is None:
                print 'Failed to extract MSI from image'
                return False

            of = open(msi_filename, 'wb')
            of.write(data)
            of.close()

        if not os.path.exists(cab_filename):
            print '%s: Extracting CAB' % msi_filename
            ret = os.system('./_build_/default/msi-extractor %s %s' % \
                (msi_filename, cab_filename))
            if ret != 0:
                sys.exit(1)

        print '%s: Extracting DLL' % cab_filename
        ret = os.system('cabextract -q -F RTMPLTFMDLL %s' % cab_filename)
        if ret != 0:
            return False

        # Hack alert
        os.system('chmod 644 RTMPLTFMDLL')
        ret = os.system('mv -f RTMPLTFMDLL RTMPLTFM.dll')

        return (ret == 0)

    def _progress_cb(self, blocks, block_size, total_size):
        mbytes_completed = (blocks * block_size) / 1024.0 / 1024.0
        mbytes_total = total_size / 1024.0 / 1024.0
        if mbytes_completed - self._last_feedback >= 1.0:
            self._last_feedback = mbytes_completed
            print 'Downloaded %.2f MB of %.2f MB' % (mbytes_completed, mbytes_total)

    def _extract_msi_from_image(self, filename):
        pe = pefile.PE(name=filename)
        for entry in pe.DIRECTORY_ENTRY_RESOURCE.entries:
            if str(entry.name) == 'BOOTSTRAPPAYLOAD':
                for subentry in entry.directory.entries:
                    if str(subentry.name) == 'MSNMSGS.MSI':
                        for ssentry in subentry.directory.entries:
                            s = ssentry.data.struct
                            return pe.get_data(s.OffsetToData, s.Size)
        return None


if __name__ == '__main__':
    helper = MSCodecsHelper()
    if helper.fetch_dll():
        print
        print "All good. Copy RTMPLTFM.dll to /usr/local/lib/win32 and enjoy."
        sys.exit(0)
    else:
        sys.exit(1)

