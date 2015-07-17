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

#ifndef AUDIODECODERFFMPEG_H_
#define AUDIODECODERFFMPEG_H_

#include "SnailConfig.h"
#include "AudioDecoder.h"
#include "MediaParser.h"
#include <iostream>
#include <memory>
#include <deque>

#include "boost/thread.hpp"
#include "boost/thread/condition.hpp"


extern "C"{
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#ifndef PRId64
#define PRId64 "lld"
#endif

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avio.h>
#include <libswresample/swresample.h>
}
using namespace std;

namespace MediaCore {

class AudioDecoderFFmpeg : public AudioDecoder {
public:
	AudioDecoderFFmpeg(MediaDecoderDelegate* delegate);
	virtual ~AudioDecoderFFmpeg();
	bool init();
	AudioDecodedFrame* popAudioDecodedFrame();
	virtual int64_t nextAudioFrameTimestamp();
	virtual void clearAudioFrameQueue();
    virtual bool IsBufferFull() override;
    virtual bool IsBufferLowLevel() override;
protected:
	virtual void decodeAudioFrame();
private:
	int queueLength();
	bool queueEmpty();
	void pushAudioDecodedFrame(AudioDecodedFrame *frame);
	int64_t convertTime(double time)const;
private:
	AVCodecContext *_audioCodecCtx;
	AVCodec *_audioCodec;
	SwrContext *_swrCtx;
	double _audioTimeBase;
	//deque used for save decoded audio frame
	deque<AudioDecodedFrame*> _audioDecodedFrameQueue;
	mutable boost::mutex queueMutex;
	boost::condition _decoderThreadWakeup;
};

} // namespace

#endif /* AUDIODECODERFFMPEG_H_ */
