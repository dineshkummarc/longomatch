/*
 * Farsight Voice+Video library
 *
 *   @author: Ole André Vadla Ravnås <oleavr@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>

#ifdef G_OS_WIN32
#include <windows.h>
#else
#include <winbase.h>
#include <ldt_keeper.h>
#endif
#include <psapi.h>

#include "mscodec.h"
#include "gstmsenc.h"
#include "gstmsdec.h"
#include "msbufferbase.h"
#include "msbufferrtpextheader.h"
#include "msbufferstream.h"
#include "msrtaudiohealer.h"
#include "gstrtprtaudiopay.h"
#include "gstrtprtaudiodepay.h"
#include "gstmsrtahealer.h"

GST_DEBUG_CATEGORY (mscodecs_debug);
#define GST_CAT_DEFAULT (mscodecs_debug)

#ifdef G_OS_WIN32
#define RTMPLPTFM_PATH "C:\\Program Files\\Windows Live\\Messenger\\RTMPLTFM.dll"
#else
#define RTMPLPTFM_PATH "RTMPLTFM.dll"
#endif

typedef struct {
  gulong image_size;
  gulong image_base;

  gulong debug_handler_va;

  gulong codec_table_va;
  guint codec_table_len;

  gulong buffer_base_init;

  gulong buffer_rtp_ext_header_new_va;

  gulong buffer_stream_new_va;
  gulong buffer_stream_add_buffer_va;
  gulong buffer_stream_update_oas_va;

  gulong rta_healer_new_va;
  gulong rta_healer_free_va;
} KnownVersion;

static const KnownVersion known_versions[] =
{
  {
    4042752, 0x400000,                  /* Image size and base   */
    0x005071D6,                         /* Debug handler         */
    0x00701330, 9,                      /* Codec table           */
    0x0050a313,                         /* MSBufferBase          */
    0x004b994a,                         /* MSBufferRtpExtHeader  */
    0x0050a694, 0x0050a73d, 0x004b40ac, /* MSBufferStream        */
    0x0053d990, 0x0053d59c              /* MSRTAudioHealer       */
  }
};

static const guint num_known_versions =
    sizeof (known_versions) / sizeof (known_versions[0]);

typedef struct {
  const gchar * canonical_name;
  const gchar * element_name;
  const gchar * mimetype;
  gint bitrate_offset;          /*< offset into encoder object, as there's no
                                 *  accessor method on the encoder interface */
  gboolean supports_lossrate;
} KnownCodec;

static const KnownCodec known_codecs[] =
{
  { "G.711-ALaw",     "alaw",   "audio/x-alaw",   -1, FALSE }, /*  4 */
  { "G.711-MuLaw",    "mulaw",  "audio/x-mulaw",  -1, FALSE }, /*  5 */
  { "MSRTAudio8KHz",  "rta8",   "audio/x-msrta",  24, TRUE  }, /*  6 */
  { "MSRTAudio16KHz", "rta16",  "audio/x-msrta",  24, TRUE  }, /*  7 */
  { "G.723.1",        "g7231",  "audio/x-g7231",  -1, FALSE }, /*  8 */
  { "SIREN",          "siren",  "audio/x-siren",  -1, FALSE }, /*  9 */
  { "G.722.1",        "g7221",  "audio/x-g7221",  -1, FALSE }, /* 10 */
  { "GSM6.10",        "gsm610", "audio/x-gsm610", -1, FALSE }, /* 11 */
  { "G.726",          "g726",   "audio/x-g726",    8, FALSE }, /* 12 */
};

#define MS_CODEC_ID_MIN  4
#define MS_CODEC_ID_MAX 12

static gint WINAPI
gst_mscodecs_debug_handler (gint a, gint b, gint c, gint d,
                            gchar * format, va_list args)
{
  vprintf (format, args);
  printf ("\n");
  fflush (stdout);
  return 0;
}

#define OPCODE_JMP 0xE9

static gboolean
gst_mscodecs_intercept_debug_handler (guint8 * func_addr)
{
#ifdef G_OS_WIN32
  /* This isn't enforced in the non-Windows implementation anyway... */
  DWORD old_protect;
  BOOL result;

  result = VirtualProtect (func_addr, 5, PAGE_EXECUTE_READWRITE, &old_protect);
  g_assert (result);
#endif

  *func_addr = OPCODE_JMP;
  *((guint32 *) (func_addr + 1)) =
      (guint8 *) gst_mscodecs_debug_handler - (func_addr + 5);

  return TRUE;
}

#define VA_FROM_ENTRY(entry) \
  ((gpointer) (module_base + (ver->entry - ver->image_base)))

