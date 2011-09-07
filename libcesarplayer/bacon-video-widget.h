/*
 * Copyright (C) 2001,2002,2003,2004,2005 Bastien Nocera <hadess@hadess.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Totem project hereby grant permission for non-gpl compatible GStreamer
 * plugins to be used and distributed together with GStreamer and Totem. This
 * permission are above and beyond the permissions granted by the GPL license
 * Totem is covered by.
 *
 * Monday 7th February 2005: Christian Schaller: Add excemption clause.
 * See license_change file for details.
 *
 */

#ifndef HAVE_BACON_VIDEO_WIDGET_H
#define HAVE_BACON_VIDEO_WIDGET_H

#ifdef WIN32
#define EXPORT __declspec (dllexport)
#else
#define EXPORT
#endif

#include <clutter-gtk/clutter-gtk.h>
#include <gst/gst.h>

G_BEGIN_DECLS
#define BACON_TYPE_VIDEO_WIDGET		     (bacon_video_widget_get_type ())
#define BACON_VIDEO_WIDGET(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), bacon_video_widget_get_type (), BaconVideoWidget))
#define BACON_VIDEO_WIDGET_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), bacon_video_widget_get_type (), BaconVideoWidgetClass))
#define BACON_IS_VIDEO_WIDGET(obj)           (G_TYPE_CHECK_INSTANCE_TYPE (obj, bacon_video_widget_get_type ()))
#define BACON_IS_VIDEO_WIDGET_CLASS(klass)   (G_CHECK_INSTANCE_GET_CLASS ((klass), bacon_video_widget_get_type ()))
#define BVW_ERROR bacon_video_widget_error_quark ()
typedef struct BaconVideoWidgetPrivate BaconVideoWidgetPrivate;


typedef struct
{
  GtkClutterEmbed parent;
  BaconVideoWidgetPrivate *priv;
} BaconVideoWidget;

typedef struct
{
  GtkClutterEmbedClass parent_class;

  void (*error) (BaconVideoWidget * bvw, const char *message);
  void (*eos) (BaconVideoWidget * bvw);
  void (*got_metadata) (BaconVideoWidget * bvw);
  void (*segment_done) (BaconVideoWidget * bvw);
  void (*got_redirect) (BaconVideoWidget * bvw, const char *mrl);
  void (*title_change) (BaconVideoWidget * bvw, const char *title);
  void (*channels_change) (BaconVideoWidget * bvw);
  void (*tick) (BaconVideoWidget * bvw, gint64 current_time,
      gint64 stream_length, float current_position, gboolean seekable);
  void (*buffering) (BaconVideoWidget * bvw, guint progress);
  void (*state_change) (BaconVideoWidget * bvw, gboolean playing);
  void (*got_duration) (BaconVideoWidget * bvw);
  void (*ready_to_seek) (BaconVideoWidget * bvw);
} BaconVideoWidgetClass;

