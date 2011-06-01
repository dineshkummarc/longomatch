// 
//  Copyright (C) 2010 Andoni Morales Alastruey
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

using Gdk;
using Gtk;
using LongoMatch.Common;
using LongoMatch.Handlers;
using LongoMatch.Store;
using Mono.Unix;
using System;
using System.Collections.Generic;

namespace LongoMatch.Gui.Component
{


	public abstract class ListTreeViewBase:TreeView
	{
		// Plays menu
		protected Menu menu, teamMenu;
		protected MenuItem local;
		protected MenuItem visitor;
		protected MenuItem noTeam;
		protected MenuItem tag;
		protected MenuItem delete;
		protected MenuItem addPLN;
		protected MenuItem deleteKeyFrame;
		protected MenuItem snapshot;
		protected MenuItem name;
		protected MenuItem players;
		protected MenuItem localPlayers;
		protected MenuItem visitorPlayers;
		
		
		protected Gtk.CellRendererText nameCell;
		protected Gtk.TreeViewColumn nameColumn;
		protected String[] teams_name;
		protected bool editing;
		protected bool projectIsLive;

		protected const string LOCAL_TEAM = "Local Team";
		protected const string VISITOR_TEAM = "Visitor Team";	
		
		public event TimeNodeChangedHandler TimeNodeChanged;
		public event TimeNodeSelectedHandler TimeNodeSelected;
		public event TimeNodeDeletedHandler TimeNodeDeleted;
		public event PlayListNodeAddedHandler PlayListNodeAdded;
		public event SnapshotSeriesHandler SnapshotSeriesEvent;
		public event PlayersTaggedHandler PlayersTagged;
		public event TagPlayHandler TagPlay;

		public ListTreeViewBase ()
		{
			Selection.Mode = SelectionMode.Multiple;
			Selection.SelectFunction = SelectFunction;
			RowActivated += new RowActivatedHandler(OnTreeviewRowActivated);
			HeadersVisible = false;
			
			SetMenu();
			ProjectIsLive = false;
			PlayListLoaded = false;

			teams_name = new String[3];
			teams_name[(int)Team.NONE] = Catalog.GetString(Catalog.GetString("None"));
			teams_name[(int)Team.LOCAL] = Catalog.GetString(Catalog.GetString(LOCAL_TEAM));
			teams_name[(int)Team.VISITOR] = Catalog.GetString(Catalog.GetString(VISITOR_TEAM));
			
			
			nameColumn = new Gtk.TreeViewColumn();
			nameColumn.Title = "Name";
			nameCell = new Gtk.CellRendererText();
			nameCell.Edited += OnNameCellEdited;
			Gtk.CellRendererPixbuf miniatureCell = new Gtk.CellRendererPixbuf();
			nameColumn.PackStart(nameCell, true);
			nameColumn.PackEnd(miniatureCell, true);

			nameColumn.SetCellDataFunc(miniatureCell, new Gtk.TreeCellDataFunc(RenderMiniature));
			nameColumn.SetCellDataFunc(nameCell, new Gtk.TreeCellDataFunc(RenderName));

			AppendColumn(nameColumn);

		}

		public bool ProjectIsLive{
			set{
				projectIsLive = value;
				addPLN.Visible = !projectIsLive;
				snapshot.Visible = !projectIsLive;
			}
		}
		
		public bool Colors{
			get;
			set;
		}
		
		public String LocalTeam {
			set{
				Label l1 = (local.Children[0] as Label);
				Label l2 = (localPlayers.Children[0] as Label);
				if (value == "")
					l1.Text = l2.Text = Catalog.GetString(LOCAL_TEAM);
				else {
					l1.Text = l2.Text = value;
				}
				teams_name[(int)Team.LOCAL] = l1.Text; 
			}
		}
		
		public string VisitorTeam {
			set{
				Label l1 = (visitor.Children[0] as Label);
				Label l2 = (visitorPlayers.Children[0] as Label);
				if (value == "")
					l1.Text = l2.Text = Catalog.GetString(VISITOR_TEAM);
				else 
					l1.Text = l2.Text = value;
				teams_name[(int)Team.VISITOR] = l1.Text; 
			}
		}

		public bool PlayListLoaded {
			set {
				addPLN.Sensitive = value;
			}
		}
		
		protected void EmitTimeNodeChanged(TimeNode tn, object o){
			if (TimeNodeChanged != null)
				TimeNodeChanged(tn, o);
		}
		
