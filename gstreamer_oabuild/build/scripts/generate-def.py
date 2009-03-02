# -*- coding: utf-8 -*-
#
# Copyright (C) 2008 Ole André Vadla Ravnås <oleavr@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

import sys
import os
import glob
import subprocess
import getopt
import re

def ishexstring(s):
    if len(s) == 0:
        return False
    for c in s:
        c = c.upper()
        if not c.isdigit() and (c < 'A' or c > 'F'):
            return False
    return True

def extract_obj_symbols(path):
    os.chdir(path)

    symbols = []
    for fn in glob.glob('*.obj'):
        output = subprocess.Popen(['dumpbin', '/symbols', fn], stdout=subprocess.PIPE).communicate()[0]
        for line in output.split('\r\n'):
            if not ishexstring(line[0:3]):
                continue

            tokens = line.split()
            if len(tokens) < 7 or tokens[3] != 'notype':
                continue

            if tokens[4] == '()':
                type_index = 5
                is_data = False
            else:
                type_index = 4
                is_data = True

            if tokens[type_index] != 'External':
                continue
            elif tokens[2] == 'UNDEF':
                continue

            name = tokens[type_index + 2]
            if len(tokens) > type_index + 3:
                continue
            elif name.startswith('_'):
                name = name[1:]

            if is_data:
                name += ' DATA'

            symbols.append(name)

    symbols.sort()
    return symbols

def usage():
    print '%s [--mode=MODE] [--add-match=MASK --add-ignore=MASK] <OBJ-DIRECTORY>' % sys.argv[0]

def main(argv):
    try:
        opts, args = getopt.getopt(argv, 'h', ['help', 'mode=', 'add-match=', 'add-ignore='])
    except getopt.GetoptError:
        usage()
        sys.exit(2)

    mode = None
    filters = []

    for opt, arg in opts:
        if opt in ('-h', '--help'):
            usage()
            sys.exit()
        elif opt == '--mode':
            mode = arg
        elif opt == '--add-match':
            filters.append((True, re.compile(arg)))
        elif opt == '--add-ignore':
            filters.append((False, re.compile(arg)))

    if mode is not None:
        if mode == 'gstreamer':
            filters.extend([
                (False, re.compile("^_gst_parse_yy")),
                (False, re.compile("^_gst_[a-z]*_init")),
                (False, re.compile("^_gst_parse_launch")),
                (False, re.compile("^__gst_element_details_")),
                (False, re.compile("^__gst_element_factory_add_")),
                (False, re.compile("^gst_interfaces_marshal")),
                (True,  re.compile("^[_]*(gst_|Gst|GST_).*"))
            ])
        else:
            usage()
            sys.exit(1)
    elif len(args) != 1:
        usage()
        sys.exit(1)

    def should_include_func(x):
        for include, filter in filters:
            result = filter.match(x)
            if include and result is None:
                return False
            elif not include and result is not None:
                return False
        return True

    symbols = extract_obj_symbols(args[0])
    symbols = filter(should_include_func, symbols)

    print 'EXPORTS'
    print '\t%s' % '\n\t'.join(symbols)

if __name__ == '__main__':
    main(sys.argv[1:])

