//
//  Copyright (C) 2011 Andoni Morales Alastruey
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
using System;
using Cairo;
using Gtk;
using Gdk;
using Pango;
using LongoMatch.Common;
using LongoMatch.Store;
using LongoMatch.Store.Templates;

namespace LongoMatch.Gui.Component
{
	public class CategoriesScale: Gtk.DrawingArea
	{
		private const int SECTION_HEIGHT = 30;
		private const int SECTION_WIDTH = 100;
		private const int LINE_WIDTH = 2;
		private double scroll;
		Pango.Layout layout;

		[System.ComponentModel.Category("LongoMatch")]
		[System.ComponentModel.ToolboxItem(true)]
		public CategoriesScale()
		{
			layout =  new Pango.Layout(PangoContext);
			layout.Wrap = Pango.WrapMode.Char;
			layout.Alignment = Pango.Alignment.Left;
		}

		public Categories Categories {
			get;
			set;
		}

		public double Scroll {
			get	{
				return scroll;
			}
			set {
				scroll = value;
				QueueDraw();
			}
		}

		private void DrawCairoText(string text, int x1, int y1) {
			layout.Width = Pango.Units.FromPixels(SECTION_WIDTH - 2);
			layout.Ellipsize = EllipsizeMode.End;
			layout.SetMarkup(text);
			GdkWindow.DrawLayout(Style.TextGC(StateType.Normal),
			                     x1 + 2, y1 ,layout);
		}

		private void DrawCategories(Gdk.Window win) {
			int i = 0;

			if(Categories == null)
				return;

			using(Cairo.Context g = Gdk.CairoHelper.Create(win)) {
				foreach(Category cat in Categories) {
					int y = LINE_WIDTH/2 + i * SECTION_HEIGHT - (int)Scroll;
					CairoUtils.DrawRoundedRectangle(g, 2, y + 3 , Allocation.Width - 3,
					                                SECTION_HEIGHT - 3, SECTION_HEIGHT/7,
					                                CairoUtils.RGBToCairoColor(cat.Color),
					                                CairoUtils.RGBToCairoColor(cat.Color));
					DrawCairoText(cat.Name, 0 + 3, y + SECTION_HEIGHT / 2 - 5);
					i++;
				}
			}
		}

		protected override bool OnExposeEvent(EventExpose evnt)
		{
			DrawCategories(evnt.Window);
			return base.OnExposeEvent(evnt);
		}
	}
}

