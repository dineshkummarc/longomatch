// ------------------------------------------------------------------------------
//  <autogenerated>
//      This code was generated by a tool.
//      
// 
//      Changes to this file may cause incorrect behavior and will be lost if 
//      the code is regenerated.
//  </autogenerated>
// ------------------------------------------------------------------------------

namespace LongoMatch.Gui.Dialog {
    
    
    public partial class EditPlayerDialog {
        
        private LongoMatch.Gui.Component.PlayerProperties playerproperties1;
        
        private Gtk.Button buttonOk;
        
        protected virtual void Build() {
            Stetic.Gui.Initialize(this);
            // Widget LongoMatch.Gui.Dialog.EditPlayerDialog
            this.Name = "LongoMatch.Gui.Dialog.EditPlayerDialog";
            this.Title = Mono.Unix.Catalog.GetString("Player Details");
            this.Icon = Gdk.Pixbuf.LoadFromResource("longomatch.png");
            this.WindowPosition = ((Gtk.WindowPosition)(4));
            this.Modal = true;
            this.SkipPagerHint = true;
            this.SkipTaskbarHint = true;
            this.HasSeparator = false;
            // Internal child LongoMatch.Gui.Dialog.EditPlayerDialog.VBox
            Gtk.VBox w1 = this.VBox;
            w1.Name = "dialog1_VBox";
            w1.BorderWidth = ((uint)(2));
            // Container child dialog1_VBox.Gtk.Box+BoxChild
            this.playerproperties1 = new LongoMatch.Gui.Component.PlayerProperties();
            this.playerproperties1.Events = ((Gdk.EventMask)(256));
            this.playerproperties1.Name = "playerproperties1";
            w1.Add(this.playerproperties1);
            Gtk.Box.BoxChild w2 = ((Gtk.Box.BoxChild)(w1[this.playerproperties1]));
            w2.Position = 0;
            w2.Expand = false;
            w2.Fill = false;
            // Internal child LongoMatch.Gui.Dialog.EditPlayerDialog.ActionArea
            Gtk.HButtonBox w3 = this.ActionArea;
            w3.Name = "dialog1_ActionArea";
            w3.Spacing = 6;
            w3.BorderWidth = ((uint)(5));
            w3.LayoutStyle = ((Gtk.ButtonBoxStyle)(4));
            // Container child dialog1_ActionArea.Gtk.ButtonBox+ButtonBoxChild
            this.buttonOk = new Gtk.Button();
            this.buttonOk.CanDefault = true;
            this.buttonOk.CanFocus = true;
            this.buttonOk.Name = "buttonOk";
            this.buttonOk.UseStock = true;
            this.buttonOk.UseUnderline = true;
            this.buttonOk.Label = "gtk-ok";
            this.AddActionWidget(this.buttonOk, -5);
            Gtk.ButtonBox.ButtonBoxChild w4 = ((Gtk.ButtonBox.ButtonBoxChild)(w3[this.buttonOk]));
            w4.Expand = false;
            w4.Fill = false;
            if ((this.Child != null)) {
                this.Child.ShowAll();
            }
            this.DefaultWidth = 254;
            this.DefaultHeight = 185;
            this.Show();
        }
    }
}