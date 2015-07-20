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
#include "MediaParserBase.h"
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

class MediaParserFFmpeg : public MediaParserBase{
public:
	MediaParserFFmpeg(std::string url, MediaParserDelegate* mediaParserDelegate);
	virtual ~MediaParserFFmpeg();

	virtual bool 	Init() override;
	virtual bool 	Seek(int64_t millPos) override;
	virtual int64_t GetRealSeekPos()  const override{return _realSeekPos;}	
	virtual int64_t GetKeyFrameFilePosByIndex(int index) override;
    virtual boost::shared_ptr<AVPacket> GetNextEncodedVideoFrame() override;
    virtual boost::shared_ptr<AVPacket> GetNextEncodedAudioFrame() override;
	virtual void SetBufferTime(uint64_t) override;
	virtual uint64_t GetBufferTime() const override;
	virtual uint64_t GetByteLoaded() const override;
	virtual void PauseMediaParser() override;
	virtual void ContinueMediaParser() override;

private:
	//ffmpeg avioContext callback functions
	static int readPacketCB(void *opaque, unsigned char *buffer, int bufSize);
	static int64_t seekMediaCB(void *opaque, int64_t offset, int whence);
	static const size_t _avioBufferSize = 4096	;
	
	virtual bool parseNextChunk();
	bool SaveVideoPacket(AVPacket *pkt);
	bool SaveAudioPacket(AVPacket *pkt);
	void PauseMediaParserIfNeeded();
	bool parseNextFrame();
	void ResumeMediaParserIfNeeded();
	void clearBuffers();
	void clearBuffersNoLock();
	bool isBufferEmpty() const;
	bool IsParseComplete();
	bool IsMediaPaserBufferFull();
	uint64_t GetPacketBufferedLength();
	uint64_t ConvertTime(double time, AVRational time_base);
	uint64_t GetPacketDTS(AVPacket* packet, AVRational time_base);
	double videoTimeBase();
	double audioTimeBase();
	int readPacket(unsigned char *data, int dataSize);
	int64_t seekMedia(int64_t offset, int whence);
	void SetMediaParserState(const MediaParserState& state);


	bool     	_isParseComplete;
	bool     	_hasVideo;
	bool 	 	_hasAudio;
	uint64_t 	_bufferTime;
	uint64_t 	_bytesLoaded;
    boost::scoped_ptr<IOChannel> 	_inputStream;

	unsigned char*   	_avioBuffer;
	AVIOContext* 	 	_avioCtx;
	AVInputFormat* 	 	_inputFmt;
	AVFormatContext* 	_formatCtx;
	//video objects
	int 				_videoStreamId;
	AVCodec* 			_videoCodec;
	AVStream* 			_videoStream;
	SwsContext* 		_swsCtx;
	AVCodecContext* 	_videoCodecCtx;
	//audio objects
	int 				_audioStreamId;
	AVCodec*			_audioCodec;
	AVStream*			_audioStream;
	SwrContext*			_swrCtx;
	AVCodecContext*		_audioCodecCtx;
	bool 				_seekFlag;
	int64_t 			_realSeekPos;
	AVPacket*			_seekPacket;
	boost::mutex 		_readFrameMutex;
	MediaInfo			_mediaInfo;
	MediaParserState 	_mediaParserState; 
	MediaParserDelegate* _delegate;
	
	boost::mutex 		_mutexVideoQueue;
	boost::mutex    	_mutexAudioQueue;
};

} // namespace

#endif /* MEDIAPARSERFFMPEG_H_ */
