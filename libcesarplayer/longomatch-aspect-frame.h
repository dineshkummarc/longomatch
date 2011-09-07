
/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-aspect-frame.h: A container that respect the aspect ratio of its child
 *
 * Copyright 2010, 2011 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * Boston, MA 02111-1307, USA.
 *
 */

#ifndef __LONGOMATCH_ASPECT_FRAME_H__
#define __LONGOMATCH_ASPECT_FRAME_H__

#include <glib-object.h>
#include <mx/mx.h>

G_BEGIN_DECLS

#define LONGOMATCH_TYPE_ASPECT_FRAME longomatch_aspect_frame_get_type()

#define LONGOMATCH_ASPECT_FRAME(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  LONGOMATCH_TYPE_ASPECT_FRAME, LongomatchAspectFrame))

#define LONGOMATCH_ASPECT_FRAME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  LONGOMATCH_TYPE_ASPECT_FRAME, LongomatchAspectFrameClass))

#define LONGOMATCH_IS_ASPECT_FRAME(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  LONGOMATCH_TYPE_ASPECT_FRAME))

#define LONGOMATCH_IS_ASPECT_FRAME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  LONGOMATCH_TYPE_ASPECT_FRAME))

#define LONGOMATCH_ASPECT_FRAME_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  LONGOMATCH_TYPE_ASPECT_FRAME, LongomatchAspectFrameClass))

typedef struct _LongomatchAspectFrame LongomatchAspectFrame;
typedef struct _LongomatchAspectFrameClass LongomatchAspectFrameClass;
typedef struct _LongomatchAspectFramePrivate LongomatchAspectFramePrivate;

struct _LongomatchAspectFrame
{
  MxBin parent;

  LongomatchAspectFramePrivate *priv;
};

struct _LongomatchAspectFrameClass
{
  MxBinClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType           longomatch_aspect_frame_get_type    (void) G_GNUC_CONST;

ClutterActor *  longomatch_aspect_frame_new         (void);

void            longomatch_aspect_frame_set_expand  (LongomatchAspectFrame *frame,
                                             gboolean       expand);
gboolean        longomatch_aspect_frame_get_expand  (LongomatchAspectFrame *frame);

void            longomatch_aspect_frame_set_ratio   (LongomatchAspectFrame *frame,
                                             gfloat         ratio);
gfloat          longomatch_aspect_frame_get_ratio   (LongomatchAspectFrame *frame);

G_END_DECLS

#endif /* __LONGOMATCH_ASPECT_FRAME_H__ */
