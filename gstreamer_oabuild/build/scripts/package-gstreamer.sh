#!/bin/bash

cd $(dirname $0) || exit 1
scripts_dir="$(pwd)"
cd ../Win32/ReleaseWdkCrt || exit 1

find . -type f | grep -v '^\./obj/' | cut -c3- | sort > ../gstreamer_all
sort ../glib_all ../gstreamer_all | uniq -u | grep -Ev "^lib/gstreamer-0.10/.*\.(lib|exp)$" > ../gstreamer_all_filtered
mv ../gstreamer_all_filtered ../gstreamer_all

grep -E "^bin/.*\.(exe|dll)$" ../gstreamer_all | grep -v "\-test.exe" > ../gstreamer
grep -E "^lib/gstreamer-0.10/.*\.dll$" ../gstreamer_all >> ../gstreamer

grep -E "^.*\.pdb$" ../gstreamer_all > ../gstreamer_dbg

grep "^bin/.*\-test.exe" ../gstreamer_all       > ../gstreamer_dev
grep "^include/" ../gstreamer_all              >> ../gstreamer_dev
grep -E "^lib/.*\.(lib|exp)$" ../gstreamer_all | grep -v "/gstreamer\-0\.10" >> ../gstreamer_dev
grep "^vsprops/" ../gstreamer_all              >> ../gstreamer_dev

sort ../gstreamer ../gstreamer_dbg ../gstreamer_dev ../gstreamer_all | uniq -u > ../gstreamer_missing
nmissing=$(cat ../gstreamer_missing | wc -l)
if [ "$nmissing" != "0" ]; then
  echo "The following files are missing:"
  cat ../gstreamer_missing
  exit 1
else
  rm ../gstreamer_missing
fi

"$scripts_dir/helpers/mkdist.sh" gstreamer gstreamer_dbg gstreamer_dev

