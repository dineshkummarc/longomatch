// 
//  Copyright (C) 2010 Andoni Morales Alastruey
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
using Mono.Unix;
using GLib;
using LongoMatch.Video.Handlers;

namespace LongoMatch.Video.Capturer
{
	
	
	public class FakeCapturer : Gtk.Bin, ICapturer
	{
		public event EllpasedTimeHandler EllapsedTime;
		
		private LiveSourceTimer timer;
		
		public FakeCapturer(): base ()
		{
			timer = new LiveSourceTimer();	
			timer.EllapsedTime += delegate(int ellapsedTime) {
				if (EllapsedTime!= null)
					EllapsedTime(ellapsedTime);
			};
		}
		
		public int CurrentTime{
			get{
				return timer.CurrentTime;
			}
		}
		
		public void Run(){
		}
		
		public void Start(){
			timer.Start();
		}
		
		public void Stop(){
			timer.Stop();
		}			
		
		public void TogglePause(){
			timer.TogglePause();
		}		
		
		public uint OutputWidth {
			get{return 0;} 
			set{}
		}

		public uint OutputHeight {
			get{return 0;}
			set{}
		}
		
		public string OutputFile {
			get {return Catalog.GetString("Fake live source");}
			set {}
		}
				
		public uint VideoBitrate {
			get {return 0;}
			set {}
		}
		
		public uint AudioBitrate {
			get {return 0;}
			set {}
		}		
		
		public bool SetVideoEncoder(LongoMatch.Video.Capturer.GccVideoEncoderType type){
			return true;
		}
		
		public bool SetAudioEncoder(LongoMatch.Video.Capturer.GccAudioEncoderType type){
			return true;
		}
		
		public bool SetVideoMuxer(LongoMatch.Video.Capturer.GccVideoMuxerType type){
			return true;
		}
	}
}
