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

#include "MediaParserFFmpeg.h"
#include "SnailException.h"
#include <iostream>

using std::cout;
using std::cin;
using std::endl;

namespace MediaCore {

 AVCodecContext *gVideoCodecCtx=0;
 AVCodec *gVideoCodec=0;
 AVCodecContext *gAudioCodecCtx=0;
 AVCodec *gAudioCodec=0;

 ////get the global parameter
 AVCodecContext *global_getVideoCtx(){
	 return gVideoCodecCtx;
 }
 AVCodec *global_getVideoCodec(){
	 return gVideoCodec;
 }
 AVCodecContext *global_getAudioCtx(){
	 return gAudioCodecCtx;
 }
 AVCodec *global_getAudioCodec(){
	 return gAudioCodec;
 }
	MediaParserFFmpeg::MediaParserFFmpeg(std::string url, MediaParserDelegate* delegate):
			MediaParser(url, delegate),
			_formatCtx(0),
			_avioCtx(0),
			_inputFmt(0),
			_avioBuffer(0),
			_videoStream(0),
			_swsCtx(NULL),
			_videoStreamId(0),
			_audioStream(0),
			_swrCtx(NULL),
			_audioStreamId(0),
			_lastParsedPosition(0),
			_seekFlag(0),
			_realSeekPos(0),
			_seekPacket(NULL),
			_audioPacketCount(0),
			_videoPacketCount(0)
{
		if(!initParser()){
			throw SnailException("ERROR: Initialize the MediaParserFFmpeg failed");
		}
		startParserThread();
}
MediaParserFFmpeg::~MediaParserFFmpeg(){

	requestParserThreadKill();
	clearBuffers();
	_parserThread->join();
	_parserThread.reset();
	avcodec_close(_videoCodecCtx);
	avcodec_close(_audioCodecCtx);
	avformat_close_input(&_formatCtx);//this function will free the _formatCtx and _avioBuffer which malloc by youself
}
/// init ffmpeg, open the input stream by using ffmpeg API, initialize the
//  demux and decoder objects of  audio&video
const char* formatToString(enum AVSampleFormat fmt){
		switch(fmt){
		case AV_SAMPLE_FMT_U8:
			return "AV_SAMPLE_FMT_U8";
		case AV_SAMPLE_FMT_S16:
			return "AV_SAMPLE_FMT_S16";
		case AV_SAMPLE_FMT_S32:
			return "AV_SAMPLE_FMT_S32";
		case AV_SAMPLE_FMT_FLT:
			return "AV_SAMPLE_FMT_FLT";
		case AV_SAMPLE_FMT_DBL:
			return "AV_SAMPLE_FMT_DBL";
		case AV_SAMPLE_FMT_U8P:
			return "AV_SAMPLE_FMT_U8P";
		case AV_SAMPLE_FMT_S16P:
			return "AV_SAMPLE_FMT_S16P";
		case AV_SAMPLE_FMT_S32P:
			return "AV_SAMPLE_FMT_S32P";
		case AV_SAMPLE_FMT_FLTP:
			return "AV_SAMPLE_FMT_FLTP";
		case AV_SAMPLE_FMT_DBLP:
			return "AV_SAMPLE_FMT_DBLP";
		default:
			return "NULL";
		}
	}

bool MediaParserFFmpeg::initParser(){
	av_register_all();
	_avioBuffer = (unsigned char*)av_malloc(1024);
	if(!_avioBuffer){
		printf("<lxn> %s(%d)--> alloc the avioContext's buffer failed\n",__FILE__,__LINE__);
		return false;
	}else{
		printf("OK: success alloc the avioContext buffer \n");
	}
	_avioCtx = avio_alloc_context(_avioBuffer,
			1024,
			0,
			this,
			readPacketCB,
			NULL,
			seekMediaCB);
	if(!_avioCtx){
		printf("<lxn> %s(%d)---> alloc avioContext failed\n",__FILE__,__LINE__);
		return false;
	}else{
		printf("OK: success alloc avioContext\n");
	}
	if(av_probe_input_buffer(_avioCtx, &_inputFmt, "", NULL, 0, 4096)<0){
		printf("<lxn> %s(%d)--> cann't probe the video type\n", __FILE__,__LINE__);
	}else{
		printf("inputFormat is :%s\n",_inputFmt->long_name);
		printf("OK: success probe the input type\n");
	}
	_formatCtx = avformat_alloc_context();
	if(!_formatCtx){
		printf("<lxn> %s(%d)--->alloc avioContext failed\n",__FILE__,__LINE__);
		return false;
	}else{
		printf("OK: success alloc AVFormatContext\n");
	}
	_avioCtx->seekable = 0;
	_formatCtx->pb = _avioCtx;
	//open the input file
	if( (avformat_open_input(&_formatCtx, "",_inputFmt, NULL))<0){
		printf("<lxn> %s(%d) could not open the input file\n",__FILE__,__LINE__);
		return false;
	}else{
		printf("OK: success open the input file\n");
	}
	//find the audio&video streams information
	if(avformat_find_stream_info(_formatCtx, NULL) < 0){
		printf("<lxn> %s(%d) ---> Could not find the stream information\n",__FILE__,__LINE__);
		return false;
	}else{
		printf("OK: success find the streams information\n");
	}

	//find the video stream  id
	if((_videoStreamId = av_find_best_stream(_formatCtx,
			AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0))>=0){
		printf("_videoStreamid = %d\n",_videoStreamId);
		_videoStream = _formatCtx->streams[_videoStreamId];
		printf("OK: success get the videoStream\n");
	}else{
		printf("<lxn> %s(%d) cann't find the video stream information\n",__FILE__,__LINE__);
		return false;
	}

	//find the audio stream id
	if( (_audioStreamId = av_find_best_stream(_formatCtx,
			AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0))>=0){
		printf("audioStreamId=%d\n",_audioStreamId);
		_audioStream = _formatCtx->streams[_audioStreamId];
		printf("OK: success get the audioStream\n");
	}else{
		printf("<lxn> %s(%d) cann't find the video stream information\n",__FILE__,__LINE__);
		return false;
	}

	MediaInfo mediaInfo;
	//init the video decoder
	if(_videoStream){
		_hasVideo = true;
		AVCodecID codec = _videoStream->codec->codec_id;
		_videoWidth = _videoStream->codec->width;
		_videoHeight = _videoStream->codec->height;
		_videoFramerate = av_q2d(_videoStream->r_frame_rate);
		_videoDuration = (int64_t)(_formatCtx->duration)/1000LL; //millsecond
		mediaInfo._videoWidth  = _videoWidth;
		mediaInfo._videoHeight = _videoHeight;
		mediaInfo._videoFramerate = _videoFramerate;
		mediaInfo._videoDuration = _videoDuration;
		mediaInfo._videoTimeBase = _videoStream->time_base;
		//get the video decoder and initialize the avcodecContext
		cout<<(int)codec<<endl;
		_videoCodec = avcodec_find_decoder(_videoStream->codec->codec_id);
		//init the gloabl parameter
		gVideoCodec = _videoCodec;
		if(!_videoCodec){
			printf("<lxn> %s(%d) failed to find the decoder\n",__FILE__,__LINE__);
			return false;
		}else{
			printf("OK: success find the suitable video decoder\n");
		}
		//initialize the video codecContext
		_videoCodecCtx = _videoStream->codec;
		//init the global video codecContext
        gVideoCodecCtx = _videoCodecCtx;
		if(avcodec_open2(_videoCodecCtx, _videoCodec, NULL) < 0){
			printf("<lxn> %s(%d)-->failed open the decodc\n",__FILE__,__LINE__);
			return false;
		}else{
			printf("OK: success open the suitable video decoder\n");
			printf("codec: %s, bit_rate:%d, wh:(%d,%d),  \n", _videoCodec->long_name,
					_videoCodecCtx->bit_rate, _videoCodecCtx->width, _videoCodecCtx->height  );
		}
	}//if(_videoStream)

	//init the audio decoder
	if(_audioStream){
		_hasAudio = true;
		mediaInfo._audioTimeBase = _audioStream->time_base;
		_audioCodec = avcodec_find_decoder(_audioStream->codec->codec_id);
		//init the global audio codec
		gAudioCodec = _audioCodec;
		if(!_audioCodec){
			printf("<lxn> %s(%d)--> fails to find the suitable audio decoder\n",__FILE__,__LINE__);
			return false;
		}else{
			printf("OK: success find the suitable audio decoder\n");
		}
		_audioCodecCtx = _audioStream->codec;
		//init the global audio codecContext
		gAudioCodecCtx = _audioCodecCtx;
		if(avcodec_open2(_audioCodecCtx, _audioCodec, NULL)<0){
			printf("<lxn> %s(%d)-->fail to open the audioCodec\n",__FILE__,__LINE__);
			return false;
		}else{
			printf("OK: success open the audioCodec\n");
		}
		printf("======audio information========\n");
		printf("sampleRate: <%d>, channles: <%d>, sampleFormat: <%s>, codec: <%s> , channleLayout: <%d> \n", _audioCodecCtx->sample_rate, _audioCodecCtx->channels,
					formatToString(_audioCodecCtx->sample_fmt),_audioCodec->long_name, (int)_audioCodecCtx->channel_layout);
	}//if(_audioStream)

	if(_delegate)
		_delegate->SetMediaInfo(mediaInfo);
	
	return true;
}

/******for ffmpeg avioContext callback function****************/
int MediaParserFFmpeg::readPacket(unsigned char *data, int dataSize){
	size_t ret = _inputStream->read(static_cast<void*>(data), dataSize);
	return ret;
}
int MediaParserFFmpeg::readPacketCB(void *opaque, unsigned char *buffer, int bufSize){
	MediaParserFFmpeg* tmpPtr = static_cast<MediaParserFFmpeg*>(opaque);
	return tmpPtr->readPacket(buffer,bufSize);
}

int64_t MediaParserFFmpeg::seekMedia(int64_t offset, int whence){
	if(whence == SEEK_SET){
		if(offset < 0)
			return -1;
		else
			_inputStream->seek(offset);
	}else if(whence == SEEK_CUR){
			_inputStream->seek(_inputStream->tell()+static_cast<std::streamoff>(offset));
	}else if(whence == SEEK_END){
		_inputStream->seek(1024);
	}else{
		return 0;
	}
	return _inputStream->tell();
}
int64_t MediaParserFFmpeg::seekMediaCB(void *opaque, int64_t offset, int whence){
	MediaParserFFmpeg *tmpPtr = static_cast<MediaParserFFmpeg*>(opaque);
	return tmpPtr->seekMedia(offset,whence);
}

//parser thread
bool MediaParserFFmpeg::parseNextChunk(){
	if(!parseNextFrame()) return false;
	return true;
}
bool MediaParserFFmpeg::parseNextFrame(){
#if ENABLE_DEBUG
	cout<<"############video packet counts = "<<_videoPacketCount<<endl;
#endif
	int result = 0;
	if(_parseComplete){
		return false;
	}

	if(!_formatCtx){
		printf("%s(%d)-->formatContext is not exist\n",__FILE__,__LINE__);
		_parseComplete = true;
	}

	AVPacket *pkt;
	boost::mutex::scoped_lock readFrameLock(_readFrameMutex);
	if(_seekFlag){
		_seekFlag = false;
		pkt = _seekPacket;
		_seekPacket = NULL;
	}else{
		pkt = new AVPacket();
		av_init_packet(pkt);
		pkt->data = NULL;
		pkt->size = 0;
		//sync  av_read_frame and seek operation. when act the seek opeation,cann't read
		//packet until seek operation complete over.
		result = av_read_frame(_formatCtx,pkt);

		if(result>=0){
			//do nothing
		}else{
			printf("%s(%d) got packet failed\n",__FILE__,__LINE__);
			delete pkt;
			pkt = NULL;
			_parseComplete = true;
			printf("%s(%d)--> maybe playover\n",__FILE__,__LINE__);
			return false;
		}
	}
	readFrameLock.unlock();

	//use mutex to protect the queue from be multiplated
	//boost::mutex::scoped_lock lock(_packetsQueueMutex);
	boost::mutex::scoped_lock lock(_packetsQueueMutex);
	if(pkt->stream_index==_videoStreamId){
#if ENABLE_DEBUG
		_videoPacketCount++;
#endif
		saveVideoPacket(pkt);
	}
	if(pkt->stream_index==_audioStreamId){
#if ENABLE_DEBUG
		_audioPacketCount++;
#endif
#if ENABLE_SOUND
		saveAudioPacket(pkt);
#else
		av_free_packet(pkt);
		delete pkt;
		pkt = NULL;
#endif
	}
	//check the packet queue state ,if the packets'duration bigger than the buffertime(the user set this value ) then block the thread.
	waitIfNeeded(lock);
	//sync the loaded bytes
	_bytesLoaded = _inputStream->tell();
	return true;
}

/*bool MediaParserFFmpeg::saveVideoPacket(AVPacket *packet){

	_videoPacketsQueue.push_back(packet);
	return true;
}

bool MediaParserFFmpeg::saveAudioPacket(AVPacket *packet){
	_audioPacketsQueue.push_back(packet);
	return true;
}*/

//utility functions
uint64_t MediaParserFFmpeg::videoConvertTime(double time) const {
	return (uint64_t)(time*av_q2d(_videoStream->time_base)*1000.0);
}

uint64_t MediaParserFFmpeg::audioConvertTime(double time) const {
	return (uint64_t)(time*av_q2d(_audioStream->time_base)*1000.0);
}
uint64_t MediaParserFFmpeg::getAudioPacketDTS(AVPacket *pt) const {
	if(!pt){
		return -1;}
	return audioConvertTime(pt->dts);
}

uint64_t MediaParserFFmpeg::getVideoPacketDTS(AVPacket *pt) const{
	if(!pt){
		return -1;}
	return videoConvertTime(pt->dts);	
}
/*
uint64_t MediaParserFFmpeg::videoBufferLength() const {
	if(!_hasVideo || _videoPacketsQueue.empty()){
		return 0;
	}else{
		AVPacket *beg, *end;
		beg = _videoPacketsQueue.front();
		end = _videoPacketsQueue.back();
		return videoConvertTime(end->dts) - videoConvertTime(beg->dts);
	}
}
uint64_t MediaParserFFmpeg::audioBufferLength() const {
	if(!_hasAudio || _audioPacketsQueue.empty()){
		return 0;
	}else{
		AVPacket *beg, *end;
		beg = _audioPacketsQueue.front();
		end = _audioPacketsQueue.back();
		return audioConvertTime(end->dts) - audioConvertTime(beg->dts);
	}

}
*/
void MediaParserFFmpeg::clearBuffers(){
	if(isBufferEmpty()){
		return;
	}else{
		//clear videodeque;
		boost::mutex::scoped_lock(_packetsQueueMutex);
		deque<AVPacket*>::iterator index, end;
		index = _videoPacketsQueue.begin();
		end = _videoPacketsQueue.end();
		for(; index<end; index++){
			av_free_packet(*index);
			delete (*index);
		}
		_videoPacketsQueue.clear();
		//clear audiodeque
		index = _audioPacketsQueue.begin();
		end = _audioPacketsQueue.end();
		for(; index<end; index++){
			av_free_packet(*index);
			delete (*index);
		}
		_audioPacketsQueue.clear();
	}
	_parserThreadWakeup.notify_all();
}
void MediaParserFFmpeg::clearBuffersNoLock(){
	if(isBufferEmpty()){
		return;
	}else{
		//clear videodeque;
		deque<AVPacket*>::iterator index, end;
		index = _videoPacketsQueue.begin();
		end = _videoPacketsQueue.end();
		for(; index<end; index++){
			av_free_packet(*index);
			delete (*index);
#if ENABLE_DEBUG
			_videoPacketCount--;
#endif
		}
		_videoPacketsQueue.clear();
		//clear audiodeque
		index = _audioPacketsQueue.begin();
		end = _audioPacketsQueue.end();
		for(; index<end; index++){
#if ENABLE_DEBUG
			_audioPacketCount--;
#endif
			av_free_packet(*index);
			delete (*index);
		}
		_audioPacketsQueue.clear();
	}
}
bool MediaParserFFmpeg::bufferFull() const {
	//boost::mutex::scoped_lock lock(_packetsQueueMutex);
	if(isBufferEmpty()){
		return false;
	}else{
		uint64_t curTimeLen = getBufferLengthNoLock();
		return curTimeLen > _bufferTime;
	}
	return 1;
}
uint64_t MediaParserFFmpeg::getBufferLengthNoLock() const {

#if ENABLE_SOUND
	if(_hasVideo&&_hasAudio){
		return std::min(videoBufferLength(),audioBufferLength());
	}else if(_hasVideo){
		return videoBufferLength();
	}else if(_hasAudio){
		return audioBufferLength();
	}else{
		return 0;
	}
#else
	return videoBufferLength();
#endif
}

uint64_t MediaParserFFmpeg::getBufferLength() const{
	//boost::mutex::scoped_lock lock(_packetsQueueMutex);
	return getBufferLengthNoLock();
}
bool MediaParserFFmpeg::isBufferEmpty() const{
#if ENABLE_SOUND
	return _videoPacketsQueue.empty()&&_audioPacketsQueue.empty();
#else
	return _videoPacketsQueue.empty();
#endif
}

double MediaParserFFmpeg::videoTimeBase() const {
	if(_videoStream)
		return av_q2d(_videoStream->time_base);
	return 0.0;
}
double MediaParserFFmpeg::audioTimeBase() const {
	if(_audioStream)
		return av_q2d(_audioStream->time_base);
	return 0.0;
}
void MediaParserFFmpeg::notifyIfNeed(){
	uint64_t bufferedLen = getBufferLength();
	uint64_t notifyLen = _bufferTime/10;
	if(bufferedLen<notifyLen && !_parseComplete){//when parse competely,then block the thread.
		_parserThreadWakeup.notify_all();
	}
}
auto_ptr<AVPacket> MediaParserFFmpeg::nextAudioEncodedFrame(){
	boost::mutex::scoped_lock lock(_packetsQueueMutex);
	auto_ptr<AVPacket> result;
	if(_audioPacketsQueue.empty()){
		//do nothing here
	}else {
		AVPacket *pkt = _audioPacketsQueue.front();
		result.reset(pkt);
		_audioPacketsQueue.pop_front();
#if ENABLE_DEBUG
		_audioPacketCount--;
#endif
	}
	notifyIfNeed();
	return result;
}
auto_ptr<AVPacket> MediaParserFFmpeg::nextVideoEncodedFrame(){
	boost::mutex::scoped_lock lock(_packetsQueueMutex);
	auto_ptr<AVPacket> result;
	if(_videoPacketsQueue.empty()){
	//do noting here
	}else{
		AVPacket *pkt = _videoPacketsQueue.front();
		result.reset(pkt);
		_videoPacketsQueue.pop_front();
#if ENABLE_DEBUG
		_videoPacketCount--;
#endif
	}
	notifyIfNeed();
	return result;
}

//play control functions
bool MediaParserFFmpeg::seek(int64_t millPos){
	boost::mutex::scoped_lock lock(_packetsQueueMutex);
	boost::mutex::scoped_lock readFrameLock(_readFrameMutex);//sync with av_read_frame
	if(millPos<0 || millPos>_videoDuration){
			return false;
	}
		//cout<<"convert time is "<<av_rescale(millPos, _videoStream->time_base.den, AV_TIME_BASE*(int64_t)_videoStream->time_base.num)<<endl;
	if(av_seek_frame(_formatCtx, _videoStreamId, (millPos/1000)/videoTimeBase(), 0)<0){
		return false;
	}
//if seek success then clear the Packet queues
	clearBuffersNoLock();

	_seekFlag = false;
	while(1){
		_seekPacket = new AVPacket();
		av_init_packet(_seekPacket);
		_seekPacket->data = NULL;
		_seekPacket->size = 0;
		int result = av_read_frame(_formatCtx, _seekPacket);
		if(result >=0){
			if(_seekPacket->stream_index==_audioStreamId){
				saveAudioPacket(_seekPacket);
			}else{
				_realSeekPos = videoConvertTime(_seekPacket->pts);
				saveVideoPacket(_seekPacket);
				break;
			}
		}else{
			break;
		}
	}
	_parseComplete = false;
	_parserThreadWakeup.notify_all();
	cout<<"ok: seek operate is over"<<endl;
	return true;
}

 int64_t MediaParserFFmpeg::getKeyFrameFilePosByIndex(int index){
	int64_t res = 0;
	if(_videoStream){
		if(index>_videoStream->nb_index_entries){
			//do nothing
		}else{
			res = _videoStream->index_entries[index].pos;
		}
	}
	return res;
}
int32_t MediaParserFFmpeg::keyFrameCount(){
	int32_t res = 0;
	if(_videoStream){
		res = _videoStream->nb_index_entries;
	}
	return res;
}

} // namespace
