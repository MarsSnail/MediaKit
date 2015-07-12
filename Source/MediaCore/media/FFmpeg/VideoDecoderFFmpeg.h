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
#include "AudioDecodedFrame.h"
extern "C"{
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}

using namespace std;

namespace MediaCore {

class VideoDecoderFFmpeg : public VideoDecoder{
public:
	VideoDecoderFFmpeg(VideoDecoderDelegate *delegate);
	
	virtual ~VideoDecoderFFmpeg() override;
	virtual bool Init() override;
    virtual bool decodeFrame() override;
	virtual auto_ptr<VideoImage> getVideoImage() override;
	virtual int64_t nextVideoFrameTimestamp() override;
	virtual void clearVideoFrameQueue() override;
	virtual int videoFrameQueueLength() override;
    
    virtual AudioDecodedFrame* popAudioDecodedFrame() override;
    virtual int64_t nextAudioFrameTimestamp() override;
    virtual void clearAudioFrameQueue() override;

private:
	bool init();
    bool InitAudioDecoder();
    int AudioQueueLength();
    bool AudioQueueEmpty();
    int64_t convertAudioTime(double time) const;
    void decodeAudioFrame();
    void pushAudioDecodedFrame(AudioDecodedFrame* frame);
    
    
	bool decodeVideoFrame();
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
    //video
	AVCodecContext *_videoCodecCtx;
	AVCodec *_videoCodec;
	SwsContext *_swsCtx;
	double _videoTimeBase;
	deque<VideoDecodedFrame*> _videoDecodedFramesQueue;
    //audio
    AVCodecContext *_audioCodecCtx;
    AVCodec *_audioCodec;
    SwrContext *_swrCtx;
    double _audioTimeBase;
    //deque used for save decoded audio frame
    deque<AudioDecodedFrame*> _audioDecodedFrameQueue;
    
	boost::mutex _framesQueueMutex;
	boost::condition _decoderThreadWakeup;

	//debug vars
	int  _videoFrameCount;
};


} // namespace

#endif /* VIDEODECODERFFMPEG_H_ */
