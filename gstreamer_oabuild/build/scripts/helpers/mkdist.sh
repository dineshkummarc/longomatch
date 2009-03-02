#!/bin/bash

if [ ! -d "../dist" ]; then
  mkdir ../dist
fi
cd ../dist
for d in $*; do
  if [ "$d" == "/" ]; then
    echo "Uh oh, don't think so..."
    exit 1
  fi
  rm -rf $d
  for f in $(cat ../$d); do
    prefixes=$(dirname $f)
    if [ ! -d "$d/$prefixes" ]; then
      mkdir -p "$d/$prefixes"
    fi
    cp -a "../ReleaseWdkCrt/$f" "$d/$f"
  done
done

