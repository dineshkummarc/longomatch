#define _CRT_SECURE_NO_WARNINGS 1

#include <glib.h>
#include <stdio.h>
#include <windows.h>
#include <psapi.h>

#include "msrtaudiohealer.h"
#include "msbufferrtpextheader.h"
#include "msbuffercustom.h"
#include "msbufferstream.h"
#include "mscodec.h"

/* Temporarily stolen from gstmscodecs.c */
#pragma region Debugging infrastructure interception

const guint8 debug_func_signature[] = { 0x8B, 0xFF, 0x55, 0x8B, 0xEC, 0x8B, 0x45, 0x08, 0x56, 0x57, 0x8B, 0xF0, 0x33, 0xFF, 0x83, 0xE6, 0x0F, 0x39 };

#define OPCODE_JMP 0xE9

static gint __stdcall
gst_mscodecs_debug_handler (gint a, gint b, gint c, gint d,
                            gchar * format, va_list args)
{
  vprintf (format, args);
  printf ("\n");
  fflush (stdout);
  return 0;
}

static gboolean
gst_mscodecs_intercept_debug_handler (guint8 * mod_start, guint8 * mod_end)
{
  guint8 * func_addr = NULL;
  guint8 * p = mod_start;
  DWORD old_protect;
  BOOL result;

  while (p <= mod_end - sizeof (debug_func_signature))
  {
    if (memcmp (p, debug_func_signature, sizeof (debug_func_signature)) == 0)
    {
      func_addr = p;
      break;
    }

    p++;
  }

  if (func_addr == NULL)
  {
    g_debug ("debug function not found");
    return FALSE;
  }

  g_debug ("found debug function at 0x%p", func_addr);

  result = VirtualProtect (func_addr, 5, PAGE_EXECUTE_READWRITE, &old_protect);
  g_assert (result);

  *func_addr = OPCODE_JMP;
  *((guint32 *) (func_addr + 1)) =
      (guint8 *) gst_mscodecs_debug_handler - (func_addr + 5);

  return TRUE;
}

#pragma endregion

typedef HRESULT (WINAPI * MSRTAudioHealerCreateFunc) (MSAudioHealer ** healer, MSRTAudioHealerMallocFunc malloc_impl, MSRTAudioHealerFreeFunc free_impl);
typedef HRESULT (WINAPI * MSRTAudioHealerDestroyFunc) (MSAudioHealer * healer);

static gpointer __stdcall
malloc_impl (gsize size)
{
  gpointer ret = malloc (size);
  g_debug ("%s: %d => %p", G_STRFUNC, size, ret);
  return ret;
}

static void __stdcall
free_impl (gpointer memory)
{
  g_debug ("%s: %p", G_STRFUNC, memory);
  free (memory);
}

