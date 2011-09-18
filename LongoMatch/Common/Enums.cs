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

namespace LongoMatch.Common
{


	public enum ProjectType {
		CaptureProject,
		FakeCaptureProject,
		FileProject,
		EditProject,
		None,
	}

	public enum EndCaptureResponse {
		Return = 234,
		Quit = 235,
		Save = 236
	}

	public enum TagMode {
		Predifined,
		Free
	}

	public enum SortMethodType {
		SortByName = 0,
		SortByStartTime = 1,
		SortByStopTime = 2,
		SortByDuration = 3
	}

	public enum Team {
		NONE = 0,
		LOCAL = 1,
		VISITOR = 2,
	}
	
	public enum JobState {
		NotStarted,
		Running,
		Finished,
		Cancelled,
		Error,
	}
}
