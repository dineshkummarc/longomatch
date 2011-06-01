//
//  Copyright (C) 2009 Andoni Morales Alastruey
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
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//

using System;
using Gdk;

namespace LongoMatch.TimeNodes
{
	
	[Serializable]
	public class Drawing
	{
		private byte[] drawingBuf;

		/// <summary>
		/// Represent a drawing in the database using a {@Gdk.Pixbuf} stored
	    /// in a bytes array in PNG format for serialization. {@Drawings}
	    /// are used by {@MediaTimeNodes} to store the key frame drawing
	    /// which stop time is stored in a int value
		/// </summary>
		public Drawing() {
		}

		/// <summary>
		/// Pixbuf with the drawing
		/// </summary>
		public Pixbuf Pixbuf {
			get {
				if (drawingBuf != null)
					return new Pixbuf(drawingBuf);
				else return null;
			}
			set {
				if (value != null)
					drawingBuf = value.SaveToBuffer("png");
				else
					drawingBuf = null;
			}
		}
		
		/// <summary>
		/// Render time of the drawing
		/// </summary>
		public int RenderTime {
			get;
			set;
		}
	
		/// <summary>
		/// Time to pause the playback and display the drawing
		/// </summary>
		public int PauseTime {
			set;
			get;
		}
	}
}