/**
 * BvwError:
 * @BVW_ERROR_AUDIO_PLUGIN: Error loading audio output plugin or device.
 * @BVW_ERROR_NO_PLUGIN_FOR_FILE: A required GStreamer plugin or xine feature is missing.
 * @BVW_ERROR_VIDEO_PLUGIN: Error loading video output plugin or device.
 * @BVW_ERROR_AUDIO_BUSY: Audio output device is busy.
 * @BVW_ERROR_BROKEN_FILE: The movie file is broken and cannot be decoded.
 * @BVW_ERROR_FILE_GENERIC: A generic error for problems with movie files.
 * @BVW_ERROR_FILE_PERMISSION: Permission was refused to access the stream, or authentication was required.
 * @BVW_ERROR_FILE_ENCRYPTED: The stream is encrypted and cannot be played.
 * @BVW_ERROR_FILE_NOT_FOUND: The stream cannot be found.
 * @BVW_ERROR_DVD_ENCRYPTED: The DVD is encrypted and libdvdcss is not installed.
 * @BVW_ERROR_INVALID_DEVICE: The device given in an MRL (e.g. DVD drive or DVB tuner) did not exist.
 * @BVW_ERROR_DEVICE_BUSY: The device was busy.
 * @BVW_ERROR_UNKNOWN_HOST: The host for a given stream could not be resolved.
 * @BVW_ERROR_NETWORK_UNREACHABLE: The host for a given stream could not be reached.
 * @BVW_ERROR_CONNECTION_REFUSED: The server for a given stream refused the connection.
 * @BVW_ERROR_INVALID_LOCATION: An MRL was malformed, or CDDB playback was attempted (which is now unsupported).
 * @BVW_ERROR_GENERIC: A generic error occurred.
 * @BVW_ERROR_CODEC_NOT_HANDLED: The audio or video codec required by the stream is not supported.
 * @BVW_ERROR_AUDIO_ONLY: An audio-only stream could not be played due to missing audio output support.
 * @BVW_ERROR_CANNOT_CAPTURE: Error determining frame capture support for a video with bacon_video_widget_can_get_frames().
 * @BVW_ERROR_READ_ERROR: A generic error for problems reading streams.
 * @BVW_ERROR_PLUGIN_LOAD: A library or plugin could not be loaded.
 * @BVW_ERROR_EMPTY_FILE: A movie file was empty.
 *
 * Error codes for #BaconVideoWidget operations.
 **/
typedef enum {
	/* Plugins */
	BVW_ERROR_AUDIO_PLUGIN,
	BVW_ERROR_NO_PLUGIN_FOR_FILE,
	BVW_ERROR_VIDEO_PLUGIN,
	BVW_ERROR_AUDIO_BUSY,
	/* File */
	BVW_ERROR_BROKEN_FILE,
	BVW_ERROR_FILE_GENERIC,
	BVW_ERROR_FILE_PERMISSION,
	BVW_ERROR_FILE_ENCRYPTED,
	BVW_ERROR_FILE_NOT_FOUND,
	/* Devices */
	BVW_ERROR_DVD_ENCRYPTED,
	BVW_ERROR_INVALID_DEVICE,
	BVW_ERROR_DEVICE_BUSY,
	/* Network */
	BVW_ERROR_UNKNOWN_HOST,
	BVW_ERROR_NETWORK_UNREACHABLE,
	BVW_ERROR_CONNECTION_REFUSED,
	/* Generic */
	BVW_ERROR_INVALID_LOCATION,
	BVW_ERROR_GENERIC,
	BVW_ERROR_CODEC_NOT_HANDLED,
	BVW_ERROR_AUDIO_ONLY,
	BVW_ERROR_CANNOT_CAPTURE,
	BVW_ERROR_READ_ERROR,
	BVW_ERROR_PLUGIN_LOAD,
	BVW_ERROR_EMPTY_FILE
} BvwError;

typedef enum {
	BVW_USE_TYPE_PLAYER,
	BVW_USE_TYPE_METADATA,
	BVW_USE_TYPE_CAPTURE
} BvwUseType;

EXPORT GQuark
bacon_video_widget_error_quark (void)
    G_GNUC_CONST;
     EXPORT GType bacon_video_widget_get_type (void) G_GNUC_CONST;
     EXPORT GOptionGroup *bacon_video_widget_get_option_group (void);
/* This can be used if the app does not use popt */
     EXPORT void bacon_video_widget_init_backend (int *argc, char ***argv);

     EXPORT GtkWidget *bacon_video_widget_new (BvwUseType use_type, GError ** error);

     EXPORT char *bacon_video_widget_get_backend_name (BaconVideoWidget * bvw);

/* Actions */
     EXPORT gboolean bacon_video_widget_open (BaconVideoWidget * bvw,
						                      const char *mrl,
						                      GError **error);
     EXPORT gboolean bacon_video_widget_play (BaconVideoWidget * bvw);
     EXPORT void bacon_video_widget_pause (BaconVideoWidget * bvw);
     EXPORT gboolean bacon_video_widget_is_playing (BaconVideoWidget * bvw);

