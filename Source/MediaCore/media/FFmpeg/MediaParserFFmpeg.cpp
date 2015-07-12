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
#include "PlatformType.h"

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
    MediaParserFFmpeg::MediaParserFFmpeg(std::string url, MediaParserDelegate* delegate)
    : _isParseComplete(false)
    , _hasVideo(false)
    , _hasAudio(false)
    , _bufferTime(100)
    , _bytesLoaded(0)
    , _inputStream(IOChannel::CreateIOChannel(url))
    , _avioBuffer(0)
    , _avioCtx(0)
    , _inputFmt(0)
    , _formatCtx(0)
    , _videoStreamId(0)
    , _videoStream(0)
    , _swsCtx(NULL)
    , _audioStreamId(0)
    , _audioStream(0)
    , _swrCtx(NULL)
    , _seekFlag(0)
    , _realSeekPos(0)
    , _seekPacket(NULL)
    , _delegate(delegate)
    {
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
    
    bool MediaParserFFmpeg::Init(){
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
        
        //init the video decoder
        if(_videoStream){
            _hasVideo = true;
            _mediaInfo._videoWidth = _videoStream->codec->width;
            _mediaInfo._videoHeight = _videoStream->codec->height;
            _mediaInfo._videoFramerate = av_q2d(_videoStream->r_frame_rate);
            _mediaInfo._videoDuration = (int64_t)(_formatCtx->duration)/1000LL; //millsecond
            _mediaInfo._videoTimeBase = _videoStream->time_base;
            _mediaInfo._key_frame_count = _videoStream->nb_index_entries;
            //get the video decoder and initialize the avcodecContext
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
            _mediaInfo._audioTimeBase = _audioStream->time_base;
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
            _delegate->SetMediaInfo(_mediaInfo);
        
        startParserThread();
        
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
        int result = 0;
        if(_isParseComplete){
            return false;
        }
        
        if(!_formatCtx){
            printf("%s(%d)-->formatContext is not exist\n",__FILE__,__LINE__);
            _isParseComplete = true;
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
            
            if(result !=0){
                printf("%s(%d) got packet failed\n",__FILE__,__LINE__);
                delete pkt;
                pkt = NULL;
                _isParseComplete = true;
                printf("%s(%d)--> maybe playover\n",__FILE__,__LINE__);
                return false;
            }
        }
        readFrameLock.unlock();
        
        //use mutex to protect the queue from be multiplated
        //boost::mutex::scoped_lock lock(_packetsQueueMutex);
        if(pkt->stream_index==_videoStreamId){
            SaveVideoPacket(pkt);
        }
        if(pkt->stream_index==_audioStreamId){
#if ENABLE_SOUND
            SaveAudioPacket(pkt);
#else
            av_free_packet(pkt);
            delete pkt;
            pkt = NULL;
#endif
        }
        //sync the loaded bytes
        _bytesLoaded = _inputStream->tell();
        return true;
    }
    
    bool MediaParserFFmpeg::SaveVideoPacket(AVPacket *pkt){
        boost::mutex::scoped_lock lock(_mutexVideoQueue);
        _videoPacketsQueue.push_back(pkt);
        PauseMediaParserIfNeeded();
        return true;
    }
    
    bool MediaParserFFmpeg::SaveAudioPacket(AVPacket *pkt){
        boost::mutex::scoped_lock lock(_mutexAudioQueue);
        _audioPacketsQueue.push_back(pkt);
        PauseMediaParserIfNeeded();
        return true;
    }
    
    void MediaParserFFmpeg::PauseMediaParserIfNeeded(){
        bool is_parser_complete = IsParseComplete();
        bool is_buffer_full = IsMediaPaserBufferFull();
        if(is_parser_complete || is_buffer_full){
            SetMediaParserState(PARSER_STATE_PARSING);
            PauseMediaParser();
        }
    }
    
    bool MediaParserFFmpeg::IsParseComplete(){
        return _isParseComplete;
    }
    
    void MediaParserFFmpeg::SetMediaParserState(const MediaParserState& state){
        _mediaParserState = state;
    }
    
    bool MediaParserFFmpeg::IsMediaPaserBufferFull(){
        uint64_t buffer_length = GetPacketBufferedLength();
        return buffer_length >= _bufferTime;
    }
    
    uint64_t MediaParserFFmpeg::ConvertTime(double time, AVRational time_base){
        return (uint64_t)(time*av_q2d(time_base)*1000.0);
    }
    
    uint64_t MediaParserFFmpeg::GetPacketDTS(AVPacket* packet, AVRational time_base){
        if(!packet)
            return 0;
        return ConvertTime(packet->dts, time_base);
    }
    
    uint64_t MediaParserFFmpeg::GetPacketBufferedLength(){
        
        
        uint64_t buffer_length = 0;
        uint64_t  videoPacketBufferedLength = 0;
        uint64_t  audioPacketBufferedLength = 0;
        AVPacket *beg, *end;
        AVRational video_time_base, audio_time_base;
        
        video_time_base = _mediaInfo._videoTimeBase;
        audio_time_base = _mediaInfo._audioTimeBase;
        
        bool  is_video_empty = _videoPacketsQueue.empty();
        bool  is_audio_empty = _audioPacketsQueue.empty();
        if(!is_video_empty){
            beg = _videoPacketsQueue.front();
            end = _videoPacketsQueue.back();
            videoPacketBufferedLength = GetPacketDTS(end, video_time_base) - GetPacketDTS(beg, video_time_base);
        }
        
        if(!is_audio_empty){
            beg = _audioPacketsQueue.front();
            end = _audioPacketsQueue.back();
            audioPacketBufferedLength =  GetPacketDTS(end, audio_time_base) - GetPacketDTS(beg, audio_time_base);
        }
        
        if(!is_video_empty && !is_audio_empty){
            buffer_length = std::min(videoPacketBufferedLength, audioPacketBufferedLength);
        }else{
            buffer_length = std::max(videoPacketBufferedLength, audioPacketBufferedLength);
        }
        
        return buffer_length;
    }
    
    void MediaParserFFmpeg::SetBufferTime(uint64_t bufferTime){
        _bufferTime = bufferTime;
    }
    uint64_t MediaParserFFmpeg::GetBufferTime() const {
        return _bufferTime;
    }
    
    uint64_t MediaParserFFmpeg::GetByteLoaded() const {
        return _bytesLoaded;
    }
    
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
    
    
    bool MediaParserFFmpeg::isBufferEmpty() const{
#if ENABLE_SOUND
        return _videoPacketsQueue.empty()&&_audioPacketsQueue.empty();
#else
        return _videoPacketsQueue.empty();
#endif
    }
    
    double MediaParserFFmpeg::videoTimeBase(){
        if(_videoStream)
            return av_q2d(_mediaInfo._videoTimeBase);
        return 0.0;
    }
    double MediaParserFFmpeg::audioTimeBase(){
        if(_audioStream)
            return av_q2d(_mediaInfo._audioTimeBase);
        return 0.0;
    }
    void MediaParserFFmpeg::ResumeMediaParserIfNeeded(){
        SetMediaParserState(PARSER_STATE_PARSING);
        ContinueMediaParser();
    }
    
    void MediaParserFFmpeg::PauseMediaParser(){
        PauseMediaParserAction();
    }
    
    void MediaParserFFmpeg::ContinueMediaParser(){
        ContinueMediaParserAction();
    }
    
    auto_ptr<AVPacket> MediaParserFFmpeg::GetNextEncodedAudioFrame(){
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
        ResumeMediaParserIfNeeded();
        return result;
    }
    auto_ptr<AVPacket> MediaParserFFmpeg::GetNextEncodedVideoFrame(){
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
        ResumeMediaParserIfNeeded();
        return result;
    }
    
    //play control functions
    bool MediaParserFFmpeg::Seek(int64_t millPos){
        boost::mutex::scoped_lock lock(_packetsQueueMutex);
        boost::mutex::scoped_lock readFrameLock(_readFrameMutex);//sync with av_read_frame
        if(millPos<0 || 0/*millPos>_videoDuration*/){
            return false;
        }
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
                    SaveAudioPacket(_seekPacket);
                }else{
                    _realSeekPos = ConvertTime(_seekPacket->pts, _mediaInfo._videoTimeBase);
                    SaveVideoPacket(_seekPacket);
                    break;
                }
            }else{
                break;
            }
        }
        _isParseComplete = false;
        _parserThreadWakeup.notify_all();
        cout<<"ok: seek operate is over"<<endl;
        return true;
    }
    
    int64_t MediaParserFFmpeg::GetKeyFrameFilePosByIndex(int index){
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
    
} // namespace
