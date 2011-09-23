//
//  Copyright (C) 2009 Andoni Morales Alastruey
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
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//

using System;
using System.Collections.Generic;
using Gdk;
using LongoMatch.Store;
using LongoMatch.Gui;
using LongoMatch.Video.Common;




namespace LongoMatch.Handlers
{


	public class VideoDrawingsManager
	{
		private PlayerBin player;
		private uint timeout;
		private bool inKeyFrame;
		private bool canStop;
		private Play loadedPlay;

		public VideoDrawingsManager(PlayerBin player)
		{
			this.player = player;
			timeout = 0;
		}

		~ VideoDrawingsManager() {
			StopClock();
		}

		public Play Play {
			set {
				loadedPlay = value;
				inKeyFrame = false;
				canStop = true;
				ResetPlayerWindow();
				ConnectSignals();
				StartClock();
			}
		}

		private Drawing Drawing {
			get {
				return loadedPlay.KeyFrameDrawing;
			}
		}

		private void StartClock() {
			if(timeout ==0)
				timeout = GLib.Timeout.Add(20,CheckStopTime);
		}

		private void StopClock() {
			if(timeout != 0) {
				GLib.Source.Remove(timeout);
				timeout = 0;
			}
		}

		private void ConnectSignals() {
			player.PlayStateChanged += OnStateChanged;
			player.SeekEvent += OnSeekEvent;
			player.SegmentClosedEvent += OnSegmentCloseEvent;
		}

		private void DisconnectSignals() {
			player.PlayStateChanged -= OnStateChanged;
			player.SeekEvent -= OnSeekEvent;
			player.SegmentClosedEvent -= OnSegmentCloseEvent;
		}

		private int NextStopTime() {
			return Drawing.RenderTime;
		}

		private void PrintDrawing() {
			Console.WriteLine (Drawing);
			player.Pause();
			player.SeekInSegment(Drawing.RenderTime);
			player.DrawingPixbuf = Drawing.Pixbuf;
			inKeyFrame = true;
		}

		private void ResetPlayerWindow() {
			player.DrawingPixbuf = null;
			player.SetLogo(System.IO.Path.Combine(MainClass.ImagesDir(),"background.png"));
		}

		private bool CheckStopTime() {
			int currentTime = (int)player.AccurateCurrentTime;

			if(Drawing == null || !canStop)
				return true;
			if((currentTime)>NextStopTime()) {
				StopClock();
				PrintDrawing();
			}
			return true;
		}

		protected virtual void OnStateChanged(object sender, StateChangeArgs args) {
			//Check if we are currently paused displaying the key frame waiting for the user to
			//go in to Play. If so we can stop
			if(inKeyFrame && args.Playing == true) {
				ResetPlayerWindow();
				inKeyFrame = false;
			}
		}

		protected virtual void OnSeekEvent(long time) {
			if(Drawing == null)
				return;
			if(inKeyFrame) {
				ResetPlayerWindow();
				inKeyFrame = false;
			}
			canStop = time < Drawing.RenderTime;
			if(canStop)
				StartClock();
			else StopClock();
		}

		protected virtual void OnSegmentCloseEvent() {
			ResetPlayerWindow();
			DisconnectSignals();
			StopClock();
		}

	}
}
