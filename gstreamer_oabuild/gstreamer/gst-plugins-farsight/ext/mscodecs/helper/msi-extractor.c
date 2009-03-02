/*
 * Copyright (C) 2008  Ole André Vadla Ravnås <oleavr@gmail.com>
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

#include <stdio.h>
#include <glib-object.h>
#include <gsf/gsf.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-output-stdio.h>

gint
main (gint argc, gchar * argv[])
{
  gint result = 1;
  GsfInput * input = NULL;
  GsfInfile * infile = NULL;
  gint num_children, i;
  GsfInput * cab_input = NULL;
  gsf_off_t largest_size;
  GsfOutput * output = NULL;
  gboolean copy_ret;

  g_type_init ();
  
  if (argc != 3)
  {
    fprintf (stderr, "Usage: %s <input.msi> <output.cab>\n", argv[0]);
    goto beach;
  }

  input = gsf_input_stdio_new (argv[1], NULL);
  if (input == NULL)
  {
    fprintf (stderr, "Failed to open %s\n", argv[1]);
    goto beach;
  }

  infile = gsf_infile_msole_new (input, NULL);
  if (infile == NULL)
  {
    fprintf (stderr, "Failed to parse %s\n", argv[1]);
    goto beach;
  }

  num_children = gsf_infile_num_children (infile);

  largest_size = 0;
  for (i = 0; i < num_children; i++)
  {
    GsfInput * child;
    gsf_off_t size;

    child = gsf_infile_child_by_index (infile, i);

    size = gsf_input_size (child);
    if (size > largest_size)
    {
      largest_size = size;

      if (cab_input != NULL)
        g_object_unref (cab_input);
      g_object_ref (child);
      cab_input = child;
    }

    g_object_unref (child);
  }

  output = gsf_output_stdio_new (argv[2], NULL);
  if (output == NULL)
  {
    fprintf (stderr, "Failed to open output file\n");
    goto beach;
  }

  gsf_input_seek (cab_input, 0, G_SEEK_SET);
  gsf_output_seek (output, 0, G_SEEK_SET);
  copy_ret = gsf_input_copy (cab_input, output);
  if (!copy_ret)
  {
    fprintf (stderr, "Copy failed\n");
    goto beach;
  }

  gsf_output_close (output);

  printf ("%s: Wrote %lu bytes\n", argv[2], (gulong) largest_size);
  result = 0;

beach:
  if (output != NULL)
    g_object_unref (output);

  if (cab_input != NULL)
    g_object_unref (cab_input);

  if (infile != NULL)
    g_object_unref (infile);

  if (input != NULL)
    g_object_unref (input);

  return result;
}

