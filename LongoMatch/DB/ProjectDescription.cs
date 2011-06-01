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
using LongoMatch.TimeNodes;
using LongoMatch.Video.Utils;

namespace LongoMatch.DB
{

	/// <summary>
	/// I'm used like a presentation card for projects. I speed up the retrieval
	/// from the database be using only the field required to describe a project
	/// </summary>
	[Serializable]
	public class ProjectDescription :  IComparable
	{
		
		public String Title {
			get {
				return System.IO.Path.GetFileNameWithoutExtension(File.FilePath);
			}
		}

		public PreviewMediaFile File {
			get;
			set;
		}

		public String Season {
			get;
			set;
		}

		public String Competition {
			get;
			set;
		}

		public String LocalName {
			get;
			set;
		}

		public String VisitorName {
			get;
			set;
		}

		public int LocalGoals {
			get;
			set;
		}

		public int VisitorGoals {
			get;
			set;
		}


		public DateTime MatchDate {
			get;
			set;
		}

		public String VideoCodec {
			get;
			set;
		}
		
		public String AudioCodec {
			get;
			set;
		}
		
		public String Format {
			get{
				return String.Format("{0}x{1}@{2}fps", 
				              File.VideoWidth, File.VideoHeight, File.Fps);
			}
		}

		public int CompareTo(object obj) {
			if (obj is ProjectDescription) {
				ProjectDescription project = (ProjectDescription) obj;

				return this.File.FilePath.CompareTo(project.File.FilePath);
			}
			else
				throw new ArgumentException("object is not a ProjectDescription and cannot be compared");
		}
	}
}
