
// This file has been generated by the GUI designer. Do not modify.
namespace LongoMatch.Gui.Dialog
{
	public partial class EditPlayerDialog
	{
		private global::LongoMatch.Gui.Component.PlayerProperties playerproperties1;
		private global::Gtk.Button buttonOk;
        
		protected virtual void Build ()
		{
			global::Stetic.Gui.Initialize (this);
			// Widget LongoMatch.Gui.Dialog.EditPlayerDialog
			this.Name = "LongoMatch.Gui.Dialog.EditPlayerDialog";
			this.Title = global::Mono.Unix.Catalog.GetString ("Player Details");
			this.Icon = global::Gdk.Pixbuf.LoadFromResource ("longomatch.png");
			this.WindowPosition = ((global::Gtk.WindowPosition)(4));
			this.Modal = true;
			this.SkipPagerHint = true;
			this.SkipTaskbarHint = true;
			// Internal child LongoMatch.Gui.Dialog.EditPlayerDialog.VBox
			global::Gtk.VBox w1 = this.VBox;
			w1.Name = "dialog1_VBox";
			w1.BorderWidth = ((uint)(2));
			// Container child dialog1_VBox.Gtk.Box+BoxChild
			this.playerproperties1 = new global::LongoMatch.Gui.Component.PlayerProperties ();
			this.playerproperties1.Events = ((global::Gdk.EventMask)(256));
			this.playerproperties1.Name = "playerproperties1";
			w1.Add (this.playerproperties1);
			global::Gtk.Box.BoxChild w2 = ((global::Gtk.Box.BoxChild)(w1 [this.playerproperties1]));
			w2.Position = 0;
			// Internal child LongoMatch.Gui.Dialog.EditPlayerDialog.ActionArea
			global::Gtk.HButtonBox w3 = this.ActionArea;
			w3.Name = "dialog1_ActionArea";
			w3.Spacing = 6;
			w3.BorderWidth = ((uint)(5));
			w3.LayoutStyle = ((global::Gtk.ButtonBoxStyle)(4));
			// Container child dialog1_ActionArea.Gtk.ButtonBox+ButtonBoxChild
			this.buttonOk = new global::Gtk.Button ();
			this.buttonOk.CanDefault = true;
			this.buttonOk.CanFocus = true;
			this.buttonOk.Name = "buttonOk";
			this.buttonOk.UseStock = true;
			this.buttonOk.UseUnderline = true;
			this.buttonOk.Label = "gtk-ok";
			this.AddActionWidget (this.buttonOk, -5);
			global::Gtk.ButtonBox.ButtonBoxChild w4 = ((global::Gtk.ButtonBox.ButtonBoxChild)(w3 [this.buttonOk]));
			w4.Expand = false;
			w4.Fill = false;
			if ((this.Child != null)) {
				this.Child.ShowAll ();
			}
			this.DefaultWidth = 257;
			this.DefaultHeight = 355;
			this.Show ();
		}
	}
}
