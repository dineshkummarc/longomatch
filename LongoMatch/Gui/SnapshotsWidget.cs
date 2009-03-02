// SnapshotsWidget.cs 
//
//  Copyright (C) 2008 Andoni Morales Alastruey
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//

using System;
using LongoMatch.TimeNodes;
using LongoMatch.Handlers;

namespace LongoMatch.Gui.Component
{
	
	
	public partial class SnapshotsWidget : Gtk.Bin
	{
		public event SnapshotSeriesHandler SnapshotSeriesEvent;
		private MediaTimeNode tNode;
		
		public SnapshotsWidget()
		{
			this.Build();
		}
		
		public MediaTimeNode Play{
			set{ 
				tNode = value;
				playLabel.Text = tNode.Name;}
		}

		protected virtual void OnButton1Clicked (object sender, System.EventArgs e)
		{
			if (SnapshotSeriesEvent != null && tNode != null){
				uint interval; 
				string seriesName;
				
				interval = (uint)spinbutton1.Value;
				seriesName = entry1.Text;
				SnapshotSeriesEvent(tNode,seriesName, interval);
			}				
				
		}
	}
}