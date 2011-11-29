
// This file has been generated by the GUI designer. Do not modify.
namespace LongoMatch.Gui.Component
{
	public partial class CategoryProperties
	{
		private global::Gtk.VBox vbox2;
		private global::Gtk.Frame frame4;
		private global::Gtk.Alignment GtkAlignment1;
		private global::Gtk.Table table2;
		private global::Gtk.ColorButton colorbutton1;
		private global::Gtk.HBox hbox5;
		private global::Gtk.Label hotKeyLabel;
		private global::Gtk.Button changebuton;
		private global::Gtk.Label label1;
		private global::Gtk.Label label4;
		private global::Gtk.Label label6;
		private global::Gtk.Label label7;
		private global::Gtk.Label label8;
		private global::Gtk.Label label9;
		private global::Gtk.SpinButton lagtimebutton;
		private global::Gtk.SpinButton leadtimebutton;
		private global::Gtk.Entry nameentry;
		private global::Gtk.ComboBox sortmethodcombobox;
		private global::Gtk.Label GtkLabel1;
		private global::Gtk.Frame frame3;
		private global::Gtk.Alignment GtkAlignment3;
		private global::Gtk.VBox vbox1;
		private global::Gtk.ScrolledWindow GtkScrolledWindow;
		private global::LongoMatch.Gui.SubCategoriesTreeView subcategoriestreeview1;
		private global::Gtk.HBox hbox3;
		private global::Gtk.Frame frame1;
		private global::Gtk.Alignment GtkAlignment4;
		private global::Gtk.Entry subcatnameentry;
		private global::Gtk.Label GtkLabel3;
		private global::Gtk.Frame frame2;
		private global::Gtk.Alignment GtkAlignment5;
		private global::Gtk.ComboBox subcatcombobox;
		private global::Gtk.Label GtkLabel4;
		private global::Gtk.Button addbutton;
		private global::Gtk.Label GtkLabel5;
        
