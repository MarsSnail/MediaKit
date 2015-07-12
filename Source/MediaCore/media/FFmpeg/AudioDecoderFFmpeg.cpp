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

#include "AudioDecoderFFmpeg.h"
#include "MediaParserFFmpeg.h"
#include "SnailException.h"
#include "AVPipeline.h"

namespace MediaCore {
    
    AudioDecoderFFmpeg::AudioDecoderFFmpeg(AVPipeline *pipeline):
    AudioDecoder(pipeline),
    _swrCtx(0),
    _audioTimeBase(0){
        if(!init()){
            throw SnailException("ERROR: Create the Audio Decoder(FFmpeg) failed");
        }
        startThread();
    }
    AudioDecoderFFmpeg::~AudioDecoderFFmpeg(){
        cout<<"in the function ~AudioDecoderFFmpeg"<<endl;
        setKillThread();
        clearAudioFrameQueue();
        _audioDecoderThread->join();
        _audioDecoderThread.reset();
        if(_swrCtx){
            swr_free(&_swrCtx);
        }
        
    }
    bool AudioDecoderFFmpeg::init(){
        _audioCodecCtx = global_getAudioCtx();
        _audioCodec = global_getAudioCodec();
        _audioTimeBase = _avpipeline->GetAudioTimeBase();
        if(!_audioCodecCtx || !_audioCodec || !_audioTimeBase){
            return false;
        }else
            return true;
    }
    
    
    void AudioDecoderFFmpeg::pushAudioDecodedFrame(AudioDecodedFrame *frame){
        boost::mutex::scoped_lock lock(queueMutex);
        _audioDecodedFrameQueue.push_back(frame);
        if(queueLength()>=4){
            _decoderThreadWakeup.wait(lock);
        }
    }
    AudioDecodedFrame *AudioDecoderFFmpeg::popAudioDecodedFrame(){
        boost::mutex::scoped_lock lock(queueMutex);
        AudioDecodedFrame *resFrame=NULL;
        if(!queueEmpty()){
            resFrame = _audioDecodedFrameQueue.front();
            _audioDecodedFrameQueue.pop_front();
        }else{
            cout<<"audio decoded frame queue empty"<<endl;
        }
        if(queueLength()<=1){
            _decoderThreadWakeup.notify_all();
        }
        return resFrame;
    }
    int64_t AudioDecoderFFmpeg::nextAudioFrameTimestamp(){
        boost::mutex::scoped_lock lock(queueMutex);
        int64_t nextTimestamp;
        if(queueEmpty()){
            nextTimestamp = -1;
        }else{
            AudioDecodedFrame *tmpAudioFrame= _audioDecodedFrameQueue.front();
            nextTimestamp = tmpAudioFrame->getPTS();
        }
        return nextTimestamp;
    }
    
    int AudioDecoderFFmpeg::queueLength() {
        return _audioDecodedFrameQueue.size();
    }
    
    bool AudioDecoderFFmpeg::queueEmpty() {
        return _audioDecodedFrameQueue.empty();
    }
    //utility functions
    int64_t AudioDecoderFFmpeg::convertTime(double time) const {
        return (int64_t)(time*_audioTimeBase*1000.0);
    }
    
    void AudioDecoderFFmpeg::decodeAudioFrame(){
        auto_ptr<AVPacket> pkt = _avpipeline->GetNextEncodedAudioFrame();
        
        if(!pkt.get() || !pkt->buf->buffer){
            //when play over if as not call the close then block the video Decoder thread
            if(_avpipeline->IsParseComplete()){
                boost::mutex::scoped_lock lock(queueMutex);
                cout<<"now parsed completely and the audio Packet queue is empty,so block the audio decoder Thread"<<endl;
                _decoderThreadWakeup.wait(lock);
            }
            
            return ;
        }
        
        AVFrame *audioFrame = NULL;
        int gotFrame, len;
        gotFrame = len = 0;
        audioFrame = av_frame_alloc();
        if(!audioFrame){
            av_free_packet(pkt.get());
            cout<<__FILE__<<": "<<__LINE__<<"alloc audio frame failed"<<endl;
            return ;
        }
        
        boost::mutex::scoped_lock lock(queueMutex);
        len = avcodec_decode_audio4(_audioCodecCtx, audioFrame, &gotFrame, pkt.get());
        lock.unlock();
        av_free_packet(pkt.get());
        if(!gotFrame){
            cout<<__FILE__<<": "<<__LINE__<<"decode the audio frame failed"<<endl;
            av_frame_free(&audioFrame);
            return ;
        }
        /*int decoded_data_size = av_samples_get_buffer_size(NULL,
         audioFrame->channels,
         audioFrame->nb_samples,
         (enum AVSampleFormat)audioFrame->format, 1);*/
        //**********************convert the sample format **********/
        int wanted_nb_samples = audioFrame->nb_samples;
        if(!_swrCtx){
            swr_free(&_swrCtx);
            _swrCtx = swr_alloc_set_opts(NULL,
                                         _audioCodecCtx->channel_layout, AV_SAMPLE_FMT_S16,audioFrame->sample_rate,
                                         _audioCodecCtx->channel_layout, (enum AVSampleFormat)audioFrame->format,
                                         audioFrame->sample_rate, 0, NULL);
            if(!_swrCtx || swr_init(_swrCtx) < 0){
                cout<<"create the swrContext failed"<<endl;
                av_frame_free(&audioFrame);
                return ;
            }
        }
        if(_swrCtx){
            const uint8_t **in = (const uint8_t **)audioFrame->extended_data;
            int outCount = wanted_nb_samples + 256;
            int outSize = av_samples_get_buffer_size(NULL, audioFrame->channels, outCount, AV_SAMPLE_FMT_S16, 0);
            
            uint8_t  *tmpPtr = NULL;
            uint8_t **out = &tmpPtr;
            *out = new uint8_t[outSize];//(uint8_t*)malloc(sizeof(uint8_t)*outSize);
            memset(*out, 0, outSize);
            if(!(*out)){
                cout<<__FILE__<<": "<<__LINE__<<"ERROR: alloc buffer failed"<<endl;
                av_frame_free(&audioFrame);
                return ;
            }
            int resLen = swr_convert(_swrCtx, out, outCount, in, audioFrame->nb_samples);
            if(resLen < 0){
                cout<<__FILE__<<": "<<__LINE__<<"convert the frame failed"<<endl;
                av_frame_free(&audioFrame);
                return ;
            }
            if(resLen == outCount){
                cout<<"WARNING: audio buffer is probably to small"<<endl;
            }
            int resampledDataSize = resLen * audioFrame->channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            int64_t framePts = convertTime(audioFrame->pkt_pts);
            
            AudioDecodedFrame *newFrame =  new AudioDecodedFrame(*out, resampledDataSize, framePts);
            pushAudioDecodedFrame(newFrame);
            av_frame_free(&audioFrame);
        }//if(_swrCtx)
    }
    
    void AudioDecoderFFmpeg::clearAudioFrameQueue(){
        boost::mutex::scoped_lock lock(queueMutex);
        deque<AudioDecodedFrame*>::iterator iter,end;
        iter = _audioDecodedFrameQueue.begin();
        end = _audioDecodedFrameQueue.end();
        for(; iter!=end; iter++){
            delete(*iter);
        }
        _audioDecodedFrameQueue.clear();
        avcodec_flush_buffers(_audioCodecCtx);
        if(queueLength()<4){
            _decoderThreadWakeup.notify_all();
        }
    }
    
    
} // namespace