/* Seeking and length */
     EXPORT gboolean bacon_video_widget_is_seekable (BaconVideoWidget * bvw);
     EXPORT gboolean bacon_video_widget_seek (BaconVideoWidget * bvw,
    gdouble position, gfloat rate);
     EXPORT gboolean bacon_video_widget_seek_time (BaconVideoWidget * bvw,
    gint64 time, gfloat rate, gboolean accurate);
     EXPORT gboolean bacon_video_widget_segment_seek (BaconVideoWidget * bvw,
    gint64 start, gint64 stop, gfloat rate);
     EXPORT gboolean bacon_video_widget_seek_in_segment (BaconVideoWidget *
    bvw, gint64 pos, gfloat rate);
     EXPORT gboolean bacon_video_widget_seek_to_next_frame (BaconVideoWidget *
    bvw, gfloat rate, gboolean in_segment);
     EXPORT gboolean
         bacon_video_widget_seek_to_previous_frame (BaconVideoWidget * bvw,
    gfloat rate, gboolean in_segment);
     EXPORT gboolean bacon_video_widget_segment_stop_update (BaconVideoWidget
    * bvw, gint64 stop, gfloat rate);
     EXPORT gboolean bacon_video_widget_segment_start_update (BaconVideoWidget
    * bvw, gint64 start, gfloat rate);
     EXPORT gboolean bacon_video_widget_new_file_seek (BaconVideoWidget * bvw,
    gint64 start, gint64 stop, gfloat rate);
     EXPORT gboolean bacon_video_widget_can_direct_seek (BaconVideoWidget *
    bvw);
     EXPORT double bacon_video_widget_get_position (BaconVideoWidget * bvw);
     EXPORT gint64 bacon_video_widget_get_current_time (BaconVideoWidget * bvw);
     EXPORT gint64 bacon_video_widget_get_stream_length (BaconVideoWidget *
    bvw);
     EXPORT gint64
         bacon_video_widget_get_accurate_current_time (BaconVideoWidget * bvw);
     EXPORT gboolean bacon_video_widget_set_rate (BaconVideoWidget * bvw,
    gfloat rate);
     EXPORT gboolean bacon_video_widget_set_rate_in_segment (BaconVideoWidget
    * bvw, gfloat rate, gint64 stop);



     EXPORT void bacon_video_widget_stop (BaconVideoWidget * bvw);
     EXPORT void bacon_video_widget_close (BaconVideoWidget * bvw);

/* Audio volume */
     EXPORT gboolean bacon_video_widget_can_set_volume (BaconVideoWidget * bvw);
     EXPORT void bacon_video_widget_set_volume (BaconVideoWidget * bvw,
    double volume);
     EXPORT double bacon_video_widget_get_volume (BaconVideoWidget * bvw);

/*Drawings Overlay*/
     EXPORT void bacon_video_widget_set_drawing_pixbuf (BaconVideoWidget * bvw, const GdkPixbuf * drawings);

/* Properties */
     EXPORT void bacon_video_widget_set_logo (BaconVideoWidget * bvw, const char *filename);
     EXPORT void bacon_video_widget_set_logo_pixbuf (BaconVideoWidget * bvw,
    GdkPixbuf * logo);
     EXPORT void bacon_video_widget_set_logo_mode (BaconVideoWidget * bvw,
    gboolean logo_mode);
     EXPORT gboolean bacon_video_widget_get_logo_mode (BaconVideoWidget * bvw);

     EXPORT void bacon_video_widget_set_fullscreen (BaconVideoWidget * bvw,
    gboolean fullscreen);