static gboolean
gst_mscodecs_initialize (GstPlugin * plugin)
{
  gboolean ret = FALSE;
  guint8 * module_base = NULL;
  MODULEINFO mi = { 0, };
#ifdef G_OS_WIN32
  BOOL result;
#endif
  guint ver_idx;

#ifndef G_OS_WIN32
  Setup_LDT_Keeper ();

  Check_FS_Segment ();
#endif

  /* FIXME: Auto-detect/probe for the path */
  module_base = (guint8 *) LoadLibrary (RTMPLPTFM_PATH);
  if (module_base == NULL)
  {
    GST_ERROR ("Failed to load RTMPLTFM.dll");
    goto beach;
  }

#ifdef G_OS_WIN32
  result = GetModuleInformation (GetCurrentProcess (), (HMODULE) module_base,
      &mi, sizeof (mi));
  if (!result)
  {
    GST_ERROR ("GetModuleInformation failed");
    goto beach;
  }
#else
  mi.SizeOfImage = 4042752; /* FIXME: hack */
#endif

  for (ver_idx = 0; ver_idx < num_known_versions; ver_idx++)
  {
    const KnownVersion * ver = &known_versions[ver_idx];

    if (ver->image_size == mi.SizeOfImage)
    {
      guint codec_idx;
      const MSCodec ** codecs = (const MSCodec **)
          (module_base + (ver->codec_table_va - ver->image_base));
      guint codec_count = 0;

      GST_INFO ("Found known DLL version (SizeOfImage = %d)", mi.SizeOfImage);

      /* Internal debug could be useful for tracking down errors. */
      gst_mscodecs_intercept_debug_handler (VA_FROM_ENTRY (debug_handler_va));

      /* Initialize function pointers for various types. */
      ms_buffer_base_class_init (VA_FROM_ENTRY (buffer_base_init));

      ms_buffer_rtp_ext_header_class_init (
          VA_FROM_ENTRY (buffer_rtp_ext_header_new_va));

      ms_buffer_stream_class_init (VA_FROM_ENTRY (buffer_stream_new_va),
          VA_FROM_ENTRY (buffer_stream_add_buffer_va),
          VA_FROM_ENTRY (buffer_stream_update_oas_va));

      ms_rtaudio_healer_class_init (VA_FROM_ENTRY (rta_healer_new_va),
          VA_FROM_ENTRY (rta_healer_free_va));

      /* Register codecs. */
      for (codec_idx = 0; codec_idx < ver->codec_table_len; codec_idx++)
      {
        const MSCodec * codec = codecs[codec_idx];

        if (codec->id >= MS_CODEC_ID_MIN &&
            codec->id <= MS_CODEC_ID_MAX)
        {
          const KnownCodec * kc = &known_codecs[codec->id - MS_CODEC_ID_MIN];

          if (strcmp (codec->name, kc->canonical_name) == 0)
          {
            GST_INFO ("Registering %6s [%-14s]", kc->element_name,
                kc->canonical_name);

            gst_msenc_register (plugin, kc->element_name, kc->mimetype,
                kc->bitrate_offset, kc->supports_lossrate, codec);
            gst_msdec_register (plugin, kc->element_name, kc->mimetype, codec);

            codec_count++;
          }
          else
          {
            GST_WARNING ("Name mismatch; found '%s', expected '%s'",
                codec->name, kc->canonical_name);
          }
        }
        else
        {
          GST_DEBUG ("Ignoring unknown codec with id %d", codec->id);
        }
      }

      GST_INFO ("Registered %d codecs", codec_count);

      break;
    }
  }

  ret = TRUE;

beach:
  /* FIXME: handle unloading */
  if (!ret && module_base != NULL)
    FreeLibrary ((HMODULE) module_base);

  return ret;
}

static gboolean
gst_mscodecs_plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (mscodecs_debug, "mscodecs", 0,
      "Microsoft binary codecs");

  if (!gst_mscodecs_initialize (plugin))
    return FALSE;

  if (!gst_element_register (plugin, "rtprtaudiopay", GST_RANK_NONE,
      GST_TYPE_RTP_RTAUDIO_PAY))
    return FALSE;

  if (!gst_element_register (plugin, "rtprtaudiodepay", GST_RANK_NONE,
      GST_TYPE_RTP_RTAUDIO_DEPAY))
    return FALSE;

  if (!gst_element_register (plugin, "msrtahealer", GST_RANK_NONE,
      GST_TYPE_MS_RTA_HEALER))
    return FALSE;

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "mscodecs",
    "Microsoft binary codecs",
    gst_mscodecs_plugin_init,
    VERSION, "LGPL", "Farsight",
    "http://farsight.sf.net")
