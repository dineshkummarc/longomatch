#!/bin/bash

OUTPUT_DIR=/c/gstreamer
OUTPUT_DIR=/c/gstreamer
OUTPUT_DIR_LIB=$OUTPUT_DIR/lib
OUTPUT_DIR_GST_LIB=$OUTPUT_DIR_LIB/gstreamer-0.10
OUTPUT_DIR_BIN=$OUTPUT_DIR/bin
OUTPUT_DIR_INCLUDE=$OUTPUT_DIR/include
RELEASE_DIR=./build/Win32/Release

mkdir -p $OUTPUT_DIR
mkdir -p $OUTPUT_DIR_LIB
mkdir -p $OUTPUT_DIR_GST_LIB
mkdir -p $OUTPUT_DIR_BIN
mkdir -p $OUTPUT_DIR_LIB/glib-2.0
mkdir -p $OUTPUT_DIR_LIB/glib-2.0/include/
mkdir -p $OUTPUT_DIR_INCLUDE

cp $RELEASE_DIR/lib/gstreamer-0.10/lib*dll $OUTPUT_DIR_GST_LIB
cp $RELEASE_DIR/bin/*.dll $OUTPUT_DIR_BIN
cp $RELEASE_DIR/bin/gst*.exe $OUTPUT_DIR_BIN
cp $RELEASE_DIR/lib/libgst*.lib $OUTPUT_DIR_LIB
cp $RELEASE_DIR/lib/libg*2.0.lib $OUTPUT_DIR_LIB
cp $RELEASE_DIR/lib/glib-2.0/include/glibconfig.h $OUTPUT_DIR_LIB/glib-2.0/include/
cp -R $RELEASE_DIR/include/* $OUTPUT_DIR_INCLUDE