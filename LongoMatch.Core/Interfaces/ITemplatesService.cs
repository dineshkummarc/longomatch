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
using System.Collections.Generic;
using LongoMatch.Store;
using LongoMatch.Store.Templates;

namespace LongoMatch.Interfaces
{
	public interface ITemplatesService
	{
		ITemplateProvider<T, U> GetTemplateProvider<T, U>() where T: ITemplate<U>;
		
		ITemplateWidget<T, U> GetTemplateEditor<T, U>() where T: ITemplate<U>;
		
		ITemplateProvider<SubCategoryTemplate, string> SubCategoriesTemplateProvider {get;}
		
		ITemplateProvider<TeamTemplate, Player> TeamTemplateProvider {get;}

		ITemplateProvider<Categories, Category> CategoriesTemplateProvider {get;}
		
		List<PlayerSubCategory> PlayerSubcategories {get;}
		
		List<TeamSubCategory> TeamSubcategories {get;}
	}
}

