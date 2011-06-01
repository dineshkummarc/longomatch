// PixbufTimeNode.cs
//
//  Copyright (C) 2007-2009 Andoni Morales Alastruey
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
//

using System;
using Gdk;

namespace LongoMatch.Store
{

	/// <summary>
	/// Base class for all the video segments containing a snapshot
	/// It has a <see cref="Gdk.Pixbuf"/> with a thumbnail of the video segment.
	/// </summary>
	[Serializable]
	public class PixbufTimeNode : TimeNode
	{
		private byte[] thumbnailBuf;
		private const int MAX_WIDTH=100;
		private const int MAX_HEIGHT=75;
		#region Contructors
		public PixbufTimeNode() {
		}
		#endregion

		#region Properties
		/// <summary>
		/// Segment thumbnail
		/// </summary>
		public Pixbuf Miniature {
			get {
				if (thumbnailBuf != null)
					return new Pixbuf(thumbnailBuf);
				else return null;
			}set {
				ScaleAndSave(value);
			}
		}
		#endregion

		private void ScaleAndSave(Pixbuf pixbuf) {
			int ow,oh,h,w;

			h = ow = pixbuf.Height;
			w = oh = pixbuf.Width;
			ow = MAX_WIDTH;
			oh = MAX_HEIGHT;

			if (w>MAX_WIDTH || h>MAX_HEIGHT) {
				double rate = (double)w/(double)h;
				if (h>w)
					ow = (int)(oh * rate);
				else
					oh = (int)(ow / rate);
				thumbnailBuf = pixbuf.ScaleSimple(ow,oh,Gdk.InterpType.Bilinear).SaveToBuffer("png");
				pixbuf.Dispose();
			}
			else thumbnailBuf =  pixbuf.SaveToBuffer("png");
		}
	}
}
