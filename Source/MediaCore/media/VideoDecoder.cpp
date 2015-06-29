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

#include "VideoDecoder.h"
#include <iostream>
#include "SnailSleep.h"

namespace MediaCore {

VideoDecoder::VideoDecoder(AVPipeline *avPipeline):
		_killThreadFlag(false),
		_isDecodeComplete(false),
		_avPipeline(avPipeline),
		_videoDecoderThread(0)
{}

VideoDecoder::~VideoDecoder(){
}
void VideoDecoder::startVideoDecoderThread(){
	_videoDecoderThread.reset(new boost::thread(
			boost::bind(decoderLoopStarter,this)));
}

void VideoDecoder::decoderLoopStarter(VideoDecoder *videoDecoder){
	videoDecoder->decoderLoop();
}

void VideoDecoder::decoderLoop(){
	while(continueRunThread()){
		decodeVideoFrame();
		//snail::timer::snailSleep(100);
	}
}

bool VideoDecoder::continueRunThread(){
	boost::mutex::scoped_lock lock(_stopThreadMutex);
	return !_killThreadFlag;
}
void VideoDecoder::setKillThread(){
	boost::mutex::scoped_lock lock(_stopThreadMutex);
	_killThreadFlag = true;
}

} // namespace
