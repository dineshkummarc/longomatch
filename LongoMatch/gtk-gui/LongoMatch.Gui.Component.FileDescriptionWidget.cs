﻿// ------------------------------------------------------------------------------
//  <autogenerated>
//      This code was generated by a tool.
//      Mono Runtime Version: 2.0.50727.42
// 
//      Changes to this file may cause incorrect behavior and will be lost if 
//      the code is regenerated.
//  </autogenerated>
// ------------------------------------------------------------------------------

namespace LongoMatch.Gui.Component {
    
    
    public partial class FileDescriptionWidget {
        
        private Gtk.VBox vbox2;
        
        private Gtk.Table table1;
        
        private Gtk.SpinButton bitratespinbutton;
        
        private Gtk.HBox hbox2;
        
        private Gtk.ComboBox combobox1;
        
        private Gtk.Button editbutton;
        
        private Gtk.HBox hbox4;
        
        private Gtk.Entry fileEntry;
        
        private Gtk.Button openbutton;
        
        private Gtk.HBox hbox5;
        
        private Gtk.Entry dateEntry;
        
        private Gtk.Button calendarbutton;
        
        private Gtk.Label label1;
        
        private Gtk.Label label2;
        
        private Gtk.Label label3;
        
        private Gtk.Label label4;
        
        private Gtk.Label label5;
        
        private Gtk.Label label6;
        
        private Gtk.Label label8;
        
        private Gtk.Label label9;
        
        private Gtk.SpinButton localSpinButton;
        
        private Gtk.Entry localTeamEntry;
        
        private Gtk.SpinButton visitorSpinButton;
        
        private Gtk.Entry visitorTeamEntry;
        