gint
main (gint argc, gchar * argv[])
{
  HMODULE mod;
  MODULEINFO mi = { 0, };
  BOOL result;
  guint8 * mod_start, * mod_end;
  MSRTAudioHealerCreateFunc ms_audio_healer_create;
  MSRTAudioHealerDestroyFunc ms_audio_healer_destroy;
  MSAudioHealer * healer = NULL;
  MSBufferStream * stream;
  MSBufferRtpExtHeader * rtp_buffer;
  MSBufferRtpExtHeaderData * hd;
  MSBufferCustom * encoded_buffer, * decoded_buffer;
  gint ret;
  gpointer p;
  static const guint8 inbuf[98] = { 0x3f, 0xf9, 0x2a, 0x5a, 0x5b, 0x8c, 0x63, 0x10, 0xff, 0x3e, 0x99, 0xee, 0x1a, 0x1a, 0xd7, 0x13, 0x3f, 0xe2, 0xbf, 0x08, 0x35, 0x78, 0x99, 0x3f, 0x11, 0xe6, 0x92, 0xf1, 0xa5, 0x74, 0x3e, 0x96, 0xeb, 0xff, 0xf9, 0xeb, 0xa2, 0xe4, 0x36, 0x58, 0x62, 0x2d, 0x59, 0xb7, 0xe8, 0x0c, 0x7e, 0xbb, 0x46, 0x08, 0x04, 0x66, 0xb2, 0x78, 0x9b, 0xb6, 0x59, 0xe3, 0xad, 0xe4, 0x17, 0x4b, 0x7e, 0x9c, 0x63, 0x18, 0x87, 0xf9, 0xf0, 0x00, 0x04, 0x75, 0x90, 0xfd, 0xff, 0x7e, 0xd4, 0x5f, 0xc8, 0x59, 0x05, 0xb6, 0xc2, 0xb3, 0xf2, 0x85, 0x0a, 0x14, 0x20, 0x7f, 0x9f, 0x67, 0xcf, 0xde, 0xa6, 0x52, 0xce, 0x40 };
  guint8 outbuf[640] = { 0, };
  gint i;

  g_assert (sizeof (MSBufferBase) == 128);
  g_assert (sizeof (MSBufferRtpExtHeader) == 280);

  mod = LoadLibraryA ("RTMPLTFM.dll");
  g_assert (mod != NULL);

  result = GetModuleInformation (GetCurrentProcess (), mod, &mi, sizeof (mi));
  g_assert (result);

  mod_start = (guint8 *) mod;
  mod_end = mod_start + mi.SizeOfImage;

  gst_mscodecs_intercept_debug_handler (mod_start, mod_end);

  /* Init function pointers */
  ms_audio_healer_create = (MSRTAudioHealerCreateFunc) ((guint8 *) mod + (0x53d990 - 0x400000));
  ms_audio_healer_destroy = (MSRTAudioHealerDestroyFunc) ((guint8 *) mod + (0x53d59c - 0x400000));

  ms_buffer_base_class_init ((guint8 *) mod + (0x50a313 - 0x400000));
  ms_buffer_rtp_ext_header_class_init ((guint8 *) mod + (0x4b994a - 0x400000));
  ms_buffer_stream_class_init (
    (guint8 *) mod + (0x50a694 - 0x400000), /* new_impl                    */
    (guint8 *) mod + (0x50a73d - 0x400000), /* add_buffer_impl             */
    (guint8 *) mod + (0x4b40ac - 0x400000)  /* update_offset_and_size_impl */
    );

  /* Create the healer */
  ret = ms_audio_healer_create (&healer, malloc_impl, free_impl);
  g_assert (ret == 0);

  ret = ms_audio_healer_start (healer, 0);
  g_assert (ret == 0);

  /* Create a buffer stream and add buffers to it */
  stream = ms_buffer_stream_new ();
  g_assert (stream != NULL);

  /* RTP ext header buffer */
  rtp_buffer = ms_buffer_rtp_ext_header_new ();
  g_assert (rtp_buffer != NULL);

  rtp_buffer->parent.timestamp = 200000; /* 20 ms in 100 nanosecond units */

  hd = &rtp_buffer->header_data;
  hd->packet_ntp_timestamp = 0.02;
  hd->seq_no = 1337;
  hd->timestamp = rtp_buffer->parent.timestamp;
  hd->unknown_enum_value = 1;
  hd->codec_id = MS_CODEC_ID_RTA16;
  hd->ssrc = 1234;
  hd->marker_bit_set = TRUE;
  hd->unknown_bool_for_enum_set = FALSE;
  hd->is_dtmf = FALSE;
  hd->csrc_count = 0;

  p = ms_buffer_stream_add_buffer (stream, 2, MS_BUFFER_BASE (rtp_buffer));
  g_assert (p != NULL);

  ms_buffer_stream_update_offset_and_size (stream, 2, 0, rtp_buffer->parent.payload_size);

  /* Encoded buffer */
  encoded_buffer = ms_buffer_custom_new ((guint8 *) inbuf, sizeof (inbuf));
  g_assert (encoded_buffer != NULL);

  p = ms_buffer_stream_add_buffer (stream, 16, MS_BUFFER_BASE (encoded_buffer));
  g_assert (p != NULL);

  ms_buffer_stream_update_offset_and_size (stream, 16, 0, sizeof (inbuf));

  /* Prepare the output buffer */
  decoded_buffer = ms_buffer_custom_new (outbuf, sizeof (outbuf));
  g_assert (decoded_buffer != NULL);

  p = ms_buffer_stream_add_buffer (stream, 10, MS_BUFFER_BASE (decoded_buffer));
  g_assert (p != NULL);

  ms_buffer_stream_update_offset_and_size (stream, 10, 0, sizeof (outbuf));

  /* Pull out 20 ms of data */
  ret = ms_audio_healer_pull_samples (healer, stream, 0, 20);
  g_assert (ret == 0);

  /* Tell the healer to process the stream */
  ret = ms_audio_healer_push_samples (healer, stream, 0);
  g_assert (ret == 0);

  /* Ask the healer for 20 ms of audio for 1 second */
  for (i = 0; i < 5 * 10; i++)
  {
    ret = ms_audio_healer_pull_samples (healer, stream, 0, 20);
    g_assert (ret == 0);

    Sleep (20);
  }

  /*
  g_debug ("calling ms_audio_healer_destroy");
  ret = ms_audio_healer_destroy (healer);
  g_debug ("ms_audio_healer_destroy ret = 0x%08x", ret);
  */

  return 0;
}

