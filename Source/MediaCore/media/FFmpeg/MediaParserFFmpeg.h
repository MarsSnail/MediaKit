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

#ifndef MEDIAPARSERFFMPEG_H_
#define MEDIAPARSERFFMPEG_H_

#include "SnailConfig.h"
#include "MediaParser.h"
#include <deque>
#include <string>
#include <memory>
extern "C"{
#ifndef INT64_C
#define INT64_C(c) (c##LL)
#endif

#ifndef UINT64_C
#define UINT64_C(c) (c##ULL)
#endif

#ifndef PRId64
#define PRId64 "lld"
#endif

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

using std::deque;

namespace MediaCore {

////get the global parameter
AVCodecContext *global_getVideoCtx();
AVCodec *global_getVideoCodec();
AVCodecContext *global_getAudioCtx();
AVCodec *global_getAudioCodec();

class MediaParserFFmpeg : public MediaParser{
public:
	MediaParserFFmpeg(std::string url, MediaParserDelegate* mediaParserDelegate);
	virtual ~MediaParserFFmpeg();

	void notifyIfNeed();
	uint64_t videoConvertTime(double time)const;
	uint64_t audioConvertTime(double time) const;
	virtual uint64_t getAudioPacketDTS(AVPacket *pt) const;
	virtual uint64_t getVideoPacketDTS(AVPacket *pt) const;
/*	virtual uint64_t videoBufferLength() const;
	virtual uint64_t audioBufferLength() const;*/
	virtual void clearBuffers();
	void clearBuffersNoLock();
	virtual bool bufferFull() const ;
	virtual uint64_t getBufferLengthNoLock() const;
	virtual uint64_t getBufferLength() const;
	virtual bool isBufferEmpty() const;
	virtual double videoTimeBase()const ;
	virtual double audioTimeBase()const ;
	virtual int64_t getKeyFrameFilePosByIndex(int index);
	virtual int32_t keyFrameCount();

	//play control function
	virtual bool seek(int64_t millPos);
	virtual int64_t getRealSeekPos()  const{
		return _realSeekPos;
	}

	virtual auto_ptr<AVPacket> nextAudioEncodedFrame();
	virtual auto_ptr<AVPacket> nextVideoEncodedFrame();
protected:
	//parser thread
	virtual bool parseNextChunk();
	bool parseNextFrame();
/*
	bool saveVideoPacket(AVPacket *pkt);
	bool saveAudioPacket(AVPacket *pkt);
*/
private:
	//ffmpeg avioContext callback functions
	int readPacket(unsigned char *data, int dataSize);
	int64_t seekMedia(int64_t offset, int whence);
	static int readPacketCB(void *opaque, unsigned char *buffer, int bufSize);
	static int64_t seekMediaCB(void *opaque, int64_t offset, int whence);

	//functions, vars for demuxer the video container
	bool initParser();

	AVFormatContext *_formatCtx;
	AVIOContext *_avioCtx;
	AVInputFormat *_inputFmt;
	unsigned char *_avioBuffer;
	static const size_t _avioBufferSize = 4096;

	//video objects
	AVCodecContext *_videoCodecCtx;
	AVCodec *_videoCodec;
	AVStream *_videoStream;
	SwsContext *_swsCtx;
	int _videoStreamId;
	//audio objects
	 AVCodecContext *_audioCodecCtx;
	 AVCodec *_audioCodec;
	AVStream *_audioStream;
	SwrContext *_swrCtx;
	int _audioStreamId;

	unsigned long _lastParsedPosition;

	//video packet deque
//	deque<AVPacket*> _videoPacketsQueue;
//	deque<AVPacket*> _audioPacketsQueue;

	//vars for seek
	bool _seekFlag;
	int64_t _realSeekPos;
	AVPacket *_seekPacket;

	//debug var
	int _audioPacketCount;
	int _videoPacketCount;

	//for sync av_read_frame and av_seek_frame;
	boost::mutex _readFrameMutex;
};

} // namespace

#endif /* MEDIAPARSERFFMPEG_H_ */
