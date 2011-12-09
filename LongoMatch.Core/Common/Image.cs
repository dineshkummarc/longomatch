// 
//  Copyright (C) 2011 Andoni Morales Alastruey
// 
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
// 
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
// 

namespace LongoMatch.Common
{
	using System;
	using System.IO;
#if HAVE_GTK
	using SImage = Gdk.Pixbuf;
#else
	using System.Drawing.Imaging;
	using SImage = System.Drawing.Image;
#endif

	public class Image
	{
		SImage image;
		
		public Image (SImage image)
		{
			this.image = image;
		}
		
		public SImage Value {
			get {
				return image;
			}
		}
		
#if HAVE_GTK
		public byte[] Serialize () {
			byte[] ser;
			
			if (image == null)
				return null;
			return image.SaveToBuffer("png");
		}
		
		static Pixbuf Deserialize (byte[] ser) {
			return new Pixbuf(ser);
		}
#else
		public byte[] Serialize () {
			if (image == null)
				return null;
			using (MemoryStream stream = new MemoryStream()) {
				image.Save(stream, ImageFormat.Png);
				byte[] buf = new byte[stream.Length - 1];
				stream.Position = 0;
				stream.Read(buf, 0, buf.Length);
				return buf;
			}
		}
		
		public static Image Deserialize (byte[] ser) {
			Image img = null;
			using (MemoryStream stream = new MemoryStream(ser)) {
				img = new Image(System.Drawing.Image.FromStream(stream));
			}
			return img;
		}
		
		public void Dispose() {
			image.Dispose();
		}
#endif
	}
}