		protected void SetMenu() {
			Menu playersMenu;
			MenuItem team;
			
			teamMenu = new Menu();
			local = new MenuItem(Catalog.GetString(LOCAL_TEAM));
			visitor = new MenuItem(Catalog.GetString(VISITOR_TEAM));
			noTeam = new MenuItem(Catalog.GetString("No Team"));
			teamMenu .Append(local);
			teamMenu .Append(visitor);
			teamMenu .Append(noTeam);

			playersMenu = new Menu();
			localPlayers = new MenuItem(Catalog.GetString(LOCAL_TEAM));
			visitorPlayers = new MenuItem(Catalog.GetString(VISITOR_TEAM));
			playersMenu.Append(localPlayers);
			playersMenu.Append(visitorPlayers);

			menu = new Menu();
			
			name = new MenuItem(Catalog.GetString("Edit"));
			team = new MenuItem(Catalog.GetString("Team Selection"));
			team.Submenu = teamMenu;
			tag = new MenuItem(Catalog.GetString("Add tag"));
			players = new MenuItem(Catalog.GetString("Tag player"));
			players.Submenu = playersMenu;
			delete = new MenuItem(Catalog.GetString("Delete"));
			deleteKeyFrame = new MenuItem(Catalog.GetString("Delete key frame"));
			addPLN = new MenuItem(Catalog.GetString("Add to playlist"));
			addPLN.Sensitive=false;
			snapshot = new MenuItem(Catalog.GetString("Export to PGN images"));

			menu.Append(name);
			menu.Append(tag);
			menu.Append(players);
			menu.Append(team);
			menu.Append(addPLN);
			menu.Append(delete);
			menu.Append(deleteKeyFrame);
			menu.Append(snapshot);

			name.Activated += OnEdit;
			tag.Activated += OnTag;
			local.Activated += OnTeamSelection;
			visitor.Activated += OnTeamSelection;
			noTeam.Activated += OnTeamSelection;
			localPlayers.Activated += OnLocalPlayers;
			visitorPlayers.Activated += OnVisitorPlayers;
			addPLN.Activated += OnAdded;
			delete.Activated += OnDeleted;
			deleteKeyFrame.Activated += OnDeleteKeyFrame;
			snapshot.Activated += OnSnapshot;
			menu.ShowAll();
		}		

		protected void MultiSelectMenu (bool enabled){
			name.Sensitive = !enabled;
			snapshot.Sensitive = !enabled;
			players.Sensitive = !enabled;
			tag.Sensitive = !enabled;
		}
		
		protected object GetValueFromPath(TreePath path){
			Gtk.TreeIter iter;
			Model.GetIter(out iter, path);
			return Model.GetValue(iter,0);					
		}	
	
