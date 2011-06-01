//
//  Copyright (C) 2007-2009 Andoni Morales Alastruey
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

using System;
using Gdk;
using LongoMatch.Common;

namespace LongoMatch.Store
{
	/// <summary>
	/// Player of a team
	/// </summary>
	[Serializable]
	public class Player
	{
		private byte[] photo;

		#region Constructors
		public Player()
		{
		}
		#endregion

		#region Properties
		/// <summary>
		/// My name
		/// </summary>
		public string Name {
			get;
			set;
		}

		/// <summary>
		/// My position in the field
		/// </summary>
		public string Position {
			get;
			set;
		}

		/// <summary>
		/// My shirt number
		/// </summary>
		public int Number {
			get;
			set;
		}

		/// <summary>
		/// My photo
		/// </summary>
		public Pixbuf Photo {
			get {
				if (photo != null)
					return new Pixbuf(photo);
				else
					return null;
			}
			set {
				photo=value.SaveToBuffer("png");
			}
		}
		
		/// <summary>
		/// Date of birth
		/// </summary>
		public DateTime Birthday{
			get;
			set;
		}
		
		/// <summary>
		/// Nationality
		/// </summary>
		public String Nationality{
			get;
			set;
		}
		
		/// <summary>
		/// Height
		/// </summary>
		public float Height{
			get;
			set;
		}

		/// <summary>
		/// Weight
		/// </summary>
		public int Weight{
			get;
			set;
		}
		
		/// <summary>
		/// Whether this player is playing or not and shouldn't be added the 
		/// list of taggable players 
		/// </summary>
		public bool Playing{
			get;
			set;
		}
		
		#endregion
	}
}
