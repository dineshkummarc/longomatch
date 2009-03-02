#!/bin/bash

cd $(dirname $0) || exit 1
scripts_dir="$(pwd)"
cd ../Win32/ReleaseWdkCrt || exit 1

find bin     -type f -name "*.dll" | sort  > ../glib

find bin     -type f -name "*.pdb" | sort  > ../glib_dbg

find bin     -type f -name "*.exe" | sort  > ../glib_dev
find bin     -type f -name "*.pl"  | sort >> ../glib_dev
find include -type f               | sort >> ../glib_dev
find lib     -type f               | sort >> ../glib_dev
find vsprops -type f               | sort >> ../glib_dev

find . -type f | grep -v '^\./obj/' | cut -c3- | sort > ../glib_all

sort ../glib ../glib_dbg ../glib_dev ../glib_all | uniq -u > ../glib_missing
nmissing=$(cat ../glib_missing | wc -l)
if [ "$nmissing" != "0" ]; then
  echo "The following files are missing:"
  cat ../glib_missing
  exit 1
else
  rm ../glib_missing
fi

if grep -q 'gstreamer' ../glib_all; then
  echo "ERROR: You need to package GLib before building anything else"
  rm ../glib ../glib_dbg ../glib_dev ../glib_all
  exit 2
fi

"$scripts_dir/helpers/mkdist.sh" glib glib_dbg glib_dev

