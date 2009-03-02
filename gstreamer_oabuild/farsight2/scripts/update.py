#!/usr/bin/env python

import sys
import os
import httplib
import socket
import gzip
import tarfile
import StringIO
from tempfile import mkstemp
from urlparse import *

def extractArchive (src, dest, filter):
  print "Extracting %s to %s with filter %s" % (src, dest, filter)

  ret = 0

  tar = tarfile.open(src)
  try:
    for item in tar:
      if (item.name.startswith(filter)):
        host_file = os.path.normpath(dest + item.name.replace(filter, "/"))
        if (item.isdir()):
          if (not os.path.isdir(host_file)):
            os.mkdir(host_file)
          print "dir  : %s" % host_file
        elif (item.isfile()):
          ret += 1
          try:
            file = open(host_file, 'w+b')
            try:
              tmp = tar.extractfile(item)
              file.write(tmp.read())
              tmp.close()
            finally:
              file.close()
          except Exception, ex:
            print "error: %s" % host_file
            continue
          print "file : %s" % host_file
    print 'Done.'
  finally:
    tar.close()

  return ret

def getArchive (url, hash = None):
  (scheme, server, path, query, fragment) = urlsplit(url)

  query_lst = []

  if (query != None and query != ""):
    query_lst.append(query)

  query_lst.append("a=snapshot")

  if (hash != None and hash != ""):
    query_lst.append("h=%s" % hash)

  query = ';'.join(query_lst)
  complete_url = urlunsplit((scheme, server, path, query, fragment))

  r = None
  conn = None
  tmp_file = (None, None)
  try:
    print "HTTP GET: %s" % complete_url
    conn = httplib.HTTPConnection(server)
    headers = {'Accept-encoding' : 'gzip'}
    conn.request("GET", "?".join((path, query)), None, headers)
    r = conn.getresponse()
    print r.status, r.reason
    if (r.status != 200):
      raise 1
    print "Creating the temp file"
    tmp_file = mkstemp(".tar.gz")

    print "Downloading %s bytes to %s" % (r.length, tmp_file[1])

    encoding = r.getheader("Content-encoding", "none")
    print "Encoding: %s" % encoding

    data = r.read ()
    print "Downloaded %s bytes" % len(data)

    if (encoding == "gzip" or encoding == "x-gzip"):
      try:
        gzip_file = gzip.GzipFile(fileobj=StringIO.StringIO(data))
        os.write(tmp_file[0], gzip_file.read())
      finally:
        gzip_file.close()
    else:
      os.write(tmp_file[0], data)

    print "Wrote %s bytes to %s" % (os.stat(tmp_file[1]).st_size, tmp_file[1])

  except SyntaxError, err:
    raise err
  except httplib.InvalidURL:
    print "Invalid URL"
  except (httplib.HTTPException, httplib.ImproperConnectionState, socket.error):
    print "HTTP client error"
  except Exception, ex:
    if (tmp_file != (None, None)):
      os.close(tmp_file[0])
      os.remove(tmp_file[1])
    tmp_file = (None, None)
  finally:
    if (r != None):
      r.close()
    if (conn != None):
      conn.close ()

  return tmp_file


def usage ():
  print "Update Farsight2 from external git repo using gitweb."
  print "    Usage: python update.py <git-branch-url> [git-hash]"
  print "        git-branch-url: url to farsight2 branch"
  print "          i.e. http://git.collabora.co.uk/?p=user/tester/farsight2.git"
  print
  print "        git-hash:       optional revision hash"
  print "          i.e. 7a258c63be5b31625f6729eac080feb6e2c1af05"

def main(argv):
  argc = len(argv)
  if (argc < 2 or argc > 3):
    usage()
    return 1;

  url = argv[1]
  hash = None
  if (argc > 2):
    hash = argv[2]
  
  binDir = os.path.dirname(os.path.abspath(os.path.dirname(sys.argv[0])))
  quote_lst = urlsplit(url)[3].replace(';', '&').split('&')
  quote_map = {}
  for item in quote_lst:
    if (item.find("=")):
      (key, val) = item.split("=")
      quote_map[key] = val

  filter = quote_map['p']

  archive = None
  fd = None
  try:
    (fd, archive) = getArchive(url, hash)
    if (archive == None):
      raise
  except SyntaxError, err:
    raise err
  except:
    print "Could not get head snapshot of %s @ %s" % (filter, url)
    return 1

  try:
    file_count = extractArchive(archive, binDir, filter)
    if (not file_count):
       (file, ext) = os.path.splitext(os.path.basename(filter))
       file_count = extractArchive(archive, binDir, file)

    print "Extracted %d files" % file_count
  except SyntaxError, err:
    raise err
  except:
    print "Could not extract downloaded snapshot %s" % archive
    return 2

  if (fd is not None):
    os.close(fd)
  os.remove(archive)
  return 0

if (__name__ == "__main__"):
  sys.exit(main(sys.argv))
