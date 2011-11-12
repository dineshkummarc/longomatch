// 
//  Copyright (C) 2011 andoni
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
using System.Collections.Generic;

namespace LongoMatch.Store
{
	[Serializable]
	public class GameUnit: List<TimeNode>
	{
		
		public GameUnit (string name)
		{
			Name=name;
		}
		
		public string Name {
			get;
			set;
		}
		
		public override string ToString ()
		{
			return string.Format ("[GameUnit: Name={0}]", Name);
		}	
	}
}

