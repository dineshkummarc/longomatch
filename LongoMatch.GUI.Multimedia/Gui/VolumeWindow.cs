// VolumeWindow.cs
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
using LongoMatch.Video.Common;
using LongoMatch.Handlers;

namespace LongoMatch.Gui
{


	public partial class VolumeWindow : Gtk.Window
	{



		public event         VolumeChangedHandler VolumeChanged;


		public VolumeWindow() :
		base(Gtk.WindowType.Toplevel)
		{
			this.Build();
			volumescale.Adjustment.PageIncrement = 0.0001;
			volumescale.Adjustment.StepIncrement = 0.0001;
		}

		public void SetLevel(double level) {
			volumescale.Value = level ;
		}

		protected virtual void OnLessbuttonClicked(object sender, System.EventArgs e)
		{
			volumescale.Value = volumescale.Value - 0.1;
		}

		protected virtual void OnMorebuttonClicked(object sender, System.EventArgs e)
		{
			volumescale.Value = volumescale.Value + 0.1;
		}

		protected virtual void OnVolumescaleValueChanged(object sender, System.EventArgs e)
		{
			VolumeChanged(volumescale.Value);
		}

		protected virtual void OnFocusOutEvent(object o, Gtk.FocusOutEventArgs args)
		{
			this.Hide();
		}



	}
}
