// XMLReader.cs
//
//  Copyright (C) 2007-2009 Andoni Morales Alastruey
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
//

using System;
using System.IO;
using System.Xml;

namespace LongoMatch.IO
{


	public class XMLReader
	{
		private XmlDocument configXml;

		#region Constructors
		public XMLReader(string filePath)
		{
			configXml = new XmlDocument();
			if(!File.Exists(filePath)) {
				//manejar el error!!!
			}

			configXml.Load(filePath);
		}
		#endregion
		#region Public methods

		public string GetStringValue(string section, string clave)
		{
			XmlNode n;
			n = configXml.SelectSingleNode(section + "/add[@key=\"" + clave + "\"]");
			if(n!=null)
				return (string)(n.Attributes["value"].Value);
			else return null;
		}

		public int GetIntValue(string section, string clave)
		{
			XmlNode n;
			n = configXml.SelectSingleNode(section + "/add[@key=\"" + clave + "\"]");
			if(n != null) {
				object result = n.Attributes["value"].Value;
				return int.Parse(result.ToString());
			}
			else return -1;

		}
		public bool GetBoolValue(string section, string clave) {
			XmlNode n;
			n = configXml.SelectSingleNode(section + "/add[@key=\"" + clave + "\"]");

			if(n != null) {
				object result = n.Attributes["value"].Value;

				return bool.Parse(result.ToString());
			}
			else return false;
		}
		public ushort GetUShortValue(string section, string clave) {
			XmlNode n;
			n = configXml.SelectSingleNode(section + "/add[@key=\"" + clave + "\"]");
			if(n != null) {
				object result = n.Attributes["value"].Value;
				return ushort.Parse(result.ToString());
			}
			else return 254;
		}
		#endregion
	}
}