		protected virtual void Build ()
		{
			global::Stetic.Gui.Initialize (this);
			// Widget LongoMatch.Gui.Component.CategoryProperties
			global::Stetic.BinContainer.Attach (this);
			this.Name = "LongoMatch.Gui.Component.CategoryProperties";
			// Container child LongoMatch.Gui.Component.CategoryProperties.Gtk.Container+ContainerChild
			this.vbox2 = new global::Gtk.VBox ();
			this.vbox2.Name = "vbox2";
			this.vbox2.Spacing = 6;
			// Container child vbox2.Gtk.Box+BoxChild
			this.frame4 = new global::Gtk.Frame ();
			this.frame4.Name = "frame4";
			this.frame4.ShadowType = ((global::Gtk.ShadowType)(1));
			// Container child frame4.Gtk.Container+ContainerChild
			this.GtkAlignment1 = new global::Gtk.Alignment (0F, 0F, 1F, 1F);
			this.GtkAlignment1.Name = "GtkAlignment1";
			this.GtkAlignment1.LeftPadding = ((uint)(12));
			// Container child GtkAlignment1.Gtk.Container+ContainerChild
			this.table2 = new global::Gtk.Table (((uint)(3)), ((uint)(4)), false);
			this.table2.Name = "table2";
			this.table2.RowSpacing = ((uint)(6));
			this.table2.ColumnSpacing = ((uint)(6));
			// Container child table2.Gtk.Table+TableChild
			this.colorbutton1 = new global::Gtk.ColorButton ();
			this.colorbutton1.CanFocus = true;
			this.colorbutton1.Events = ((global::Gdk.EventMask)(784));
			this.colorbutton1.Name = "colorbutton1";
			this.table2.Add (this.colorbutton1);
			global::Gtk.Table.TableChild w1 = ((global::Gtk.Table.TableChild)(this.table2 [this.colorbutton1]));
			w1.LeftAttach = ((uint)(3));
			w1.RightAttach = ((uint)(4));
			w1.XOptions = ((global::Gtk.AttachOptions)(4));
			w1.YOptions = ((global::Gtk.AttachOptions)(0));
			// Container child table2.Gtk.Table+TableChild
			this.hbox5 = new global::Gtk.HBox ();
			this.hbox5.Name = "hbox5";
			this.hbox5.Spacing = 6;
			// Container child hbox5.Gtk.Box+BoxChild
			this.hotKeyLabel = new global::Gtk.Label ();
			this.hotKeyLabel.Name = "hotKeyLabel";
			this.hotKeyLabel.LabelProp = global::Mono.Unix.Catalog.GetString ("none");
			this.hbox5.Add (this.hotKeyLabel);
			global::Gtk.Box.BoxChild w2 = ((global::Gtk.Box.BoxChild)(this.hbox5 [this.hotKeyLabel]));
			w2.Position = 0;
			w2.Expand = false;
			w2.Fill = false;
			// Container child hbox5.Gtk.Box+BoxChild
			this.changebuton = new global::Gtk.Button ();
			this.changebuton.CanFocus = true;
			this.changebuton.Name = "changebuton";
			this.changebuton.UseUnderline = true;
			this.changebuton.Label = global::Mono.Unix.Catalog.GetString ("Change");
			this.hbox5.Add (this.changebuton);
			global::Gtk.Box.BoxChild w3 = ((global::Gtk.Box.BoxChild)(this.hbox5 [this.changebuton]));
			w3.Position = 1;
			w3.Expand = false;
			w3.Fill = false;
			this.table2.Add (this.hbox5);
			global::Gtk.Table.TableChild w4 = ((global::Gtk.Table.TableChild)(this.table2 [this.hbox5]));
			w4.TopAttach = ((uint)(2));
			w4.BottomAttach = ((uint)(3));
			w4.LeftAttach = ((uint)(1));
			w4.RightAttach = ((uint)(2));
			w4.XOptions = ((global::Gtk.AttachOptions)(4));
			w4.YOptions = ((global::Gtk.AttachOptions)(0));
			// Container child table2.Gtk.Table+TableChild
			this.label1 = new global::Gtk.Label ();
			this.label1.Name = "label1";
			this.label1.Xalign = 0F;
			this.label1.LabelProp = global::Mono.Unix.Catalog.GetString ("<b>Name:</b>");
			this.label1.UseMarkup = true;
			this.table2.Add (this.label1);
			global::Gtk.Table.TableChild w5 = ((global::Gtk.Table.TableChild)(this.table2 [this.label1]));
			w5.XOptions = ((global::Gtk.AttachOptions)(4));
			w5.YOptions = ((global::Gtk.AttachOptions)(4));
			// Container child table2.Gtk.Table+TableChild
			this.label4 = new global::Gtk.Label ();
			this.label4.Name = "label4";
			this.label4.Xalign = 0F;
			this.label4.LabelProp = global::Mono.Unix.Catalog.GetString ("<b>Color:</b>    ");
			this.label4.UseMarkup = true;
			this.table2.Add (this.label4);
			global::Gtk.Table.TableChild w6 = ((global::Gtk.Table.TableChild)(this.table2 [this.label4]));
			w6.LeftAttach = ((uint)(2));
			w6.RightAttach = ((uint)(3));
			w6.XOptions = ((global::Gtk.AttachOptions)(4));
			w6.YOptions = ((global::Gtk.AttachOptions)(4));
			// Container child table2.Gtk.Table+TableChild
			this.label6 = new global::Gtk.Label ();
			this.label6.Name = "label6";
			this.label6.Xalign = 0F;
			this.label6.LabelProp = global::Mono.Unix.Catalog.GetString ("<b>HotKey:</b>");
			this.label6.UseMarkup = true;
			this.table2.Add (this.label6);
			global::Gtk.Table.TableChild w7 = ((global::Gtk.Table.TableChild)(this.table2 [this.label6]));
			w7.TopAttach = ((uint)(2));
			w7.BottomAttach = ((uint)(3));
			w7.XOptions = ((global::Gtk.AttachOptions)(4));
			w7.YOptions = ((global::Gtk.AttachOptions)(4));
			// Container child table2.Gtk.Table+TableChild
			this.label7 = new global::Gtk.Label ();
			this.label7.Name = "label7";
			this.label7.Xalign = 0F;
			this.label7.LabelProp = global::Mono.Unix.Catalog.GetString ("<b>Lead time:</b>");
			this.label7.UseMarkup = true;
			this.table2.Add (this.label7);
			global::Gtk.Table.TableChild w8 = ((global::Gtk.Table.TableChild)(this.table2 [this.label7]));
			w8.TopAttach = ((uint)(1));
			w8.BottomAttach = ((uint)(2));
			w8.XOptions = ((global::Gtk.AttachOptions)(4));
			w8.YOptions = ((global::Gtk.AttachOptions)(4));
			// Container child table2.Gtk.Table+TableChild
			this.label8 = new global::Gtk.Label ();
			this.label8.Name = "label8";
			this.label8.Xalign = 0F;
			this.label8.LabelProp = global::Mono.Unix.Catalog.GetString ("<b>Lag time:</b>");
			this.label8.UseMarkup = true;
			this.table2.Add (this.label8);
			global::Gtk.Table.TableChild w9 = ((global::Gtk.Table.TableChild)(this.table2 [this.label8]));
			w9.TopAttach = ((uint)(1));
			w9.BottomAttach = ((uint)(2));
			w9.LeftAttach = ((uint)(2));
			w9.RightAttach = ((uint)(3));
			w9.XOptions = ((global::Gtk.AttachOptions)(4));
			w9.YOptions = ((global::Gtk.AttachOptions)(4));
			// Container child table2.Gtk.Table+TableChild
			this.label9 = new global::Gtk.Label ();
			this.label9.Name = "label9";
			this.label9.Xalign = 0F;
			this.label9.LabelProp = global::Mono.Unix.Catalog.GetString ("<b>Sort Method</b>");
			this.label9.UseMarkup = true;
			this.table2.Add (this.label9);
			global::Gtk.Table.TableChild w10 = ((global::Gtk.Table.TableChild)(this.table2 [this.label9]));
			w10.TopAttach = ((uint)(2));
			w10.BottomAttach = ((uint)(3));
			w10.LeftAttach = ((uint)(2));
			w10.RightAttach = ((uint)(3));
			w10.XOptions = ((global::Gtk.AttachOptions)(4));
			w10.YOptions = ((global::Gtk.AttachOptions)(4));
			// Container child table2.Gtk.Table+TableChild
			this.lagtimebutton = new global::Gtk.SpinButton (0, 100, 1);
			this.lagtimebutton.CanFocus = true;
			this.lagtimebutton.Name = "lagtimebutton";
			this.lagtimebutton.Adjustment.PageIncrement = 1;
			this.lagtimebutton.ClimbRate = 1;
			this.lagtimebutton.Numeric = true;
			this.table2.Add (this.lagtimebutton);
			global::Gtk.Table.TableChild w11 = ((global::Gtk.Table.TableChild)(this.table2 [this.lagtimebutton]));
			w11.TopAttach = ((uint)(1));
			w11.BottomAttach = ((uint)(2));
			w11.LeftAttach = ((uint)(3));
			w11.RightAttach = ((uint)(4));
			w11.YOptions = ((global::Gtk.AttachOptions)(4));
			// Container child table2.Gtk.Table+TableChild
			this.leadtimebutton = new global::Gtk.SpinButton (0, 100, 1);
			this.leadtimebutton.CanFocus = true;
			this.leadtimebutton.Name = "leadtimebutton";
			this.leadtimebutton.Adjustment.PageIncrement = 1;
			this.leadtimebutton.ClimbRate = 1;
			this.leadtimebutton.Numeric = true;
			this.table2.Add (this.leadtimebutton);
			global::Gtk.Table.TableChild w12 = ((global::Gtk.Table.TableChild)(this.table2 [this.leadtimebutton]));
			w12.TopAttach = ((uint)(1));
			w12.BottomAttach = ((uint)(2));
			w12.LeftAttach = ((uint)(1));
			w12.RightAttach = ((uint)(2));
			w12.XOptions = ((global::Gtk.AttachOptions)(4));
			w12.YOptions = ((global::Gtk.AttachOptions)(4));
			// Container child table2.Gtk.Table+TableChild
			this.nameentry = new global::Gtk.Entry ();
			this.nameentry.CanFocus = true;
			this.nameentry.Name = "nameentry";
			this.nameentry.IsEditable = true;
			this.nameentry.InvisibleChar = '●';
			this.table2.Add (this.nameentry);
			global::Gtk.Table.TableChild w13 = ((global::Gtk.Table.TableChild)(this.table2 [this.nameentry]));
			w13.LeftAttach = ((uint)(1));
			w13.RightAttach = ((uint)(2));
			w13.YOptions = ((global::Gtk.AttachOptions)(4));
			// Container child table2.Gtk.Table+TableChild
			this.sortmethodcombobox = global::Gtk.ComboBox.NewText ();
			this.sortmethodcombobox.AppendText (global::Mono.Unix.Catalog.GetString ("Sort by name"));
			this.sortmethodcombobox.AppendText (global::Mono.Unix.Catalog.GetString ("Sort by start time"));
			this.sortmethodcombobox.AppendText (global::Mono.Unix.Catalog.GetString ("Sort by stop time"));
			this.sortmethodcombobox.AppendText (global::Mono.Unix.Catalog.GetString ("Sort by duration"));
			this.sortmethodcombobox.Name = "sortmethodcombobox";
			this.sortmethodcombobox.Active = 3;
			this.table2.Add (this.sortmethodcombobox);
			global::Gtk.Table.TableChild w14 = ((global::Gtk.Table.TableChild)(this.table2 [this.sortmethodcombobox]));
			w14.TopAttach = ((uint)(2));
			w14.BottomAttach = ((uint)(3));
			w14.LeftAttach = ((uint)(3));
			w14.RightAttach = ((uint)(4));
			w14.XOptions = ((global::Gtk.AttachOptions)(4));
			w14.YOptions = ((global::Gtk.AttachOptions)(0));
			this.GtkAlignment1.Add (this.table2);
			this.frame4.Add (this.GtkAlignment1);
			this.GtkLabel1 = new global::Gtk.Label ();
			this.GtkLabel1.Name = "GtkLabel1";
			this.GtkLabel1.LabelProp = global::Mono.Unix.Catalog.GetString ("<b>Properties</b>");
			this.GtkLabel1.UseMarkup = true;
			this.frame4.LabelWidget = this.GtkLabel1;
			this.vbox2.Add (this.frame4);
			global::Gtk.Box.BoxChild w17 = ((global::Gtk.Box.BoxChild)(this.vbox2 [this.frame4]));
			w17.Position = 0;
			w17.Expand = false;
			w17.Fill = false;
			// Container child vbox2.Gtk.Box+BoxChild
			this.frame3 = new global::Gtk.Frame ();
			this.frame3.Name = "frame3";
			this.frame3.ShadowType = ((global::Gtk.ShadowType)(1));
			// Container child frame3.Gtk.Container+ContainerChild
			this.GtkAlignment3 = new global::Gtk.Alignment (0F, 0F, 1F, 1F);
			this.GtkAlignment3.Name = "GtkAlignment3";
			this.GtkAlignment3.LeftPadding = ((uint)(12));
			// Container child GtkAlignment3.Gtk.Container+ContainerChild
			this.vbox1 = new global::Gtk.VBox ();
			this.vbox1.Name = "vbox1";
			this.vbox1.Spacing = 6;
			// Container child vbox1.Gtk.Box+BoxChild
			this.GtkScrolledWindow = new global::Gtk.ScrolledWindow ();
			this.GtkScrolledWindow.Name = "GtkScrolledWindow";
			this.GtkScrolledWindow.ShadowType = ((global::Gtk.ShadowType)(1));
			// Container child GtkScrolledWindow.Gtk.Container+ContainerChild
			this.subcategoriestreeview1 = new global::LongoMatch.Gui.SubCategoriesTreeView ();
			this.subcategoriestreeview1.CanFocus = true;
			this.subcategoriestreeview1.Name = "subcategoriestreeview1";
			this.GtkScrolledWindow.Add (this.subcategoriestreeview1);
			this.vbox1.Add (this.GtkScrolledWindow);
			global::Gtk.Box.BoxChild w19 = ((global::Gtk.Box.BoxChild)(this.vbox1 [this.GtkScrolledWindow]));
			w19.Position = 0;
			// Container child vbox1.Gtk.Box+BoxChild
			this.hbox3 = new global::Gtk.HBox ();
			this.hbox3.Name = "hbox3";
			this.hbox3.Spacing = 6;
			// Container child hbox3.Gtk.Box+BoxChild
			this.frame1 = new global::Gtk.Frame ();
			this.frame1.Name = "frame1";
			this.frame1.ShadowType = ((global::Gtk.ShadowType)(0));
			// Container child frame1.Gtk.Container+ContainerChild
			this.GtkAlignment4 = new global::Gtk.Alignment (0F, 0F, 1F, 1F);
			this.GtkAlignment4.Name = "GtkAlignment4";
			this.GtkAlignment4.LeftPadding = ((uint)(12));
			// Container child GtkAlignment4.Gtk.Container+ContainerChild
			this.subcatnameentry = new global::Gtk.Entry ();
			this.subcatnameentry.CanFocus = true;
			this.subcatnameentry.Name = "subcatnameentry";
			this.subcatnameentry.IsEditable = true;
			this.subcatnameentry.InvisibleChar = '•';
			this.GtkAlignment4.Add (this.subcatnameentry);
			this.frame1.Add (this.GtkAlignment4);
			this.GtkLabel3 = new global::Gtk.Label ();
			this.GtkLabel3.Name = "GtkLabel3";
			this.GtkLabel3.LabelProp = global::Mono.Unix.Catalog.GetString ("<b>Subcategory name</b>");
			this.GtkLabel3.UseMarkup = true;
			this.frame1.LabelWidget = this.GtkLabel3;
			this.hbox3.Add (this.frame1);
			global::Gtk.Box.BoxChild w22 = ((global::Gtk.Box.BoxChild)(this.hbox3 [this.frame1]));
			w22.Position = 0;
			w22.Expand = false;
			w22.Fill = false;
			// Container child hbox3.Gtk.Box+BoxChild
			this.frame2 = new global::Gtk.Frame ();
			this.frame2.Name = "frame2";
			this.frame2.ShadowType = ((global::Gtk.ShadowType)(0));
			// Container child frame2.Gtk.Container+ContainerChild
			this.GtkAlignment5 = new global::Gtk.Alignment (0F, 0F, 1F, 1F);
			this.GtkAlignment5.Name = "GtkAlignment5";
			this.GtkAlignment5.LeftPadding = ((uint)(12));
			// Container child GtkAlignment5.Gtk.Container+ContainerChild
			this.subcatcombobox = new global::Gtk.ComboBox ();
			this.subcatcombobox.Name = "subcatcombobox";
			this.GtkAlignment5.Add (this.subcatcombobox);
			this.frame2.Add (this.GtkAlignment5);
			this.GtkLabel4 = new global::Gtk.Label ();
			this.GtkLabel4.Name = "GtkLabel4";
			this.GtkLabel4.LabelProp = global::Mono.Unix.Catalog.GetString ("<b>Subcategory type</b>");
			this.GtkLabel4.UseMarkup = true;
			this.frame2.LabelWidget = this.GtkLabel4;
			this.hbox3.Add (this.frame2);
			global::Gtk.Box.BoxChild w25 = ((global::Gtk.Box.BoxChild)(this.hbox3 [this.frame2]));
			w25.Position = 1;
			// Container child hbox3.Gtk.Box+BoxChild
			this.addbutton = new global::Gtk.Button ();
			this.addbutton.TooltipMarkup = "Add this subcategory";
			this.addbutton.Sensitive = false;
			this.addbutton.CanFocus = true;
			this.addbutton.Name = "addbutton";
			this.addbutton.UseUnderline = true;
			// Container child addbutton.Gtk.Container+ContainerChild
			global::Gtk.Alignment w26 = new global::Gtk.Alignment (0.5F, 0.5F, 0F, 0F);
			// Container child GtkAlignment.Gtk.Container+ContainerChild
			global::Gtk.HBox w27 = new global::Gtk.HBox ();
			w27.Spacing = 2;
			// Container child GtkHBox.Gtk.Container+ContainerChild
			global::Gtk.Image w28 = new global::Gtk.Image ();
			w28.Pixbuf = global::Stetic.IconLoader.LoadIcon (this, "gtk-add", global::Gtk.IconSize.Menu);
			w27.Add (w28);
			// Container child GtkHBox.Gtk.Container+ContainerChild
			global::Gtk.Label w30 = new global::Gtk.Label ();
			w30.LabelProp = global::Mono.Unix.Catalog.GetString ("_Add subcategory");
			w30.UseUnderline = true;
			w27.Add (w30);
			w26.Add (w27);
			this.addbutton.Add (w26);
			this.hbox3.Add (this.addbutton);
			global::Gtk.Box.BoxChild w34 = ((global::Gtk.Box.BoxChild)(this.hbox3 [this.addbutton]));
			w34.Position = 2;
			w34.Expand = false;
			w34.Fill = false;
			this.vbox1.Add (this.hbox3);
			global::Gtk.Box.BoxChild w35 = ((global::Gtk.Box.BoxChild)(this.vbox1 [this.hbox3]));
			w35.Position = 1;
			w35.Expand = false;
			w35.Fill = false;
			this.GtkAlignment3.Add (this.vbox1);
			this.frame3.Add (this.GtkAlignment3);
			this.GtkLabel5 = new global::Gtk.Label ();
			this.GtkLabel5.Name = "GtkLabel5";
			this.GtkLabel5.LabelProp = global::Mono.Unix.Catalog.GetString ("<b>Subcategories</b>");
			this.GtkLabel5.UseMarkup = true;
			this.frame3.LabelWidget = this.GtkLabel5;
			this.vbox2.Add (this.frame3);
			global::Gtk.Box.BoxChild w38 = ((global::Gtk.Box.BoxChild)(this.vbox2 [this.frame3]));
			w38.Position = 1;
			this.Add (this.vbox2);
			if ((this.Child != null)) {
				this.Child.ShowAll ();
			}
			this.Show ();
			this.sortmethodcombobox.Changed += new global::System.EventHandler (this.OnSortmethodcomboboxChanged);
			this.nameentry.Changed += new global::System.EventHandler (this.OnNameentryChanged);
			this.changebuton.Clicked += new global::System.EventHandler (this.OnChangebutonClicked);
			this.colorbutton1.ColorSet += new global::System.EventHandler (this.OnColorbutton1ColorSet);
			this.subcatcombobox.Changed += new global::System.EventHandler (this.OnSubcatcomboboxChanged);
			this.addbutton.Clicked += new global::System.EventHandler (this.OnAddbuttonClicked);
		}
	}
}