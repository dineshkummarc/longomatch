
// This file has been generated by the GUI designer. Do not modify.
namespace LongoMatch.Gui.Component
{
	public partial class TagsTreeWidget
	{
		private global::Gtk.VBox vbox1;
		private global::Gtk.ScrolledWindow GtkScrolledWindow;
		private global::LongoMatch.Gui.Component.TagsTreeView treeview;
		private global::Gtk.VBox tagsvbox;
		private global::Gtk.HBox hbox1;
		private global::Gtk.ComboBox tagscombobox;
		private global::Gtk.Button AddFilterButton;
		private global::Gtk.HBox hbox2;
		private global::Gtk.ComboBox filtercombobox;
        
		protected virtual void Build ()
		{
			global::Stetic.Gui.Initialize (this);
			// Widget LongoMatch.Gui.Component.TagsTreeWidget
			global::Stetic.BinContainer.Attach (this);
			this.Name = "LongoMatch.Gui.Component.TagsTreeWidget";
			// Container child LongoMatch.Gui.Component.TagsTreeWidget.Gtk.Container+ContainerChild
			this.vbox1 = new global::Gtk.VBox ();
			this.vbox1.Name = "vbox1";
			this.vbox1.Spacing = 6;
			// Container child vbox1.Gtk.Box+BoxChild
			this.GtkScrolledWindow = new global::Gtk.ScrolledWindow ();
			this.GtkScrolledWindow.Name = "GtkScrolledWindow";
			this.GtkScrolledWindow.ShadowType = ((global::Gtk.ShadowType)(1));
			// Container child GtkScrolledWindow.Gtk.Container+ContainerChild
			this.treeview = new global::LongoMatch.Gui.Component.TagsTreeView ();
			this.treeview.CanFocus = true;
			this.treeview.Name = "treeview";
			this.treeview.Colors = false;
			this.GtkScrolledWindow.Add (this.treeview);
			this.vbox1.Add (this.GtkScrolledWindow);
			global::Gtk.Box.BoxChild w2 = ((global::Gtk.Box.BoxChild)(this.vbox1 [this.GtkScrolledWindow]));
			w2.Position = 0;
			// Container child vbox1.Gtk.Box+BoxChild
			this.tagsvbox = new global::Gtk.VBox ();
			this.tagsvbox.Name = "tagsvbox";
			this.tagsvbox.Spacing = 6;
			this.vbox1.Add (this.tagsvbox);
			global::Gtk.Box.BoxChild w3 = ((global::Gtk.Box.BoxChild)(this.vbox1 [this.tagsvbox]));
			w3.Position = 1;
			w3.Expand = false;
			w3.Fill = false;
			// Container child vbox1.Gtk.Box+BoxChild
			this.hbox1 = new global::Gtk.HBox ();
			this.hbox1.Name = "hbox1";
			this.hbox1.Spacing = 6;
			// Container child hbox1.Gtk.Box+BoxChild
			this.tagscombobox = global::Gtk.ComboBox.NewText ();
			this.tagscombobox.Name = "tagscombobox";
			this.hbox1.Add (this.tagscombobox);
			global::Gtk.Box.BoxChild w4 = ((global::Gtk.Box.BoxChild)(this.hbox1 [this.tagscombobox]));
			w4.Position = 0;
			// Container child hbox1.Gtk.Box+BoxChild
			this.AddFilterButton = new global::Gtk.Button ();
			this.AddFilterButton.CanFocus = true;
			this.AddFilterButton.Name = "AddFilterButton";
			this.AddFilterButton.UseUnderline = true;
			// Container child AddFilterButton.Gtk.Container+ContainerChild
			global::Gtk.Alignment w5 = new global::Gtk.Alignment (0.5F, 0.5F, 0F, 0F);
			// Container child GtkAlignment.Gtk.Container+ContainerChild
			global::Gtk.HBox w6 = new global::Gtk.HBox ();
			w6.Spacing = 2;
			// Container child GtkHBox.Gtk.Container+ContainerChild
			global::Gtk.Image w7 = new global::Gtk.Image ();
			w7.Pixbuf = global::Stetic.IconLoader.LoadIcon (this, "gtk-add", global::Gtk.IconSize.Menu);
			w6.Add (w7);
			// Container child GtkHBox.Gtk.Container+ContainerChild
			global::Gtk.Label w9 = new global::Gtk.Label ();
			w9.LabelProp = global::Mono.Unix.Catalog.GetString ("Add Filter");
			w9.UseUnderline = true;
			w6.Add (w9);
			w5.Add (w6);
			this.AddFilterButton.Add (w5);
			this.hbox1.Add (this.AddFilterButton);
			global::Gtk.Box.BoxChild w13 = ((global::Gtk.Box.BoxChild)(this.hbox1 [this.AddFilterButton]));
			w13.Position = 1;
			w13.Expand = false;
			w13.Fill = false;
			this.vbox1.Add (this.hbox1);
			global::Gtk.Box.BoxChild w14 = ((global::Gtk.Box.BoxChild)(this.vbox1 [this.hbox1]));
			w14.Position = 2;
			w14.Expand = false;
			w14.Fill = false;
			// Container child vbox1.Gtk.Box+BoxChild
			this.hbox2 = new global::Gtk.HBox ();
			this.hbox2.Name = "hbox2";
			this.hbox2.Spacing = 6;
			// Container child hbox2.Gtk.Box+BoxChild
			this.filtercombobox = global::Gtk.ComboBox.NewText ();
			this.filtercombobox.Name = "filtercombobox";
			this.hbox2.Add (this.filtercombobox);
			global::Gtk.Box.BoxChild w15 = ((global::Gtk.Box.BoxChild)(this.hbox2 [this.filtercombobox]));
			w15.Position = 0;
			this.vbox1.Add (this.hbox2);
			global::Gtk.Box.BoxChild w16 = ((global::Gtk.Box.BoxChild)(this.vbox1 [this.hbox2]));
			w16.Position = 3;
			w16.Expand = false;
			w16.Fill = false;
			this.Add (this.vbox1);
			if ((this.Child != null)) {
				this.Child.ShowAll ();
			}
			this.Hide ();
			this.AddFilterButton.Clicked += new global::System.EventHandler (this.OnAddFilter);
			this.filtercombobox.Changed += new global::System.EventHandler (this.OnFiltercomboboxChanged);
		}
	}
}
