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

#include "MediaParserBase.h"
#include <memory>
#include <boost/bind.hpp>
#include "SnailSleep.h"

namespace MediaCore {

MediaParserBase::MediaParserBase()
    : _parserThreadKillRequest(false)
    , _parserThreadStartBarrier(2)
    , _parserThread(0)
{
}

//parser thread
void MediaParserBase::startParserThread(){
	_parserThread.reset(new boost::thread(
			boost::bind(&MediaParserBase::parserLoopStart, this)));
	_parserThreadStartBarrier.wait();
}

void MediaParserBase::parserLoopStart(){
	this->parserLoop();
}

void MediaParserBase::parserLoop(){
	_parserThreadStartBarrier.wait();
	while(!parserThreadKillRequested()){
		parseNextChunk();
	}
}

void MediaParserBase::PauseMediaParserAction()
{
	boost::mutex::scoped_lock lock(_packetsQueueMutex);
	if(!parserThreadKillRequested())
	{
		_parserThreadWakeup.wait(lock);
	}
}

void MediaParserBase::ContinueMediaParserAction(){
	_parserThreadWakeup.notify_all();
}

bool MediaParserBase::parserThreadKillRequested(){
	boost::mutex::scoped_lock lock(_parserThreadRunFlag);
	return _parserThreadKillRequest;
}
void MediaParserBase::requestParserThreadKill(){
	boost::mutex::scoped_lock lock(_parserThreadRunFlag);
	_parserThreadKillRequest = true;
	_parserThreadWakeup.notify_all();
}
} // namespace
