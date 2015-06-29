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

#ifndef VIDEODECODER_H_
#define VIDEODECODER_H_

#include "SnailConfig.h"
#include <iostream>
#include <memory>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <deque>
#include "ImageType.h"

using namespace std;


namespace MediaCore {
    class AVPipeline;
//no copyable
class VideoDecoder {
public:
	VideoDecoder(AVPipeline *avPipeline);
	virtual  ~VideoDecoder();
	virtual auto_ptr<VideoImage> getVideoImage() = 0;
	virtual int64_t nextVideoFrameTimestamp() = 0;
	virtual void clearVideoFrameQueue() = 0;
	virtual int videoFrameQueueLength() = 0;
	bool IsDecodeComplete() {return _isDecodeComplete;}

protected:
	void startVideoDecoderThread();
	static void decoderLoopStarter(VideoDecoder *videoDecoder);
	void decoderLoop();
	//this virtual function must be implemented by the child class
	virtual bool decodeVideoFrame() = 0;
	//control the video decoder thread life time
	bool continueRunThread();
	void setKillThread();
protected:
	bool _killThreadFlag;
	bool _isDecodeComplete;

	AVPipeline	*_avPipeline;
	//fileds for video decoder thread
	auto_ptr<boost::thread> _videoDecoderThread;
	mutable boost::mutex _stopThreadMutex; //used for get/set _stopThread
};

} // namespace
#endif /* VIDEODECODER_H_ */
