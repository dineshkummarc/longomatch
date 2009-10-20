// 
//  Copyright (C) 2007-2009 Andoni Morales Alastruey
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
using System.Collections.Generic;
using Gtk;
using LongoMatch.DB;
using LongoMatch.TimeNodes;

namespace LongoMatch.Gui.Dialog
{
	
	
	public partial class PlayersSelectionDialog : Gtk.Dialog
	{
		TeamTemplate template;
		List<CheckButton> checkButtonsList;
		
		public PlayersSelectionDialog()
		{
			this.Build();
			checkButtonsList = new List<CheckButton>();
		}
		
		public void SetPlayersInfo (TeamTemplate template ){	
			int playersCount;
			CheckButton button;
			Player player;
			
			if (this.template != null)
				return;
			
			this.template = template;
			playersCount = template.PlayersCount;			
		
			table1.NColumns =(uint)(playersCount/10);
			table1.NRows =(uint) 10; 
			
			for( int i=0;i<playersCount;i++){
				player = template.GetPlayer(i);
				button = new CheckButton();
				button.Label = player.Name;
				button.Name = i.ToString();
				button.Show();
				
				uint row_top =(uint) (i%table1.NRows);
				uint row_bottom = (uint) row_top+1 ;
				uint col_left = (uint) i/table1.NRows;
				uint col_right = (uint) col_left+1 ;
					
				table1.Attach(button,col_left,col_right,row_top,row_bottom);
				checkButtonsList.Add(button);
			}
		}
		
		public List<int> PlayersChecked{
			set{				
				for (int i=0; i<checkButtonsList.Count; i++)
						checkButtonsList[i].Active = value.Contains(i);								
			}
			get{
				List<int> playersList = new List<int>();
				for (int i=0; i<checkButtonsList.Count; i++){
					if (checkButtonsList[i].Active){
						playersList.Add(i);
					}
				}					
				return playersList;				
			}				
		}	
	}
}