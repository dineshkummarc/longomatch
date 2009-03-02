import sys
import subprocess
import os
import os.path
import tempfile

if len (sys.argv) not in (7, 8):
    print >> sys.stderr, 'usage:'
    print >> sys.stderr, '  %s <glib-mkenums-path> <input-dir> <input-files> <output-dir> <output-filename-base> <define> <namespace>' % sys.argv[0]
    print >> sys.stderr, 'or with output templates:'
    print >> sys.stderr, '  %s <glib-mkenums-path> <input-dir> <input-files> <output-dir> <output-filename-base> <output-template-files>' % sys.argv[0]
    sys.exit (1)

def run_redirected (cmd, output_file):
    process = subprocess.Popen (cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output = process.stdout.read ()
    ret = process.wait ()
    if ret != 0:
        raise RuntimeError ("Error running '%s', exit code %d: '%s'" % (cmd, ret, output))
    f = open (output_file, "wb")
    f.write (output)
    f.close ()
    return ret

def _mkdir (newdir):
    """ http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/82465

        Works the way a good mkdir should :)
         - already exists, silently complete
         - regular file in the way, raise an exception
         - parent directory(ies) does not exist, make them as well
    """
    if os.path.isdir (newdir):
        pass
    elif os.path.isfile (newdir):
        raise OSError ("a file with the same name as the desired " \
                       "dir, '%s', already exists." % newdir)
    else:
        head, tail = os.path.split (newdir)
        if head and not os.path.isdir (head):
            _mkdir (head)
        if tail:
            os.mkdir (newdir)

opts = {}
opts['mkenums_path'] = sys.argv[1]
opts['inputdir'] = sys.argv[2]
if sys.argv[3] == '-':
    files = os.environ['INPUTFILES']
else:
    files = sys.argv[3]
opts['inputfiles'] = [os.path.basename (f) for f in files.split (';')]
opts['inputfiles_str'] = ';'.join (opts['inputfiles'])
opts['inputfiles_mapped'] = '\\n'.join (map (lambda name: '#include \\"%s\\"' % name, opts['inputfiles']))
opts['output_dir'] = sys.argv[4]
opts['output_filename_base'] = sys.argv[5]

os.chdir (opts['inputdir'])

if len (sys.argv) == 8:
    opts['enum_define'] = sys.argv[6]
    opts['namespace_uc'] = sys.argv[7].upper ()

    header_cmd = """%(mkenums_path)s \
    --fhead "#ifndef __%(enum_define)s_ENUM_TYPES_H__\\n#define __%(enum_define)s_ENUM_TYPES_H__\\n\\n#include <glib-object.h>\\n\\nG_BEGIN_DECLS\\n" \
    --fprod "\\n/* enumerations from \"@filename@\" */\\n" \
    --vhead "GType @enum_name@_get_type (void);\\n#define %(namespace_uc)s_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\\n" \
    --ftail "G_END_DECLS\\n\\n#endif /* __%(enum_define)s_ENUM_TYPES_H__ */" \
    --flist-env-var INPUTFILES""" % opts

    body_cmd = """%(mkenums_path)s \
    --fhead "#include <glib-object.h>\\n%(inputfiles_mapped)s" \
    --fprod "\\n/* enumerations from \"@filename@\" */" \
    --vhead "GType\\n@enum_name@_get_type (void)\\n{\\n  static GType etype = 0;\\n  if (etype == 0) {\\n    static const G@Type@Value values[] = {" \
    --vprod "      { @VALUENAME@, \\"@VALUENAME@\\", \\"@valuenick@\\" }," \
    --vtail "      { 0, NULL, NULL }\\n    };\\n    etype = g_@type@_register_static (\\"@EnumName@\\", values);\\n  }\\n  return etype;\\n}\\n" \
    --flist-env-var INPUTFILES""" % opts
else:
    templates = sys.argv[6].split (';')
    assert len (templates) == 2

    for template in templates:
        filename = os.path.basename (template)
        if '.h' in filename:
            opts['header_template'] = template
        elif '.c' in filename:
            opts['body_template'] = template

    assert 'header_template' in opts
    assert 'body_template' in opts

    header_cmd = '%(mkenums_path)s --template "%(header_template)s" --flist-env-var INPUTFILES' % opts
    body_cmd = '%(mkenums_path)s --template "%(body_template)s" --flist-env-var INPUTFILES' % opts


_mkdir (opts['output_dir'])

os.environ['INPUTFILES'] = opts['inputfiles_str']

handle1 = None
handle2 = None
tmp_h_path = None
tmp_c_path = None

output_h_path = os.path.join (opts['output_dir'], '%(output_filename_base)s.h' % opts)
output_c_path = os.path.join (opts['output_dir'], '%(output_filename_base)s.c' % opts)

try:
    try:
        handle1, tmp_h_path = tempfile.mkstemp ()
        handle2, tmp_c_path = tempfile.mkstemp ()
    finally:
        for handle in (handle1, handle2):
            try:
                os.close (handle)
            except:
                pass

    run_redirected (header_cmd, tmp_h_path)
    run_redirected (body_cmd, tmp_c_path)

    for file in (output_h_path, output_c_path):
        if os.path.exists (file):
            os.unlink (file)

    try:
        os.rename (tmp_h_path, output_h_path)
        os.rename (tmp_c_path, output_c_path)
    except Exception, e:
        for file in (output_h_path, output_c_path):
            if os.path.exists (file):
                try:
                    os.unlink (file)
                except:
                    pass
        raise e
finally:
    for file in (tmp_h_path, tmp_c_path):
        try:
            os.unlink (file)
        except:
            pass

