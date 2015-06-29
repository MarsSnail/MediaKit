/*
 * Copyright (C) 2013, 2014 Mark Li (lixiaonan06@163.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef MEDIAPARSER_H_
#define MEDIAPARSER_H_

#include "SnailConfig.h"
#include "PlatformType.h"

#include <iostream>
#include <string>
#include <memory>
#include <iosfwd>
#include <deque>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/barrier.hpp>

#include "IOChannel.h"
#include "MediaParserDelegate.h"

using std::auto_ptr;
using std::deque;

namespace MediaCore {

	class MediaParser{
	public:
		MediaParser(std::string url, MediaParserDelegate* mediaParserDelegate);
		virtual ~MediaParser();
		uint64_t getByteLoaded() const {return _bytesLoaded;}
		uint64_t getBytesTotal() const {return _inputStream->size();}
		bool parseComplete() const {return _parseComplete;}

		virtual uint64_t getAudioPacketDTS(PlatformAVPacket *) const=0;
		virtual uint64_t getVideoPacketDTS(PlatformAVPacket *) const=0;
		uint64_t videoBufferLength() const;
		uint64_t audioBufferLength() const;
		virtual void clearBuffers() = 0;
		virtual bool bufferFull() const =0;
		virtual uint64_t getBufferLengthNoLock() const =0;
		virtual uint64_t getBufferLength() const =0;
		virtual bool isBufferEmpty()const =0;
		uint64_t getBufferTime()  const;
		void setBufferTime(uint64_t bufferTime);
		void waitIfNeeded(boost::mutex::scoped_lock& lock);
		//if the parserThread was blocked,then to notify the thread and continue to run
		void notifyParserThread();

		//get video/audio configuer information
		int getVideoWidth() const;
		int getVideoHeight() const;
		double getFramerate() const;
		int64_t getDuration() const; ///secod

		//play control function
		virtual bool seek(int64_t millPos) = 0;
		//million second
		virtual int64_t getRealSeekPos()  const = 0;

		virtual double videoTimeBase() const =0;
		virtual double audioTimeBase() const = 0;

		virtual auto_ptr<PlatformAVPacket> nextAudioEncodedFrame()=0;
		virtual auto_ptr<PlatformAVPacket> nextVideoEncodedFrame()=0;
		virtual int64_t getKeyFrameFilePosByIndex(int index)=0;
		virtual int32_t keyFrameCount()=0;

protected:
		//parser thread
		bool saveVideoPacket(PlatformAVPacket *pkt);
		bool saveAudioPacket(PlatformAVPacket *pkt);
		void startParserThread();
		static void parserLoopStart(MediaParser* mp);
		void parserLoop();
		virtual bool parseNextChunk()=0;
		bool parserThreadKillRequested();
		void requestParserThreadKill();

protected:
		boost::scoped_ptr<IOChannel> _inputStream;
		MediaParserDelegate* _delegate;

		uint64_t _bufferTime;
		mutable boost::mutex _bufferTimeMutex;
		uint64_t _bytesLoaded;
		bool _parseComplete;
		bool _seekRequest;
		auto_ptr<boost::thread> _parserThread;
		mutable boost::mutex _parserThreadKillRequestMutex;
		boost::condition _parserThreadWakeup;
		boost::barrier _parserThreadStartBarrier;
		bool _parserThreadKillRequest;
		//thread run flag mutex
		mutable boost::mutex _parserThreadRunFlag;
		//packets Queue mutex
		mutable boost::mutex _packetsQueueMutex;

		//flag parameters
		bool _hasVideo;
		bool _hasAudio;

		//video/audio configure information
		int _videoWidth;
		int  _videoHeight;
		int64_t  _videoDuration; //millseconds
		double _videoFramerate;
		double _videoAudioSamplerate;

		//packet's container
		deque<AVPacket*> _videoPacketsQueue;
		deque<AVPacket*> _audioPacketsQueue;
	}; // class MediaParser


} // namespace

#endif /* MEDIAPARSER_H_ */
