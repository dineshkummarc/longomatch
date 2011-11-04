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

namespace LongoMatch.Video.Editor
{


	public class VideoSegment
	{
		private string filePath;
		private long start;
		private long duration;
		private double rate;
		private string title;
		private bool hasAudio;

		public VideoSegment(string filePath, long start, long duration, double rate, string title,bool hasAudio)
		{
			this.filePath = filePath;
			this.start = start;
			this.duration = duration;
			this.rate = rate;
			this.title = title;
			this.hasAudio= hasAudio;
		}

		public string FilePath {
			get {
				return filePath;
			}
		}

		public string Title {
			get {
				return title;
			}
		}

		public long Start {
			get {
				return start;
			}
		}

		public long Duration {
			get {
				return duration;
			}
		}

		public double Rate {
			get {
				return rate;
			}
		}

		public bool HasAudio {
			get {
				return hasAudio;
			}
		}


	}
}
