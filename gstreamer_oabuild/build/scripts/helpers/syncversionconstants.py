#!/usr/bin/env python

import sys

class Version:
    def __init__(self, version_str):
        tokens = version_str.split('.', 3)
        self.major = int(tokens[0])
        self.minor = int(tokens[1])
        self.micro = int(tokens[2])
        self.nano  = int(tokens[3])

    def __str__(self):
        return '%(major)d.%(minor)d.%(micro)d.%(nano)d' % self.__dict__

    def __repr__(self):
        return '<Version major=%d minor=%d micro=%d nano=%d>' % \
            (self.major, self.minor, self.micro, self.nano)

class Module:
    def __init__(self, dir_name, name, vsprops_prefix):
        self.dir_name = dir_name
        self.name = name
        self.vsprops_prefix = vsprops_prefix
        self.version = None

class VSPropsVersionFile:
    def __init__(self, filename):
        self.filename = filename
        self.parts = []
        self.macro_indexes = {}

        # Parse the file in a presumptuous manner. The idea is that we try to
        # stick to the existing indentation and formatting used by MSVS.
        # We simply just store the contents as a list of chunks, so one chunk
        # for each macro value and whatever is between those as chunks between
        # the values. While doing the parsing we also create a mapping from
        # macro names to chunk indices so we can easily change the values.
        f = open(self.filename, 'rb')
        buf = f.read()
        f.close()
        del f

        tokens = buf.split('Value=\"')
        i = 0
        last_macro_name = None
        for t in tokens:
            if i > 0:
                # Get the value and add it, and keep track of its index using
                # the name from the previous iteration
                start = t.find('\"')
                assert start >= 0
                value = t[0:start]
                value_index = len(self.parts)
                self.macro_indexes[last_macro_name] = value_index
                self.parts.append(value)

                # Strip off the value
                t = t[start:]

            if i < len(tokens) - 1:
                # Extract the macro name of the next item
                start = t.rfind('Name=\"')
                assert start >= 0
                start += 6
                end = t.find('\"', start)
                last_macro_name = t[start:end]

                # Add the token and the separator removed by split()
                self.parts.append(t + 'Value=\"')
            else:
                # Just add the token
                self.parts.append(t)

            i += 1

    def save(self):
        f = open(self.filename, 'wb')
        f.write("".join(self.parts))
        f.close()
        del f

    def __getitem__(self, key):
        return self.parts[self.macro_indexes[key]]

    def __setitem__(self, key, item):
        assert key in self.macro_indexes
        self.parts[self.macro_indexes[key]] = item


known_modules = {
    'gstreamer':        Module('gstreamer', 'GStreamer', 'GstCore'),
    'gst-plugins-base': Module('gst-plugins-base', 'GstPluginsBase', 'GstPluginsBase'),
    'gst-plugins-good': Module('gst-plugins-good', 'GstPluginsGood', 'GstPluginsGood'),
    'gst-plugins-bad':  Module('gst-plugins-bad', 'GstPluginsBad', 'GstPluginsBad'),
}

if len(sys.argv) < 2:
    print 'usage: %s MODULE1 MODULE2 ...' % sys.argv[0]
    sys.exit(1)

module_names = sys.argv[1:]
for name in module_names:
    if name not in known_modules:
        print 'unknown module %s' % name
        sys.exit(2)

for name in module_names:
    m = known_modules[name]

    buf = open('%s\\configure.ac' % m.dir_name, 'r').read()
    start = buf.find('\nAC_INIT')
    end = buf.find('\n', start + 1)

    version_str = buf[start:end].split(' ')[-1].rstrip(',')
    m.version = Version(version_str)

    dic = m.__dict__
    filename = '%(dir_name)s\\win32\\oa\\%(vsprops_prefix)sVersion.vsprops' \
        % dic
    vf = VSPropsVersionFile(filename)

    version_macro_name = '%(name)sVersion' % dic
    old_version = vf[version_macro_name]
    new_version = str(m.version)
    if old_version != new_version:
        print '%s: %s => %s' % (name, old_version, new_version)
    else:
        print '%s: %s' % (name, new_version)

    vf['%(name)sMajorVersion' % dic] = str(m.version.major)
    vf['%(name)sMinorVersion' % dic] = str(m.version.minor)
    vf['%(name)sMicroVersion' % dic] = str(m.version.micro)
    vf['%(name)sNanoVersion' % dic] = str(m.version.nano)
    vf[version_macro_name] = new_version
    vf['%(name)sApiVersion' % dic] = \
        '%d.%d' % (m.version.major, m.version.minor)

    vf.save()

sys.exit(0)