/* Metadata */
/**
 * BvwMetadataType:
 * @BVW_INFO_TITLE: the stream's title
 * @BVW_INFO_ARTIST: the artist who created the work
 * @BVW_INFO_YEAR: the year in which the work was created
 * @BVW_INFO_COMMENT: a comment attached to the stream
 * @BVW_INFO_ALBUM: the album in which the work was released
 * @BVW_INFO_DURATION: the stream's duration, in seconds
 * @BVW_INFO_TRACK_NUMBER: the track number of the work on the album
 * @BVW_INFO_COVER: a #GdkPixbuf of the cover artwork
 * @BVW_INFO_HAS_VIDEO: whether the stream has video
 * @BVW_INFO_DIMENSION_X: the video's width, in pixels
 * @BVW_INFO_DIMENSION_Y: the video's height, in pixels
 * @BVW_INFO_VIDEO_BITRATE: the video's bitrate, in kilobits per second
 * @BVW_INFO_VIDEO_CODEC: the video's codec
 * @BVW_INFO_FPS: the number of frames per second in the video
 * @BVW_INFO_HAS_AUDIO: whether the stream has audio
 * @BVW_INFO_AUDIO_BITRATE: the audio's bitrate, in kilobits per second
 * @BVW_INFO_AUDIO_CODEC: the audio's codec
 * @BVW_INFO_AUDIO_SAMPLE_RATE: the audio sample rate, in bits per second
 * @BVW_INFO_AUDIO_CHANNELS: a string describing the number of audio channels in the stream
 *
 * The different metadata available for querying from a #BaconVideoWidget
 * stream with bacon_video_widget_get_metadata().
 **/
     typedef enum
     {
       BVW_INFO_TITLE,
       BVW_INFO_ARTIST,
       BVW_INFO_YEAR,
       BVW_INFO_COMMENT,
       BVW_INFO_ALBUM,
       BVW_INFO_DURATION,
       BVW_INFO_TRACK_NUMBER,
       BVW_INFO_COVER,
       /* Video */
       BVW_INFO_HAS_VIDEO,
       BVW_INFO_DIMENSION_X,
       BVW_INFO_DIMENSION_Y,
       BVW_INFO_VIDEO_BITRATE,
       BVW_INFO_VIDEO_CODEC,
       BVW_INFO_FPS,
       /* Audio */
       BVW_INFO_HAS_AUDIO,
       BVW_INFO_AUDIO_BITRATE,
       BVW_INFO_AUDIO_CODEC,
       BVW_INFO_AUDIO_SAMPLE_RATE,
       BVW_INFO_AUDIO_CHANNELS
     } BvwMetadataType;

     EXPORT void bacon_video_widget_get_metadata (BaconVideoWidget * bvw,
    BvwMetadataType type, GValue * value);


/* Picture settings */
/**
 * BvwVideoProperty:
 * @BVW_VIDEO_BRIGHTNESS: the video brightness
 * @BVW_VIDEO_CONTRAST: the video contrast
 * @BVW_VIDEO_SATURATION: the video saturation
 * @BVW_VIDEO_HUE: the video hue
 *
 * The video properties queryable with bacon_video_widget_get_video_property(),
 * and settable with bacon_video_widget_set_video_property().
 **/
     typedef enum
     {
       BVW_VIDEO_BRIGHTNESS,
       BVW_VIDEO_CONTRAST,
       BVW_VIDEO_SATURATION,
       BVW_VIDEO_HUE
     } BvwVideoProperty;

     EXPORT int bacon_video_widget_get_video_property (BaconVideoWidget * bvw,
    BvwVideoProperty type);
     EXPORT void bacon_video_widget_set_video_property (BaconVideoWidget *
    bvw, BvwVideoProperty type, int value);


/* Screenshot functions */
     EXPORT gboolean bacon_video_widget_can_get_frames (BaconVideoWidget *
    bvw, GError ** error);
     EXPORT GdkPixbuf *bacon_video_widget_get_current_frame (BaconVideoWidget
    * bvw);
     EXPORT void bacon_video_widget_unref_pixbuf (GdkPixbuf * pixbuf);

G_END_DECLS
#endif /* HAVE_BACON_VIDEO_WIDGET_H */
