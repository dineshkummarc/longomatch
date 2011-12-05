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
using Gdk;
using LongoMatch.Handlers;

namespace LongoMatch.Interfaces.GUI
{
	public interface IPlayer
	{
		event SegmentClosedHandler SegmentClosedEvent;
		event TickHandler Tick;
		event ErrorHandler Error;
		event StateChangeHandler PlayStateChanged;
		event NextButtonClickedHandler Next;
		event PrevButtonClickedHandler Prev;
		event DrawFrameHandler DrawFrame;
		event SeekEventHandler SeekEvent;
		
		long AccurateCurrentTime {get;}
		long CurrentTime {get;}
		long StreamLength {get;}
		Pixbuf CurrentMiniatureFrame {get;}
		Pixbuf CurrentFrame {get;}
		Pixbuf LogoPixbuf {set;}
		Pixbuf DrawingPixbuf {set;}
		bool DrawingMode {set;}
		bool LogoMode {set;}
		bool ExpandLogo {set; get;}
		bool Opened {get;}
		bool FullScreen {set;}
		float Rate {get;set;}

		void Open(string mrl);
		void Play();
		void Pause();
		void TogglePlay();
		void SetLogo(string filename);
		void ResetGui();
		void SetPlayListElement(string fileName,long start, long stop, float rate, bool hasNext);
		void Close();
		void SeekTo(long time, bool accurate);
		void SeekInSegment(long pos);
		void SeekToNextFrame(bool in_segment);
		void SeekToPreviousFrame(bool in_segment);
		void StepForward();
		void StepBackward();
		void FramerateUp();
		void FramerateDown();
		void UpdateSegmentStartTime(long start);
		void UpdateSegmentStopTime(long stop);
		void SetStartStop(long start, long stop);
		void CloseActualSegment();
		void SetSensitive();
		void UnSensitive();
	}
}