		protected void RenderMiniature(Gtk.TreeViewColumn column, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
		{
			var item = model.GetValue(iter, 0);
			var c = cell as CellRendererPixbuf;

			if (item is Play){
				c.Pixbuf = (item as Play).Miniature;
				if (Colors) {
					c.CellBackgroundGdk = (item as Play).Category.Color;
				} else{ 
					c.CellBackground = "white";
				}
			}
			else if (item is Player){
				c.Pixbuf= (item as Player).Photo;
				c.CellBackground = "white";
			}
			else {
				c.Pixbuf = null;
				c.CellBackground = "white";
			}
		}
		
		protected void RenderName(Gtk.TreeViewColumn column, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
		{
			object o = model.GetValue(iter, 0);
			var c = cell as CellRendererText;
			
			/* Handle special case in which we replace the text in the cell by the name of the TimeNode
			 * We need to check if we are editing and only change it for the path that's currently beeing edited */
			if (editing && Selection.IterIsSelected(iter)){
				if (o is Player)
					c.Markup = (o as Player).Name;
				else
					c.Markup = (o as TimeNode).Name;
				return;
			} 			
			
			if (o is Play){
				var mtn = o as Play;
				if (Colors) {
					Color col = mtn.Category.Color;
					c.CellBackgroundGdk = col;
					c.BackgroundGdk = col;
				} else{ 
					c.Background = "white";
					c.CellBackground = "white";
				}
				c.Markup = mtn.ToString(teams_name[(int)mtn.Team]);
			}else if (o is Player) {
				c.Background = "white";
				c.CellBackground = "white";
				c.Markup = String.Format("{0} ({1})", (o as Player).Name, Model.IterNChildren(iter));
			}else if (o is Category) {
				c.Background = "white";
				c.CellBackground = "white";
				c.Markup = String.Format("{0} ({1})", (o as TimeNode).Name, Model.IterNChildren(iter));
			}
		}	

		protected virtual void OnNameCellEdited(object o, Gtk.EditedArgs args)
		{
			Gtk.TreeIter iter;
			object item;
			
			Model.GetIter(out iter, new Gtk.TreePath(args.Path));
			item = this.Model.GetValue(iter,0);
			
			if (item is TimeNode){
				(item as TimeNode).Name = args.NewText;
				EmitTimeNodeChanged((item as TimeNode), args.NewText);
			}else if (item is Player){
				(item as Player).Name = args.NewText;
			}
			editing = false;
			nameCell.Editable=false;
			
		}

		protected virtual void OnTreeviewRowActivated(object o, Gtk.RowActivatedArgs args)
		{
			Gtk.TreeIter iter;
			this.Model.GetIter(out iter, args.Path);
			object item = this.Model.GetValue(iter, 0);
			
			if (!(item is Play))
				return;

			if (TimeNodeSelected != null && !projectIsLive)
				this.TimeNodeSelected(item as Play);
		}

		protected void OnDeleted(object obj, EventArgs args) {
			List <Play> playsList = new List<Play>();
			List <TreeIter> iters = new List<TreeIter>();
			TreePath[] paths = Selection.GetSelectedRows();
			
			/* Get the iter for all of the paths first, because the path changes
			 * each time a row is deleted */
			foreach (var path in paths) {
				TreeIter iter;
				Model.GetIter(out iter, path);
				playsList.Add((Play)Model.GetValue(iter, 0));
				iters.Add(iter);
			}
			/* Delete all the iters now */
			for (int i=0; i< iters.Count; i++){
				TreeIter iter = iters[i];
				(Model as TreeStore).Remove(ref iter);
			}
			if (TimeNodeDeleted != null)
				TimeNodeDeleted(playsList);
		}

		protected void OnDeleteKeyFrame(object obj, EventArgs args) {
			MessageDialog md = new MessageDialog((Gtk.Window)Toplevel,
			                                     DialogFlags.Modal,
			                                     MessageType.Question,
			                                     ButtonsType.YesNo,
			                                     false,
			                                     Catalog.GetString("Do you want to delete the key frame for this play?")
			                                    );
			if (md.Run() == (int)ResponseType.Yes){
				TreePath[] paths = Selection.GetSelectedRows();
				for (int i=0; i<paths.Length; i++){	
					Play tNode = (Play)GetValueFromPath(paths[i]);
					tNode.Drawings.Clear();
				}
				// Refresh the thumbnails
				QueueDraw();
			}
			md.Destroy();
		}

		protected virtual void OnEdit(object obj, EventArgs args) {
			TreePath[] paths = Selection.GetSelectedRows();
			
			editing = true;
			nameCell.Editable = true;
			SetCursor(paths[0],  nameColumn, true);
		}

		protected void OnTeamSelection(object obj, EventArgs args) {
			MenuItem sender = (MenuItem)obj;
			Team team = Team.NONE;
			if (sender == local)
				team = Team.LOCAL;
			else if (sender == visitor)
				team = Team.VISITOR;
			else if (sender == noTeam)
				team = Team.NONE;
			
			TreePath[] paths = Selection.GetSelectedRows();
			for (int i=0; i<paths.Length; i++){	
					Play tNode = (Play)GetValueFromPath(paths[i]);
					tNode.Team = team;
			}
		}

		protected void OnAdded(object obj, EventArgs args) {
			if (PlayListNodeAdded != null){
				TreePath[] paths = Selection.GetSelectedRows();
				for (int i=0; i<paths.Length; i++){	
					Play tNode = (Play)GetValueFromPath(paths[i]);
					PlayListNodeAdded(tNode);
				}
			}
		}
		
		protected void OnTag (object obj, EventArgs args){
			if (TagPlay != null)
				TagPlay((Play)GetValueFromPath(Selection.GetSelectedRows()[0]));
		}

		protected void OnSnapshot(object obj, EventArgs args) {
			if (SnapshotSeriesEvent != null)
				SnapshotSeriesEvent((Play)GetValueFromPath(Selection.GetSelectedRows()[0]));
		}

		protected virtual void OnLocalPlayers(object o, EventArgs args) {
			if (PlayersTagged != null)
				PlayersTagged((Play)GetValueFromPath(Selection.GetSelectedRows()[0]), Team.LOCAL);
		}

		protected virtual void OnVisitorPlayers(object o, EventArgs args) {
			if (PlayersTagged != null)
				PlayersTagged((Play)GetValueFromPath(Selection.GetSelectedRows()[0]), Team.VISITOR);
		}
		
		protected abstract bool SelectFunction(TreeSelection selection, TreeModel model, TreePath path, bool selected);
	}
}
