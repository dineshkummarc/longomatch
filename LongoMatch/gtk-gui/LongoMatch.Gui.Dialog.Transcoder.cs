// ------------------------------------------------------------------------------
//  <autogenerated>
//      This code was generated by a tool.
//      Mono Runtime Version: 2.0.50727.42
// 
//      Changes to this file may cause incorrect behavior and will be lost if 
//      the code is regenerated.
//  </autogenerated>
// ------------------------------------------------------------------------------

namespace LongoMatch.Gui.Dialog {
    
    
    public partial class Transcoder {
        
        private Gtk.VBox vbox2;
        
        private Gtk.FileChooserButton filechooserbutton1;
        
        private Gtk.HBox hbox1;
        
        private Gtk.Label label1;
        
        private Gtk.SpinButton spinbutton1;
        
        private Gtk.HBox hbox2;
        
        private Gtk.Label label2;
        
        private Gtk.SpinButton spinbutton2;
        
        private Gtk.Button buttonCancel;
        
        private Gtk.Button buttonOk;
        
        protected virtual void Build() {
            Stetic.Gui.Initialize(this);
            // Widget LongoMatch.Gui.Dialog.Transcoder
            this.Name = "LongoMatch.Gui.Dialog.Transcoder";
            this.Title = Mono.Unix.Catalog.GetString("Transcoder");
            this.Icon = Gdk.Pixbuf.LoadFromResource("lgmlogo");
            this.WindowPosition = ((Gtk.WindowPosition)(4));
            this.Modal = true;
            this.Gravity = ((Gdk.Gravity)(5));
            this.SkipPagerHint = true;
            this.SkipTaskbarHint = true;
            this.HasSeparator = false;
            // Internal child LongoMatch.Gui.Dialog.Transcoder.VBox
            Gtk.VBox w1 = this.VBox;
            w1.Name = "dialog1_VBox";
            w1.BorderWidth = ((uint)(2));
            // Container child dialog1_VBox.Gtk.Box+BoxChild
            this.vbox2 = new Gtk.VBox();
            this.vbox2.Name = "vbox2";
            this.vbox2.Spacing = 6;
            // Container child vbox2.Gtk.Box+BoxChild
            this.filechooserbutton1 = new Gtk.FileChooserButton(Mono.Unix.Catalog.GetString("Seleccione un archivo"), ((Gtk.FileChooserAction)(0)));
            this.filechooserbutton1.Name = "filechooserbutton1";
            this.filechooserbutton1.ShowHidden = true;
            this.vbox2.Add(this.filechooserbutton1);
            Gtk.Box.BoxChild w2 = ((Gtk.Box.BoxChild)(this.vbox2[this.filechooserbutton1]));
            w2.Position = 0;
            w2.Expand = false;
            w2.Fill = false;
            // Container child vbox2.Gtk.Box+BoxChild
            this.hbox1 = new Gtk.HBox();
            this.hbox1.Name = "hbox1";
            this.hbox1.Spacing = 6;
            // Container child hbox1.Gtk.Box+BoxChild
            this.label1 = new Gtk.Label();
            this.label1.Name = "label1";
            this.label1.LabelProp = Mono.Unix.Catalog.GetString("Audio bitrate (kbps)");
            this.hbox1.Add(this.label1);
            Gtk.Box.BoxChild w3 = ((Gtk.Box.BoxChild)(this.hbox1[this.label1]));
            w3.Position = 0;
            w3.Fill = false;
            // Container child hbox1.Gtk.Box+BoxChild
            this.spinbutton1 = new Gtk.SpinButton(32, 320, 1);
            this.spinbutton1.CanFocus = true;
            this.spinbutton1.Name = "spinbutton1";
            this.spinbutton1.Adjustment.PageIncrement = 10;
            this.spinbutton1.ClimbRate = 1;
            this.spinbutton1.Numeric = true;
            this.spinbutton1.Value = 128;
            this.hbox1.Add(this.spinbutton1);
            Gtk.Box.BoxChild w4 = ((Gtk.Box.BoxChild)(this.hbox1[this.spinbutton1]));
            w4.Position = 1;
            w4.Expand = false;
            w4.Fill = false;
            this.vbox2.Add(this.hbox1);
            Gtk.Box.BoxChild w5 = ((Gtk.Box.BoxChild)(this.vbox2[this.hbox1]));
            w5.Position = 1;
            w5.Expand = false;
            // Container child vbox2.Gtk.Box+BoxChild
            this.hbox2 = new Gtk.HBox();
            this.hbox2.Name = "hbox2";
            this.hbox2.Spacing = 6;
            // Container child hbox2.Gtk.Box+BoxChild
            this.label2 = new Gtk.Label();
            this.label2.Name = "label2";
            this.label2.LabelProp = Mono.Unix.Catalog.GetString("Video bitrate (kbps)");
            this.hbox2.Add(this.label2);
            Gtk.Box.BoxChild w6 = ((Gtk.Box.BoxChild)(this.hbox2[this.label2]));
            w6.Position = 0;
            // Container child hbox2.Gtk.Box+BoxChild
            this.spinbutton2 = new Gtk.SpinButton(400, 15000, 1);
            this.spinbutton2.CanFocus = true;
            this.spinbutton2.Name = "spinbutton2";
            this.spinbutton2.Adjustment.PageIncrement = 10;
            this.spinbutton2.ClimbRate = 1;
            this.spinbutton2.Numeric = true;
            this.spinbutton2.Value = 2000;
            this.hbox2.Add(this.spinbutton2);
            Gtk.Box.BoxChild w7 = ((Gtk.Box.BoxChild)(this.hbox2[this.spinbutton2]));
            w7.Position = 1;
            w7.Expand = false;
            w7.Fill = false;
            this.vbox2.Add(this.hbox2);
            Gtk.Box.BoxChild w8 = ((Gtk.Box.BoxChild)(this.vbox2[this.hbox2]));
            w8.Position = 2;
            w8.Expand = false;
            w8.Fill = false;
            w1.Add(this.vbox2);
            Gtk.Box.BoxChild w9 = ((Gtk.Box.BoxChild)(w1[this.vbox2]));
            w9.Position = 0;
            w9.Expand = false;
            w9.Fill = false;
            // Internal child LongoMatch.Gui.Dialog.Transcoder.ActionArea
            Gtk.HButtonBox w10 = this.ActionArea;
            w10.Name = "dialog1_ActionArea";
            w10.Spacing = 6;
            w10.BorderWidth = ((uint)(5));
            w10.LayoutStyle = ((Gtk.ButtonBoxStyle)(4));
            // Container child dialog1_ActionArea.Gtk.ButtonBox+ButtonBoxChild
            this.buttonCancel = new Gtk.Button();
            this.buttonCancel.CanDefault = true;
            this.buttonCancel.CanFocus = true;
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.UseStock = true;
            this.buttonCancel.UseUnderline = true;
            this.buttonCancel.Label = "gtk-cancel";
            this.AddActionWidget(this.buttonCancel, -6);
            Gtk.ButtonBox.ButtonBoxChild w11 = ((Gtk.ButtonBox.ButtonBoxChild)(w10[this.buttonCancel]));
            w11.Expand = false;
            w11.Fill = false;
            // Container child dialog1_ActionArea.Gtk.ButtonBox+ButtonBoxChild
            this.buttonOk = new Gtk.Button();
            this.buttonOk.CanDefault = true;
            this.buttonOk.CanFocus = true;
            this.buttonOk.Name = "buttonOk";
            this.buttonOk.UseStock = true;
            this.buttonOk.UseUnderline = true;
            this.buttonOk.Label = "gtk-ok";
            this.AddActionWidget(this.buttonOk, -5);
            Gtk.ButtonBox.ButtonBoxChild w12 = ((Gtk.ButtonBox.ButtonBoxChild)(w10[this.buttonOk]));
            w12.Position = 1;
            w12.Expand = false;
            w12.Fill = false;
            if ((this.Child != null)) {
                this.Child.ShowAll();
            }
            this.DefaultWidth = 400;
            this.DefaultHeight = 192;
            this.Show();
        }
    }
}
