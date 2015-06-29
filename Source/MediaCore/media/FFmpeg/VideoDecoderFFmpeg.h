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

#ifndef VIDEODECODERFFMPEG_H_
#define VIDEODECODERFFMPEG_H_

#include "SnailConfig.h"
#include "VideoDecoder.h"
#include "ImageType.h"
#include "VideoDecodedFrame.h"
extern "C"{
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}

using namespace std;

namespace MediaCore {
/*
class VideoDecodedFrame{
public:
	VideoDecodedFrame(VideoImage *image, int64_t pts):
		_image(image),
		_pts(pts){

	}
	VideoImage *getImage() const {
		return _image;
	}
	int64_t getPts() const {
		return _pts;
	}
private:
	VideoImage *_image;
	int64_t _pts;
};
*/
class VideoDecoderFFmpeg : public VideoDecoder{
public:
	VideoDecoderFFmpeg(AVPipeline *pipeline);
	 virtual ~VideoDecoderFFmpeg();
	virtual auto_ptr<VideoImage> getVideoImage() ;
	virtual int64_t nextVideoFrameTimestamp();
	virtual void clearVideoFrameQueue();
	virtual int videoFrameQueueLength();
private:
	bool init();
	virtual bool decodeVideoFrame();
	void pushDecodedVideoFrame(AVFrame *frame);
	VideoDecodedFrame* popDecodedVideoFrame();
	//function for operate the video decoded frame queue
	int queueSize();
	bool queueEmpty();
	void pushFrame(VideoDecodedFrame *frame);
	VideoDecodedFrame *popFrame();
	int64_t convertTime(double time)const;
	VideoImage *yuvToRgb(AVFrame* frame);
	VideoImage *saveYuv(AVFrame *frame);
private:
	AVCodecContext *_videoCodecCtx;
	AVCodec *_videoCodec;
	SwsContext *_swsCtx;
	double _videoTimeBase;
	//video decoded frame queue
	//deque<AVFrame*> _videoDecodedFramesQueue;
	deque<VideoDecodedFrame*> _videoDecodedFramesQueue;
	boost::mutex _framesQueueMutex;
	boost::condition _decoderThreadWakeup;

	//debug vars
	int  _videoFrameCount;
};


} // namespace

#endif /* VIDEODECODERFFMPEG_H_ */
