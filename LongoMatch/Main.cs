// Main.cs
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
//Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
//


using System;
using System.IO;
using Gtk;
using LongoMatch.Common;
using LongoMatch.DB;
using LongoMatch.Gui;
using LongoMatch.IO;
using LongoMatch.Store.Templates;
using Mono.Unix;

namespace LongoMatch

{

	class MainClass
	{
		private static DataBase db;
		private static string baseDirectory;
		private static string homeDirectory;
		private static string configDirectory;
		private const string WIN32_CONFIG_FILE = "longomatch.conf";

		public static void Main(string[] args)
		{
			SetupBaseDir();

			//Iniciamos la internalización
			Catalog.Init(Constants.SOFTWARE_NAME.ToLower(),RelativeToPrefix("share/locale"));

			//Iniciamos la aplicación
			Application.Init();

			GLib.ExceptionManager.UnhandledException += new GLib.UnhandledExceptionHandler(OnException);

			LongoMatch.Video.Player.GstPlayer.InitBackend("");

			//Comprobamos los archivos de inicio
			CheckDirs();
			CheckFiles();

			//Iniciamos la base de datos
			db = new DataBase(Path.Combine(DBDir(),Constants.DB_FILE));

			//Check for previous database
			CheckOldFiles();

			try {
				MainWindow win = new MainWindow();
				win.Show();
				Application.Run();
			} catch (Exception ex) {
				ProcessExecutionError(ex);
			}
		}

		public static string RelativeToPrefix(string relativePath) {
			return System.IO.Path.Combine(baseDirectory, relativePath);
		}

		public static string HomeDir() {
			return homeDirectory;
		}

		public static string PlayListDir() {
			return System.IO.Path.Combine(homeDirectory, "playlists");
		}

		public static string SnapshotsDir() {
			return System.IO.Path.Combine(homeDirectory, "snapshots");
		}

		public static string TemplatesDir() {
			return System.IO.Path.Combine(configDirectory, "templates");
		}

		public static string VideosDir() {
			return System.IO.Path.Combine(homeDirectory, "videos");
		}

		public static string TempVideosDir() {
			return System.IO.Path.Combine(configDirectory, "temp");
		}

		public static string ImagesDir() {
			return RelativeToPrefix("share/longomatch/images");
		}

		public static string DBDir() {
			return System.IO.Path.Combine(configDirectory, "db");
		}

		public static void CheckDirs() {
			if (!System.IO.Directory.Exists(homeDirectory))
				System.IO.Directory.CreateDirectory(homeDirectory);
			if (!System.IO.Directory.Exists(TemplatesDir()))
				System.IO.Directory.CreateDirectory(TemplatesDir());
			if (!System.IO.Directory.Exists(SnapshotsDir()))
				System.IO.Directory.CreateDirectory(SnapshotsDir());
			if (!System.IO.Directory.Exists(PlayListDir()))
				System.IO.Directory.CreateDirectory(PlayListDir());
			if (!System.IO.Directory.Exists(DBDir()))
				System.IO.Directory.CreateDirectory(DBDir());
			if (!System.IO.Directory.Exists(VideosDir()))
				System.IO.Directory.CreateDirectory(VideosDir());
			if (!System.IO.Directory.Exists(TempVideosDir()))
				System.IO.Directory.CreateDirectory(TempVideosDir());
		}

		public static void CheckFiles() {
			string fConfig;
			fConfig = System.IO.Path.Combine(TemplatesDir(),"default.sct");
			if (!System.IO.File.Exists(fConfig)) {
				CategoriesWriter.CreateNewTemplate("default.sct");
			}

			fConfig = System.IO.Path.Combine(TemplatesDir(),"default.tem");
			if (!System.IO.File.Exists(fConfig)) {
				TeamTemplate tt = TeamTemplate.DefaultTemplate(20);
				tt.Save(fConfig);
			}
		}

		public static void CheckOldFiles() {
			string oldDBFile= System.IO.Path.Combine(homeDirectory, "db/db.yap");
			//We supose that if the conversion as already be done successfully,
			//old DB file has been renamed to db.yap.bak
			if (File.Exists(oldDBFile)) {
				MessageDialog md = new MessageDialog(null,
				                                     DialogFlags.Modal,
				                                     MessageType.Question,
				                                     Gtk.ButtonsType.YesNo,
				                                     Catalog.GetString("Some elements from the previous version (database, templates and/or playlists) have been found.")+"\n"+
				                                     Catalog.GetString("Do you want to import them?"));
				md.Icon=Stetic.IconLoader.LoadIcon(md, "longomatch", Gtk.IconSize.Dialog);
				if (md.Run()==(int)ResponseType.Yes) {
					md.Destroy();
					//Migrator migrator = new Migrator(homeDirectory);
					//migrator.Run();
					//migrator.Destroy();
				}
				else
					md.Destroy();
			}
		}

		public static DataBase DB {
			get {
				return db;
			}
		}
		
		private static void SetupBaseDir() {
			string home;
			
			if (Environment.OSVersion.Platform == PlatformID.Win32NT)
				baseDirectory = System.IO.Path.Combine(System.AppDomain.CurrentDomain.BaseDirectory,"../");
			else
				baseDirectory = System.IO.Path.Combine(System.AppDomain.CurrentDomain.BaseDirectory,"../../");
			
			/* Check for the magic file PORTABLE to check if it's a portable version
			 * and the config goes in the same folder as the binaries */
			if (File.Exists(System.IO.Path.Combine(baseDirectory, Constants.PORTABLE_FILE)))
				home = baseDirectory;
			else
				home = System.Environment.GetFolderPath(Environment.SpecialFolder.Personal);
			
			homeDirectory = System.IO.Path.Combine(home,Constants.SOFTWARE_NAME);
			if (Environment.OSVersion.Platform == PlatformID.Win32NT)
				configDirectory = homeDirectory;
			else
				configDirectory = System.IO.Path.Combine(home,".longomatch");
		}

		private static void OnException(GLib.UnhandledExceptionArgs args) {
			ProcessExecutionError((Exception)args.ExceptionObject);
		}

		private static void ProcessExecutionError(Exception ex) {
			string logFile = Constants.SOFTWARE_NAME + "-" + DateTime.Now +".log";
			string message;

			logFile = logFile.Replace("/","-");
			logFile = logFile.Replace(" ","-");
			logFile = logFile.Replace(":","-");
			logFile = System.IO.Path.Combine(HomeDir(),logFile);

			if (ex.InnerException != null)
				message = String.Format("{0}\n{1}\n{2}\n{3}\n{4}",ex.Message,ex.InnerException.Message,ex.Source,ex.StackTrace,ex.InnerException.StackTrace);
			else
				message = String.Format("{0}\n{1}\n{2}",ex.Message,ex.Source,ex.StackTrace);

			using(StreamWriter s = new StreamWriter(logFile)) {
				s.WriteLine(message);
				s.WriteLine("\n\n\nStackTrace:");
				s.WriteLine(System.Environment.StackTrace);
			}
			//TODO Add bug reports link
			MessagePopup.PopupMessage(null, MessageType.Error,
			                          Catalog.GetString("The application has finished with an unexpected error.")+"\n"+
			                          Catalog.GetString("A log has been saved at: ")+logFile+ "\n"+
			                          Catalog.GetString("Please, fill a bug report "));

			Application.Quit();
		}
	}
}