        protected virtual void Build() {
            Stetic.Gui.Initialize(this);
            // Widget LongoMatch.Gui.Component.FileDescriptionWidget
            Stetic.BinContainer.Attach(this);
            this.Name = "LongoMatch.Gui.Component.FileDescriptionWidget";
            // Container child LongoMatch.Gui.Component.FileDescriptionWidget.Gtk.Container+ContainerChild
            this.vbox2 = new Gtk.VBox();
            this.vbox2.Name = "vbox2";
            this.vbox2.Spacing = 6;
            // Container child vbox2.Gtk.Box+BoxChild
            this.table1 = new Gtk.Table(((uint)(8)), ((uint)(2)), false);
            this.table1.Name = "table1";
            this.table1.RowSpacing = ((uint)(6));
            this.table1.ColumnSpacing = ((uint)(6));
            // Container child table1.Gtk.Table+TableChild
            this.bitratespinbutton = new Gtk.SpinButton(1000, 8000, 1);
            this.bitratespinbutton.CanFocus = true;
            this.bitratespinbutton.Name = "bitratespinbutton";
            this.bitratespinbutton.Adjustment.PageIncrement = 10;
            this.bitratespinbutton.ClimbRate = 1;
            this.bitratespinbutton.Numeric = true;
            this.bitratespinbutton.Value = 4000;
            this.table1.Add(this.bitratespinbutton);
            Gtk.Table.TableChild w1 = ((Gtk.Table.TableChild)(this.table1[this.bitratespinbutton]));
            w1.TopAttach = ((uint)(7));
            w1.BottomAttach = ((uint)(8));
            w1.LeftAttach = ((uint)(1));
            w1.RightAttach = ((uint)(2));
            w1.XOptions = ((Gtk.AttachOptions)(0));
            w1.YOptions = ((Gtk.AttachOptions)(4));
            // Container child table1.Gtk.Table+TableChild
            this.hbox2 = new Gtk.HBox();
            this.hbox2.Name = "hbox2";
            this.hbox2.Spacing = 6;
            // Container child hbox2.Gtk.Box+BoxChild
            this.combobox1 = Gtk.ComboBox.NewText();
            this.combobox1.Name = "combobox1";
            this.combobox1.Active = 0;
            this.hbox2.Add(this.combobox1);
            Gtk.Box.BoxChild w2 = ((Gtk.Box.BoxChild)(this.hbox2[this.combobox1]));
            w2.Position = 0;
            // Container child hbox2.Gtk.Box+BoxChild
            this.editbutton = new Gtk.Button();
            this.editbutton.CanFocus = true;
            this.editbutton.Name = "editbutton";
            this.editbutton.UseStock = true;
            this.editbutton.UseUnderline = true;
            this.editbutton.Label = "gtk-edit";
            this.hbox2.Add(this.editbutton);
            Gtk.Box.BoxChild w3 = ((Gtk.Box.BoxChild)(this.hbox2[this.editbutton]));
            w3.Position = 1;
            w3.Expand = false;
            w3.Fill = false;
            this.table1.Add(this.hbox2);
            Gtk.Table.TableChild w4 = ((Gtk.Table.TableChild)(this.table1[this.hbox2]));
            w4.TopAttach = ((uint)(4));
            w4.BottomAttach = ((uint)(5));
            w4.LeftAttach = ((uint)(1));
            w4.RightAttach = ((uint)(2));
            w4.XOptions = ((Gtk.AttachOptions)(4));
            w4.YOptions = ((Gtk.AttachOptions)(4));
            // Container child table1.Gtk.Table+TableChild
            this.hbox4 = new Gtk.HBox();
            this.hbox4.Name = "hbox4";
            this.hbox4.Spacing = 6;
            // Container child hbox4.Gtk.Box+BoxChild
            this.fileEntry = new Gtk.Entry();
            this.fileEntry.CanFocus = true;
            this.fileEntry.Name = "fileEntry";
            this.fileEntry.IsEditable = false;
            this.fileEntry.InvisibleChar = '●';
            this.hbox4.Add(this.fileEntry);
            Gtk.Box.BoxChild w5 = ((Gtk.Box.BoxChild)(this.hbox4[this.fileEntry]));
            w5.Position = 0;
            // Container child hbox4.Gtk.Box+BoxChild
            this.openbutton = new Gtk.Button();
            this.openbutton.CanFocus = true;
            this.openbutton.Name = "openbutton";
            this.openbutton.UseStock = true;
            this.openbutton.UseUnderline = true;
            this.openbutton.Label = "gtk-open";
            this.hbox4.Add(this.openbutton);
            Gtk.Box.BoxChild w6 = ((Gtk.Box.BoxChild)(this.hbox4[this.openbutton]));
            w6.Position = 1;
            w6.Expand = false;
            this.table1.Add(this.hbox4);
            Gtk.Table.TableChild w7 = ((Gtk.Table.TableChild)(this.table1[this.hbox4]));
            w7.TopAttach = ((uint)(6));
            w7.BottomAttach = ((uint)(7));
            w7.LeftAttach = ((uint)(1));
            w7.RightAttach = ((uint)(2));
            w7.XOptions = ((Gtk.AttachOptions)(4));
            w7.YOptions = ((Gtk.AttachOptions)(4));
            // Container child table1.Gtk.Table+TableChild
            this.hbox5 = new Gtk.HBox();
            this.hbox5.Name = "hbox5";
            // Container child hbox5.Gtk.Box+BoxChild
            this.dateEntry = new Gtk.Entry();
            this.dateEntry.CanFocus = true;
            this.dateEntry.Name = "dateEntry";
            this.dateEntry.IsEditable = false;
            this.dateEntry.InvisibleChar = '●';
            this.hbox5.Add(this.dateEntry);
            Gtk.Box.BoxChild w8 = ((Gtk.Box.BoxChild)(this.hbox5[this.dateEntry]));
            w8.Position = 0;
            // Container child hbox5.Gtk.Box+BoxChild
            this.calendarbutton = new Gtk.Button();
            this.calendarbutton.CanFocus = true;
            this.calendarbutton.Name = "calendarbutton";
            this.calendarbutton.UseUnderline = true;
            // Container child calendarbutton.Gtk.Container+ContainerChild
            Gtk.Alignment w9 = new Gtk.Alignment(0.5F, 0.5F, 0F, 0F);
            // Container child GtkAlignment.Gtk.Container+ContainerChild
            Gtk.HBox w10 = new Gtk.HBox();
            w10.Spacing = 2;
            // Container child GtkHBox.Gtk.Container+ContainerChild
            Gtk.Image w11 = new Gtk.Image();
            w11.Pixbuf = Stetic.IconLoader.LoadIcon(this, "stock_calendar", Gtk.IconSize.Button, 20);
            w10.Add(w11);
            // Container child GtkHBox.Gtk.Container+ContainerChild
            Gtk.Label w13 = new Gtk.Label();
            w13.LabelProp = Mono.Unix.Catalog.GetString("_Calendar");
            w13.UseUnderline = true;
            w10.Add(w13);
            w9.Add(w10);
            this.calendarbutton.Add(w9);
            this.hbox5.Add(this.calendarbutton);
            Gtk.Box.BoxChild w17 = ((Gtk.Box.BoxChild)(this.hbox5[this.calendarbutton]));
            w17.Position = 1;
            w17.Expand = false;
            w17.Fill = false;
            this.table1.Add(this.hbox5);
            Gtk.Table.TableChild w18 = ((Gtk.Table.TableChild)(this.table1[this.hbox5]));
            w18.TopAttach = ((uint)(5));
            w18.BottomAttach = ((uint)(6));
            w18.LeftAttach = ((uint)(1));
            w18.RightAttach = ((uint)(2));
            w18.YOptions = ((Gtk.AttachOptions)(1));
            // Container child table1.Gtk.Table+TableChild
            this.label1 = new Gtk.Label();
            this.label1.Name = "label1";
            this.label1.LabelProp = Mono.Unix.Catalog.GetString("Video Bitrate:");
            this.table1.Add(this.label1);
            Gtk.Table.TableChild w19 = ((Gtk.Table.TableChild)(this.table1[this.label1]));
            w19.TopAttach = ((uint)(7));
            w19.BottomAttach = ((uint)(8));
            w19.XOptions = ((Gtk.AttachOptions)(4));
            w19.YOptions = ((Gtk.AttachOptions)(4));
            // Container child table1.Gtk.Table+TableChild
            this.label2 = new Gtk.Label();
            this.label2.Name = "label2";
            this.label2.LabelProp = Mono.Unix.Catalog.GetString("Visitor Team:");
            this.table1.Add(this.label2);
            Gtk.Table.TableChild w20 = ((Gtk.Table.TableChild)(this.table1[this.label2]));
            w20.TopAttach = ((uint)(1));
            w20.BottomAttach = ((uint)(2));
            w20.XOptions = ((Gtk.AttachOptions)(4));
            w20.YOptions = ((Gtk.AttachOptions)(4));
            // Container child table1.Gtk.Table+TableChild
            this.label3 = new Gtk.Label();
            this.label3.Name = "label3";
            this.label3.LabelProp = Mono.Unix.Catalog.GetString("Local Goals:");
            this.table1.Add(this.label3);
            Gtk.Table.TableChild w21 = ((Gtk.Table.TableChild)(this.table1[this.label3]));
            w21.TopAttach = ((uint)(2));
            w21.BottomAttach = ((uint)(3));
            w21.XOptions = ((Gtk.AttachOptions)(4));
            w21.YOptions = ((Gtk.AttachOptions)(4));
            // Container child table1.Gtk.Table+TableChild
            this.label4 = new Gtk.Label();
            this.label4.Name = "label4";
            this.label4.LabelProp = Mono.Unix.Catalog.GetString("Visitor Goals:");
            this.table1.Add(this.label4);
            Gtk.Table.TableChild w22 = ((Gtk.Table.TableChild)(this.table1[this.label4]));
            w22.TopAttach = ((uint)(3));
            w22.BottomAttach = ((uint)(4));
            w22.XOptions = ((Gtk.AttachOptions)(4));
            w22.YOptions = ((Gtk.AttachOptions)(4));
            // Container child table1.Gtk.Table+TableChild
            this.label5 = new Gtk.Label();
            this.label5.Name = "label5";
            this.label5.LabelProp = Mono.Unix.Catalog.GetString("Date:");
            this.table1.Add(this.label5);
            Gtk.Table.TableChild w23 = ((Gtk.Table.TableChild)(this.table1[this.label5]));
            w23.TopAttach = ((uint)(5));
            w23.BottomAttach = ((uint)(6));
            w23.XOptions = ((Gtk.AttachOptions)(4));
            w23.YOptions = ((Gtk.AttachOptions)(4));
            // Container child table1.Gtk.Table+TableChild
            this.label6 = new Gtk.Label();
            this.label6.Name = "label6";
            this.label6.LabelProp = Mono.Unix.Catalog.GetString("File:");
            this.table1.Add(this.label6);
            Gtk.Table.TableChild w24 = ((Gtk.Table.TableChild)(this.table1[this.label6]));
            w24.TopAttach = ((uint)(6));
            w24.BottomAttach = ((uint)(7));
            w24.XOptions = ((Gtk.AttachOptions)(4));
            w24.YOptions = ((Gtk.AttachOptions)(4));
            // Container child table1.Gtk.Table+TableChild
            this.label8 = new Gtk.Label();
            this.label8.Name = "label8";
            this.label8.LabelProp = Mono.Unix.Catalog.GetString("Local Team:");
            this.table1.Add(this.label8);
            Gtk.Table.TableChild w25 = ((Gtk.Table.TableChild)(this.table1[this.label8]));
            w25.XOptions = ((Gtk.AttachOptions)(4));
            w25.YOptions = ((Gtk.AttachOptions)(4));
            // Container child table1.Gtk.Table+TableChild
            this.label9 = new Gtk.Label();
            this.label9.Name = "label9";
            this.label9.LabelProp = Mono.Unix.Catalog.GetString("Template:");
            this.table1.Add(this.label9);
            Gtk.Table.TableChild w26 = ((Gtk.Table.TableChild)(this.table1[this.label9]));
            w26.TopAttach = ((uint)(4));
            w26.BottomAttach = ((uint)(5));
            w26.XOptions = ((Gtk.AttachOptions)(4));
            w26.YOptions = ((Gtk.AttachOptions)(4));
            // Container child table1.Gtk.Table+TableChild
            this.localSpinButton = new Gtk.SpinButton(0, 100, 1);
            this.localSpinButton.CanFocus = true;
            this.localSpinButton.Name = "localSpinButton";
            this.localSpinButton.Adjustment.PageIncrement = 10;
            this.localSpinButton.ClimbRate = 1;
            this.localSpinButton.Numeric = true;
            this.table1.Add(this.localSpinButton);
            Gtk.Table.TableChild w27 = ((Gtk.Table.TableChild)(this.table1[this.localSpinButton]));
            w27.TopAttach = ((uint)(2));
            w27.BottomAttach = ((uint)(3));
            w27.LeftAttach = ((uint)(1));
            w27.RightAttach = ((uint)(2));
            w27.XOptions = ((Gtk.AttachOptions)(1));
            w27.YOptions = ((Gtk.AttachOptions)(1));
            // Container child table1.Gtk.Table+TableChild
            this.localTeamEntry = new Gtk.Entry();
            this.localTeamEntry.CanFocus = true;
            this.localTeamEntry.Name = "localTeamEntry";
            this.localTeamEntry.IsEditable = true;
            this.localTeamEntry.InvisibleChar = '●';
            this.table1.Add(this.localTeamEntry);
            Gtk.Table.TableChild w28 = ((Gtk.Table.TableChild)(this.table1[this.localTeamEntry]));
            w28.LeftAttach = ((uint)(1));
            w28.RightAttach = ((uint)(2));
            // Container child table1.Gtk.Table+TableChild
            this.visitorSpinButton = new Gtk.SpinButton(0, 100, 1);
            this.visitorSpinButton.CanFocus = true;
            this.visitorSpinButton.Name = "visitorSpinButton";
            this.visitorSpinButton.Adjustment.PageIncrement = 10;
            this.visitorSpinButton.ClimbRate = 1;
            this.visitorSpinButton.Numeric = true;
            this.table1.Add(this.visitorSpinButton);
            Gtk.Table.TableChild w29 = ((Gtk.Table.TableChild)(this.table1[this.visitorSpinButton]));
            w29.TopAttach = ((uint)(3));
            w29.BottomAttach = ((uint)(4));
            w29.LeftAttach = ((uint)(1));
            w29.RightAttach = ((uint)(2));
            w29.XOptions = ((Gtk.AttachOptions)(1));
            w29.YOptions = ((Gtk.AttachOptions)(1));
            // Container child table1.Gtk.Table+TableChild
            this.visitorTeamEntry = new Gtk.Entry();
            this.visitorTeamEntry.CanFocus = true;
            this.visitorTeamEntry.Name = "visitorTeamEntry";
            this.visitorTeamEntry.IsEditable = true;
            this.visitorTeamEntry.InvisibleChar = '●';
            this.table1.Add(this.visitorTeamEntry);
            Gtk.Table.TableChild w30 = ((Gtk.Table.TableChild)(this.table1[this.visitorTeamEntry]));
            w30.TopAttach = ((uint)(1));
            w30.BottomAttach = ((uint)(2));
            w30.LeftAttach = ((uint)(1));
            w30.RightAttach = ((uint)(2));
            w30.XOptions = ((Gtk.AttachOptions)(4));
            this.vbox2.Add(this.table1);
            Gtk.Box.BoxChild w31 = ((Gtk.Box.BoxChild)(this.vbox2[this.table1]));
            w31.Position = 0;
            this.Add(this.vbox2);
            if ((this.Child != null)) {
                this.Child.ShowAll();
            }
            this.editbutton.Hide();
            this.label1.Hide();
            this.Show();
            this.calendarbutton.Clicked += new System.EventHandler(this.OnCalendarbuttonClicked);
            this.openbutton.Clicked += new System.EventHandler(this.OnOpenbuttonClicked);
            this.editbutton.Clicked += new System.EventHandler(this.OnEditbuttonClicked);
        }
    }
}
