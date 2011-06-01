// NotesWidget.cs
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
using Gtk;
using LongoMatch.TimeNodes;
using LongoMatch.Handlers;

namespace LongoMatch.Gui.Component
{

	[System.ComponentModel.Category("LongoMatch")]
	[System.ComponentModel.ToolboxItem(true)]
	public partial class NotesWidget : Gtk.Bin
	{
		public event TimeNodeChangedHandler TimeNodeChanged;
		TextBuffer buf;
		Play play;

		public NotesWidget()
		{
			this.Build();
			this.buf = textview1.Buffer;
			buf.Changed += new EventHandler(OnEdition);

		}

		public Play Play {
			set {
				play = value;
				Notes = play.Notes;
			}
		}
		string Notes {
			set {
				buf.Clear();
				buf.InsertAtCursor(value);
			}
			get {
				return buf.GetText(buf.StartIter,buf.EndIter,true);
			}
		}

		protected virtual void OnEdition(object sender, EventArgs args) {
			if (Notes != play.Notes) {
				savebutton.Sensitive = true;
			}
		}

		protected virtual void OnSavebuttonClicked(object sender, System.EventArgs e)
		{
			if (play != null) {
				play.Notes=Notes;
				if (TimeNodeChanged != null)
					TimeNodeChanged(play,null);
				savebutton.Sensitive = false;
			}
		}

	}
}
