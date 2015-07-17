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
#include "AudioDecodedFrame.h"
#include "MediaDecoderDelegate.h"

using namespace std;


namespace MediaCore {
    class VideoDecoderDelegate;
    class VideoDecodedFrame;
    

//no copyable
class VideoDecoder {
public:
    static VideoDecoder* CreateVideoDecoder(MediaDecoderDelegate* delegate);
    virtual  ~VideoDecoder(){}
	virtual bool Init() = 0;
	virtual auto_ptr<VideoImage> getVideoImage() = 0;
	virtual int64_t nextVideoFrameTimestamp() = 0;
    virtual VideoDecodedFrame* GetDecodedVideoFrame(int64_t timestamp) = 0;
	virtual void clearVideoFrameQueue() = 0;
	virtual int videoFrameQueueLength() = 0;
    virtual bool IsBufferFull() = 0;
    virtual bool IsBufferLowLevel() = 0;
    virtual bool DecodeVideoFrame() = 0;
};

} // namespace
#endif /* VIDEODECODER_H_ */
