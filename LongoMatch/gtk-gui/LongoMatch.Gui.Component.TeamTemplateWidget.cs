
// This file has been generated by the GUI designer. Do not modify.
namespace LongoMatch.Gui.Component
{
	public partial class TeamTemplateWidget
	{
		private global::Gtk.HBox hbox1;

		private global::Gtk.ScrolledWindow scrolledwindow2;

		private global::LongoMatch.Gui.Component.PlayerPropertiesTreeView playerpropertiestreeview1;

		protected virtual void Build ()
		{
			global::Stetic.Gui.Initialize (this);
			// Widget LongoMatch.Gui.Component.TeamTemplateWidget
			global::Stetic.BinContainer.Attach (this);
			this.Name = "LongoMatch.Gui.Component.TeamTemplateWidget";
			// Container child LongoMatch.Gui.Component.TeamTemplateWidget.Gtk.Container+ContainerChild
			this.hbox1 = new global::Gtk.HBox ();
			this.hbox1.Name = "hbox1";
			this.hbox1.Spacing = 6;
			// Container child hbox1.Gtk.Box+BoxChild
			this.scrolledwindow2 = new global::Gtk.ScrolledWindow ();
			this.scrolledwindow2.CanFocus = true;
			this.scrolledwindow2.Name = "scrolledwindow2";
			this.scrolledwindow2.ShadowType = ((global::Gtk.ShadowType)(1));
			// Container child scrolledwindow2.Gtk.Container+ContainerChild
			this.playerpropertiestreeview1 = new global::LongoMatch.Gui.Component.PlayerPropertiesTreeView ();
			this.playerpropertiestreeview1.CanFocus = true;
			this.playerpropertiestreeview1.Name = "playerpropertiestreeview1";
			this.scrolledwindow2.Add (this.playerpropertiestreeview1);
			this.hbox1.Add (this.scrolledwindow2);
			global::Gtk.Box.BoxChild w2 = ((global::Gtk.Box.BoxChild)(this.hbox1[this.scrolledwindow2]));
			w2.Position = 0;
			this.Add (this.hbox1);
			if ((this.Child != null)) {
				this.Child.ShowAll ();
			}
			this.Hide ();
			this.playerpropertiestreeview1.PlayerClicked += new global::LongoMatch.Gui.Component.PlayerPropertiesHandler (this.OnPlayerpropertiestreeview1PlayerClicked);
			this.playerpropertiestreeview1.PlayerSelected += new global::LongoMatch.Gui.Component.PlayerPropertiesHandler (this.OnPlayerpropertiestreeview1PlayerSelected);
		}
	}
}