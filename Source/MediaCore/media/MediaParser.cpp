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

#include "MediaParser.h"
#include <memory>
#include <boost/bind.hpp>
#include "SnailSleep.h"

namespace MediaCore {

MediaParser::MediaParser(std::string url, MediaParserDelegate* mediaParserDelegate):
	_inputStream(IOChannel::CreateIOChannel(url)),
	_delegate(mediaParserDelegate),
	_bufferTime(100), //millsecond
	_bytesLoaded(0),
	_parseComplete(0),
	_seekRequest(false),
	_parserThread(0),
	_parserThreadStartBarrier(2),
	_parserThreadKillRequest(false),
	_hasVideo(false),
	_hasAudio(false),
	_videoWidth(0),
	_videoHeight(0),
	_videoDuration(0),
	_videoFramerate(0),
	_videoAudioSamplerate(0)
{
}

MediaParser::~MediaParser(){

}

void MediaParser::setBufferTime(uint64_t bufferTime){
	boost::mutex::scoped_lock lock(_bufferTimeMutex);
	_bufferTime = bufferTime;
}
uint64_t MediaParser::getBufferTime() const {
	boost::mutex::scoped_lock lock(_bufferTimeMutex);
	return _bufferTime;
}

bool MediaParser::saveVideoPacket(AVPacket *packet){

	_videoPacketsQueue.push_back(packet);
	return true;
}

bool MediaParser::saveAudioPacket(AVPacket *packet){
	_audioPacketsQueue.push_back(packet);
	return true;
}

uint64_t MediaParser::videoBufferLength() const{
	if(!_hasVideo || _videoPacketsQueue.empty()){
		return 0;
	}else{
		PlatformAVPacket *beg, *end;
		beg = _videoPacketsQueue.front();
		end = _videoPacketsQueue.back();
		return getVideoPacketDTS(end) - getAudioPacketDTS(beg);
	}
}
uint64_t MediaParser::audioBufferLength() const{
	if(!_hasAudio || _audioPacketsQueue.empty()){
		return 0;
	}else{
		PlatformAVPacket *beg, *end;
		beg = _audioPacketsQueue.front();
		end = _audioPacketsQueue.back();
		return getAudioPacketDTS(end) - getAudioPacketDTS(beg);
	}
}

void MediaParser::waitIfNeeded(boost::mutex::scoped_lock& lock){
	bool pc = parseComplete();
	bool bf = bufferFull();
	if( (pc || bf)&&(!parserThreadKillRequested())){
		_parserThreadWakeup.wait(lock);
	}
}

//parser thread
void MediaParser::startParserThread(){
	_parserThread.reset(new boost::thread(
			boost::bind(parserLoopStart, this)));
	_parserThreadStartBarrier.wait();
}

void MediaParser::parserLoopStart(MediaParser *mp){
	mp->parserLoop();
}

void MediaParser::parserLoop(){
	_parserThreadStartBarrier.wait();
	while(!parserThreadKillRequested()){
		parseNextChunk();
		boost::mutex::scoped_lock lock(_packetsQueueMutex);
		waitIfNeeded(lock);
	}
}
bool MediaParser::parserThreadKillRequested(){
	boost::mutex::scoped_lock lock(_parserThreadRunFlag);
	return _parserThreadKillRequest;
}
void MediaParser::requestParserThreadKill(){
	boost::mutex::scoped_lock lock(_parserThreadRunFlag);
	_parserThreadKillRequest = true;
	_parserThreadWakeup.notify_all();
}

//v
void MediaParser::notifyParserThread(){
	_parserThreadWakeup.notify_all();
}
//get video/audio configuer information
int MediaParser::getVideoWidth() const{
	return _videoWidth;
}
int MediaParser::getVideoHeight() const{
	return _videoHeight;
}
double MediaParser::getFramerate() const{
	return _videoFramerate;
}
int64_t MediaParser::getDuration() const{
	return _videoDuration/1000LL;
}

} // namespace
