// ------------------------------------------------------------------------------
//  <autogenerated>
//      This code was generated by a tool.
//      Mono Runtime Version: 2.0.50727.42
// 
//      Changes to this file may cause incorrect behavior and will be lost if 
//      the code is regenerated.
//  </autogenerated>
// ------------------------------------------------------------------------------

namespace LongoMatch.Widgets.Component {
    
    
    public partial class TreeWidget {
        
        private Gtk.ScrolledWindow scrolledwindow1;
        
        private LongoMatch.Widgets.Component.TimeNodesTreeView treeview;
        
        protected virtual void Build() {
            Stetic.Gui.Initialize(this);
            // Widget LongoMatch.Widgets.Component.TreeWidget
            Stetic.BinContainer.Attach(this);
            this.Name = "LongoMatch.Widgets.Component.TreeWidget";
            // Container child LongoMatch.Widgets.Component.TreeWidget.Gtk.Container+ContainerChild
            this.scrolledwindow1 = new Gtk.ScrolledWindow();
            this.scrolledwindow1.CanFocus = true;
            this.scrolledwindow1.Name = "scrolledwindow1";
            // Container child scrolledwindow1.Gtk.Container+ContainerChild
            Gtk.Viewport w1 = new Gtk.Viewport();
            w1.ShadowType = ((Gtk.ShadowType)(0));
            // Container child GtkViewport.Gtk.Container+ContainerChild
            this.treeview = new LongoMatch.Widgets.Component.TimeNodesTreeView();
            this.treeview.CanFocus = true;
            this.treeview.Name = "treeview";
            this.treeview.HeadersClickable = true;
            w1.Add(this.treeview);
            this.scrolledwindow1.Add(w1);
            this.Add(this.scrolledwindow1);
            if ((this.Child != null)) {
                this.Child.ShowAll();
            }
            this.Show();
            this.treeview.TimeNodeChanged += new LongoMatch.Handlers.TimeNodeChangedHandler(this.OnTimeNodeChanged);
            this.treeview.TimeNodeSelected += new LongoMatch.Handlers.TimeNodeSelectedHandler(this.OnTimeNodeSelected);
            this.treeview.TimeNodeDeleted += new LongoMatch.Handlers.TimeNodeDeletedHandler(this.OnTimeNodeDeleted);
            this.treeview.PlayListNodeAdded += new LongoMatch.Handlers.PlayListNodeAddedHandler(this.OnPlayListNodeAdded);
        }
    }
}
