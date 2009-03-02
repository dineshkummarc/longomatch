/* Farsight
 * Copyright (C) <2005> Philippe Khalaf <burger@speedy.org> 
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

#include "gstmsgsmpayload.h"
#include "gstmsgsmdepayload.h"
#include "gstdvpayload.h"
#include "gstdvdepayload.h"
#include "gstrtpcnpayload.h"
#include "gstrtpcndepayload.h"
#ifdef ENABLE_G729
#include "gstrtpg729pay.h"
#include "gstrtpg729depay.h"
#endif

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_msgsmpayload_plugin_init (plugin))
    return FALSE;
  if (!gst_msgsmdepayload_plugin_init (plugin))
    return FALSE;
  if (!gst_dvpayload_plugin_init (plugin))
    return FALSE;
  if (!gst_dvdepayload_plugin_init (plugin))
    return FALSE;
  if (!gst_rtpcnpayload_plugin_init (plugin))
    return FALSE;
  if (!gst_rtpcndepayload_plugin_init (plugin))
    return FALSE;
#ifdef ENABLE_G729
  if (!gst_rtp_g729_pay_plugin_init (plugin))    
    return FALSE;
  if (!gst_rtp_g729_depay_plugin_init (plugin))    
    return FALSE;
#endif

 return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "rtppayloads",
    "Real-time protocol (RTP) payloaders/depayloader",
    plugin_init, VERSION, 
    "GPL", 
    "Farsight", 
    "http://farsight.sf.net");
