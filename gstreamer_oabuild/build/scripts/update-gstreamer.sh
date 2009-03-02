#!/bin/bash

branches="gstreamer gst-plugins-base gst-plugins-good gst-plugins-bad"

TORTOISEDIR="/cygdrive/c/Program Files/TortoiseCVS"
if [ ! -d "$TORTOISEDIR" ]; then
  echo "You need to modify this script to point to your installation of TortoiseCVS."
  exit 1
fi

export PATH="$TORTOISEDIR:$PATH"
if ! cvs --version &>/dev/null; then
  echo "CVS binary not found. Check your installation."
  exit 1
fi

cd $(dirname $0) || exit 1
scripts_dir="$(pwd)"
cd ../../gstreamer || exit 1
root_dir="$(pwd)"
logfile=$root_dir/update-gstreamer.log
clean=0

function cleanup ()
{
  for branch in $branches; do
    echo "$branch: Cleaning"
    cd "$root_dir/$branch" || exit 1
    unknown_files="$(bzr status -S | awk '{ if ($1 == "?") print "\"" $2 "\"" }' | tr -s '\n' ' ')"
    [ -n "$unknown_files" ] && eval rm -rf $unknown_files
    ignored_files="$(bzr ignored | awk '{ print "\"" $1 "\"" }' | grep -v '\.shelf' | tr -s '\n' ' ')"
    [ -n "$ignored_files" ] && eval rm -rf $ignored_files
    bzr revert --no-backup >/dev/null 2>&1
  done
}

function show_msg ()
{
  echo "$1" | tee -a "$logfile"
}

function try_run_cmd ()
{
  cmd_output=$(mktemp /tmp/update.XXXXXX)
  eval "$* > $cmd_output 2>&1"
  RESULT=$?

  echo "> $*" >> "$logfile"
  cat $cmd_output >> "$logfile"

  if [ $RESULT -ne 0 ]; then
    echo "'$*' failed: $RESULT"
    echo "Output:"
    cat $cmd_output
  fi

  rm -f $cmd_output
}

function run_cmd ()
{
  try_run_cmd "$*"
  if [ $RESULT -ne 0 ]; then
    exit 1
  fi
}

if [ "$1" == "--clean" ]; then
  clean=1
  shift 1
fi

if [ $# -ne 0 ]; then
  branches=$*
fi

if [ $clean -ne 0 ]; then
  cleanup
  exit 0
fi

>$logfile

for branch in $branches; do
  show_msg "$branch: Preparing for update"
  cd "$root_dir/$branch" || exit 1
  if quilt top >/dev/null 2>&1; then
    run_cmd "quilt pop -a"
  fi
done

timestamp=$(date -u +"%Y-%m-%d %H:%M UTC")
show_msg "About to pull in changes from upstream: $timestamp"

for branch in $branches; do
  show_msg "$branch: Pulling in changes from upstream"
  cd "$root_dir/$branch" || exit 1
  run_cmd "cvs --lf update -ARPCd"
  bzr diff ChangeLog | grep '^+' | awk '{ if (NR >= 2) print $0 }' | cut -c2- > $root_dir/$branch-WhatsNew.txt
done

success_branches=""
failure_branches=""

for branch in $branches; do
  show_msg "$branch: Applying quilt patch stack"
  cd "$root_dir/$branch" || exit 1
  while [ 1 ]; do
    quilt unapplied >/dev/null 2>&1
    if [ $? -ne 0 ]; then
      success_branches="$success_branches $branch"
      break
    fi

    export QUILT_PATCH_OPTS="-F 0"
    try_run_cmd "quilt push"
    unset QUILT_PATCH_OPTS

    if [ $RESULT -ne 0 ]; then
      failure_branches="$failure_branches $branch"
      break
    fi

    run_cmd "quilt refresh"
  done
done

show_msg "Updating version constants"
cd "$root_dir"
python "$scripts_dir/helpers/syncversionconstants.py" $branches

if [ -n "$success_branches" ]; then
  for branch in $success_branches; do
    show_msg "$branch: Committing"
    cd "$root_dir/$branch" || exit 1
    run_cmd "bzr add ."
    count=$(eval "bzr status | wc -l")
    if [ $count -gt 0 ]; then
      run_cmd "bzr commit -m \"Merged in changes from upstream as of $timestamp.\""
    fi
  done
fi

if [ -n "$failure_branches" ]; then
  show_msg "The following branches require manual work:$failure_branches"
  exit 1
fi

exit 0

