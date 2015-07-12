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

#include "MediaParser.h"

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

#include "SnailConfig.h"
#include "CommonDef.h"
#include "PlatformType.h"
#include "IOChannel.h"
#include "MediaParserDelegate.h"

using std::auto_ptr;
using std::deque;

namespace MediaCore {

class MediaParserBase : public MediaParser{
public:
	MediaParserBase();

protected:
		virtual bool parseNextChunk() = 0;

		void startParserThread();
		void parserLoopStart();
		void parserLoop();
		bool parserThreadKillRequested();
		void requestParserThreadKill();
		void PauseMediaParserAction();
		void ContinueMediaParserAction();

		bool     						_parserThreadKillRequest;
		boost::barrier 					_parserThreadStartBarrier;
		boost::condition 				_parserThreadWakeup;
		mutable boost::mutex 			_bufferTimeMutex;
		mutable boost::mutex 			_parserThreadKillRequestMutex;
		mutable boost::mutex 			_parserThreadRunFlag;		
		mutable boost::mutex 			_packetsQueueMutex;
		deque<EncodedAVFrame*> 			_videoPacketsQueue;
		deque<EncodedAVFrame*> 			_audioPacketsQueue;
		auto_ptr<boost::thread> 		_parserThread;
}; // class MediaParser


} // namespace

#endif /* MEDIAPARSER_H_ */
