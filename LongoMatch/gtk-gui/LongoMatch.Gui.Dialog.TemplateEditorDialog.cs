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
    
    
    public partial class TemplateEditorDialog {
        
        private LongoMatch.Gui.Component.SectionsPropertiesWidget sectionspropertieswidget3;
        
        private Gtk.Button buttonCancel;
        
        private Gtk.Button buttonOk;
        
        protected virtual void Build() {
            Stetic.Gui.Initialize(this);
            // Widget LongoMatch.Gui.Dialog.TemplateEditorDialog
            this.Name = "LongoMatch.Gui.Dialog.TemplateEditorDialog";
            this.Title = Mono.Unix.Catalog.GetString("Template Editor");
            this.WindowPosition = ((Gtk.WindowPosition)(4));
            this.Modal = true;
            this.DestroyWithParent = true;
            this.Gravity = ((Gdk.Gravity)(5));
            this.SkipPagerHint = true;
            this.SkipTaskbarHint = true;
            this.HasSeparator = false;
            // Internal child LongoMatch.Gui.Dialog.TemplateEditorDialog.VBox
            Gtk.VBox w1 = this.VBox;
            w1.Name = "dialog1_VBox";
            w1.BorderWidth = ((uint)(2));
            // Container child dialog1_VBox.Gtk.Box+BoxChild
            this.sectionspropertieswidget3 = new LongoMatch.Gui.Component.SectionsPropertiesWidget();
            this.sectionspropertieswidget3.Events = ((Gdk.EventMask)(256));
            this.sectionspropertieswidget3.Name = "sectionspropertieswidget3";
            w1.Add(this.sectionspropertieswidget3);
            Gtk.Box.BoxChild w2 = ((Gtk.Box.BoxChild)(w1[this.sectionspropertieswidget3]));
            w2.Position = 0;
            // Internal child LongoMatch.Gui.Dialog.TemplateEditorDialog.ActionArea
            Gtk.HButtonBox w3 = this.ActionArea;
            w3.Name = "dialog1_ActionArea";
            w3.Spacing = 6;
            w3.BorderWidth = ((uint)(5));
            w3.LayoutStyle = ((Gtk.ButtonBoxStyle)(4));
            // Container child dialog1_ActionArea.Gtk.ButtonBox+ButtonBoxChild
            this.buttonCancel = new Gtk.Button();
            this.buttonCancel.CanDefault = true;
            this.buttonCancel.CanFocus = true;
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.UseStock = true;
            this.buttonCancel.UseUnderline = true;
            this.buttonCancel.Label = "gtk-cancel";
            this.AddActionWidget(this.buttonCancel, -6);
            Gtk.ButtonBox.ButtonBoxChild w4 = ((Gtk.ButtonBox.ButtonBoxChild)(w3[this.buttonCancel]));
            w4.Expand = false;
            w4.Fill = false;
            // Container child dialog1_ActionArea.Gtk.ButtonBox+ButtonBoxChild
            this.buttonOk = new Gtk.Button();
            this.buttonOk.CanDefault = true;
            this.buttonOk.CanFocus = true;
            this.buttonOk.Name = "buttonOk";
            this.buttonOk.UseStock = true;
            this.buttonOk.UseUnderline = true;
            this.buttonOk.Label = "gtk-apply";
            this.AddActionWidget(this.buttonOk, -10);
            Gtk.ButtonBox.ButtonBoxChild w5 = ((Gtk.ButtonBox.ButtonBoxChild)(w3[this.buttonOk]));
            w5.Position = 1;
            w5.Expand = false;
            w5.Fill = false;
            if ((this.Child != null)) {
                this.Child.ShowAll();
            }
            this.DefaultWidth = 1066;
            this.DefaultHeight = 473;
            this.Show();
        }
    }
}