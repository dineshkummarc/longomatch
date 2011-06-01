// PlayListTreeView.cs
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
using Gdk;
using Mono.Unix;
using LongoMatch.Store;
using LongoMatch.Playlist;
using LongoMatch.Handlers;
using LongoMatch.Gui.Dialog;

namespace LongoMatch.Gui.Component
{


	[System.ComponentModel.Category("LongoMatch")]
	[System.ComponentModel.ToolboxItem(true)]
	public class PlayListTreeView : Gtk.TreeView
	{
		private Menu menu;
		private MenuItem setRate;
		private ListStore ls;
		private PlayList playlist;
		private PlayListPlay loadedTimeNode = null; //The play currently loaded in the player
		private PlayListPlay selectedTimeNode = null; //The play selected in the tree
		private TreeIter selectedIter;

		public event ApplyCurrentRateHandler ApplyCurrentRate;


		public PlayListTreeView() {

			this.HeadersVisible = false;

			ls = new ListStore(typeof(PlayListPlay));
			this.Model = ls;

			menu = new Menu();
			MenuItem title = new MenuItem(Catalog.GetString("Edit Title"));
			title.Activated += new EventHandler(OnTitle);
			title.Show();
			MenuItem delete = new MenuItem(Catalog.GetString("Delete"));
			delete.Activated += new EventHandler(OnDelete);
			delete.Show();
			setRate = new MenuItem(Catalog.GetString("Apply current play rate"));
			setRate.Activated += new EventHandler(OnApplyRate);
			setRate.Show();
			menu.Append(title);
			menu.Append(setRate);
			menu.Append(delete);


			Gtk.TreeViewColumn nameColumn = new Gtk.TreeViewColumn();
			nameColumn.Title = Catalog.GetString("Name");
			Gtk.CellRendererText nameCell = new Gtk.CellRendererText();
			nameColumn.PackStart(nameCell, true);
			nameColumn.SetCellDataFunc(nameCell, new Gtk.TreeCellDataFunc(RenderName));
			this.AppendColumn(nameColumn);
		}

		public PlayList PlayList {
			set {
				this.playlist = value;
			}
		}

		public PlayListPlay LoadedPlay {
			set {
				loadedTimeNode = value;
				this.QueueDraw();
			}
		}

		~PlayListTreeView()
		{
		}


		protected override bool OnButtonPressEvent(EventButton evnt)
		{
			if ((evnt.Type == EventType.ButtonPress) && (evnt.Button == 3))
			{
				TreePath path;
				GetPathAtPos((int)evnt.X,(int)evnt.Y,out path);
				if (path!=null) {
					ListStore list = ((ListStore)Model);
					Model.GetIter(out selectedIter,path);
					selectedTimeNode = (PlayListPlay)(list.GetValue(selectedIter,0));
					setRate.Sensitive = selectedTimeNode == loadedTimeNode;
					menu.Popup();
				}
			}
			return base.OnButtonPressEvent(evnt);
		}

		protected void OnTitle(object o, EventArgs args) {
			EntryDialog ed = new EntryDialog();
			ed.Title = Catalog.GetString("Edit Title");
			ed.Text = selectedTimeNode.Name;
			if (ed.Run() == (int)ResponseType.Ok) {
				selectedTimeNode.Name = ed.Text;
				this.QueueDraw();
			}
			ed.Destroy();
		}

		protected void OnDelete(object obj, EventArgs args) {
			ListStore list = ((ListStore)Model);
			playlist.Remove(selectedTimeNode);
			list.Remove(ref selectedIter);
		}

		protected void OnApplyRate(object obj, EventArgs args) {
			if (ApplyCurrentRate != null)
				ApplyCurrentRate(selectedTimeNode);
		}

		private void RenderName(Gtk.TreeViewColumn column, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
		{
			PlayListPlay tNode = (PlayListPlay) model.GetValue(iter, 0);
			(cell as Gtk.CellRendererText).Text = 	Catalog.GetString("Title")+": "+tNode.Name +"\n"+
			                                       Catalog.GetString("Start")+": "+tNode.Start.ToMSecondsString()+Catalog.GetString(" sec")+"\n"+
			                                       Catalog.GetString("Duration")+": "+tNode.Duration.ToMSecondsString()+Catalog.GetString(" sec")+"\n"+
			                                       Catalog.GetString("Play Rate")+": "+tNode.Rate.ToString();
			if (!tNode.Valid) {
				(cell as Gtk.CellRendererText).Foreground = "red";
				(cell as Gtk.CellRendererText).Text += "\n"+Catalog.GetString("File not found")+": "+tNode.MediaFile.FilePath;
			}
			else if (tNode == loadedTimeNode)
				(cell as Gtk.CellRendererText).Foreground = "blue";
			else
				(cell as Gtk.CellRendererText).Foreground = "black";

		}

		protected override bool OnKeyPressEvent (Gdk.EventKey evnt)
		{
			return false;
		}		
	}
}
