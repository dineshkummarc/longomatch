
// This file has been generated by the GUI designer. Do not modify.
namespace LongoMatch.Gui.Component
{
	public partial class GameUnitWidget
	{
		private global::Gtk.HBox gameunitsbox;
		private global::Gtk.Button button1;
        
		protected virtual void Build ()
		{
			global::Stetic.Gui.Initialize (this);
			// Widget LongoMatch.Gui.Component.GameUnitWidget
			global::Stetic.BinContainer.Attach (this);
			this.Name = "LongoMatch.Gui.Component.GameUnitWidget";
			// Container child LongoMatch.Gui.Component.GameUnitWidget.Gtk.Container+ContainerChild
			this.gameunitsbox = new global::Gtk.HBox ();
			this.gameunitsbox.Name = "gameunitsbox";
			this.gameunitsbox.Spacing = 6;
			// Container child gameunitsbox.Gtk.Box+BoxChild
			this.button1 = new global::Gtk.Button ();
			this.button1.CanFocus = true;
			this.button1.Name = "button1";
			this.button1.UseUnderline = true;
			// Container child button1.Gtk.Container+ContainerChild
			global::Gtk.Alignment w1 = new global::Gtk.Alignment (0.5F, 0.5F, 0F, 0F);
			// Container child GtkAlignment.Gtk.Container+ContainerChild
			global::Gtk.HBox w2 = new global::Gtk.HBox ();
			w2.Spacing = 2;
			// Container child GtkHBox.Gtk.Container+ContainerChild
			global::Gtk.Image w3 = new global::Gtk.Image ();
			w2.Add (w3);
			// Container child GtkHBox.Gtk.Container+ContainerChild
			global::Gtk.Label w5 = new global::Gtk.Label ();
			w5.LabelProp = global::Mono.Unix.Catalog.GetString ("GtkButton");
			w5.UseUnderline = true;
			w2.Add (w5);
			w1.Add (w2);
			this.button1.Add (w1);
			this.gameunitsbox.Add (this.button1);
			global::Gtk.Box.BoxChild w9 = ((global::Gtk.Box.BoxChild)(this.gameunitsbox [this.button1]));
			w9.Position = 2;
			w9.Expand = false;
			w9.Fill = false;
			this.Add (this.gameunitsbox);
			if ((this.Child != null)) {
				this.Child.ShowAll ();
			}
			this.Hide ();
		}
	}
}
