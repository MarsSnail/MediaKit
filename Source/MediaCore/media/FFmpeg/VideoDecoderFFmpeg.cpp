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

#include "VideoDecoderFFmpeg.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include "AVPipeline.h"
#include "SnailException.h"
#include "MediaParserFFmpeg.h"
#include "VideoDecoderDelegate.h"

using namespace std;

namespace MediaCore {
    
    VideoDecoder* VideoDecoder::CreateVideoDecoder(MediaDecoderDelegate* delegate){
        return new VideoDecoderFFmpeg(delegate);
    }
    
    VideoDecoderFFmpeg::VideoDecoderFFmpeg(MediaDecoderDelegate *delegate):
    delegate_(delegate),
    _videoCodecCtx(0),
    _videoCodec(0),
    _swsCtx(0),
    _videoTimeBase(0),
    _videoFrameCount(0),
    _audioCodecCtx(0),
    _audioCodec(0),
    _swrCtx(0),
    _audioTimeBase(0){
    }
    
    VideoDecoderFFmpeg::~VideoDecoderFFmpeg(){
        if(_swsCtx){
            sws_freeContext(_swsCtx);
            _swsCtx = NULL;
        }
    }
    
    bool VideoDecoderFFmpeg::Init(){
        _videoCodecCtx = global_getVideoCtx();
        _videoCodec = global_getVideoCodec();
        if(!_videoCodecCtx || !_videoCodec){
            return false;
        }
        if(!delegate_)
            return false;
        _videoTimeBase = delegate_->GetVideoTimeBase();
        if(!_videoTimeBase){
            return false;
        }
        return true;
    }
    
    bool VideoDecoderFFmpeg::DecodeVideoFrame(){
        if(!delegate_)
            return false;
        if(IsBufferFull())
            return false;
        auto_ptr<AVPacket> pkt = delegate_->GetNextEncodedVideoFrame();
        
        if(!pkt.get()){
            if(delegate_->IsParseComplete()){
                boost::mutex::scoped_lock lock(_framesQueueMutex);
                cout<<"now parsed completely and the video Packet queue is empty,so block the video decoder Thread"<<endl;
                _decoderThreadWakeup.wait(lock);
            }
            return false;
        }
        
        AVFrame *videoFrame = NULL;
        int gotFrame ,len;
        gotFrame = len = 0;
        videoFrame = av_frame_alloc();
        if(!videoFrame){
            printf("<lxn>%s(%d)-->alloc the video avFrame failed\n",__FILE__,__LINE__);
            av_free_packet(pkt.get());
            return false;
        }
        boost::mutex::scoped_lock lock(_framesQueueMutex);
        len = avcodec_decode_video2(_videoCodecCtx, videoFrame, &gotFrame, pkt.get());
        lock.unlock();
        
        //free the packet's memory
        av_free_packet(pkt.get());
        
        if(!gotFrame){
            //printf("<lxn>%s(%d) decode the video frame failed\n",__FILE__,__LINE__);
            return false;
        }
        pushDecodedVideoFrame(videoFrame);
        return true;
    }
    
    //public interface
    int VideoDecoderFFmpeg::videoFrameQueueLength(){
        boost::mutex::scoped_lock lock(_framesQueueMutex);
        return queueSize();
    }
    
    bool VideoDecoderFFmpeg::IsBufferFull(){
        std::cout<<"Video queue Length:"<<queueSize()<<std::endl;
        return queueSize() >= 12;
    }
    
    bool VideoDecoderFFmpeg::IsBufferLowLevel(){
        return queueSize() <= 4;
    }
    
    VideoDecodedFrame* VideoDecoderFFmpeg::GetDecodedVideoFrame(int64_t timestamp){
        return popFrame();
    }
    
    //function for operate the video decoded frame queue
    int VideoDecoderFFmpeg::queueSize(){
        return _videoDecodedFramesQueue.size();
    }
    bool VideoDecoderFFmpeg::queueEmpty(){
        return _videoDecodedFramesQueue.empty();
    }
    void VideoDecoderFFmpeg::pushFrame(VideoDecodedFrame *frame){
        _videoDecodedFramesQueue.push_back(frame);
    }
    VideoDecodedFrame *VideoDecoderFFmpeg::popFrame(){
        VideoDecodedFrame *frame = _videoDecodedFramesQueue.front();
        _videoDecodedFramesQueue.pop_front();
        return frame;
    }
    void VideoDecoderFFmpeg::pushDecodedVideoFrame(AVFrame* frame){
        boost::mutex::scoped_lock lock(_framesQueueMutex);
#if 0
        pushFrame(frame);
#endif
#if ENABLE_DEBUG
        cout<<"#####VideodecodedFrame count = "<<_videoFrameCount<<"queueSize= "<<queueSize()<<endl;
#endif
        int64_t framePts = convertTime(frame->pkt_pts);
        VideoImage *frameImage;
#if USE_YUV
        frameImage = saveYuv(frame);
#elif USE_YUV_RGB
        frameImage = saveYuv(frame);
#else
        frameImage = yuvToRgb(frame);
#endif
        VideoDecodedFrame *newFrame = new VideoDecodedFrame(frameImage, framePts);
#if ENABLE_DEBUG
        _videoFrameCount++;
#endif
        pushFrame(newFrame);
    }
    
    
    VideoDecodedFrame* VideoDecoderFFmpeg::popDecodedVideoFrame(){
        boost::mutex::scoped_lock lock(_framesQueueMutex);
        VideoDecodedFrame *frame=NULL;
        if(!queueEmpty()){
            frame= popFrame();
        }else{
            cout<<"queue empty"<<endl;
        }
        return frame;
    }
    VideoImage *VideoDecoderFFmpeg::saveYuv(AVFrame *frame){
        VideoImage *reImage = NULL;
        if(frame){
            int width = _videoCodecCtx->width;
            int height  = _videoCodecCtx->height;
            reImage = new VideoImage(width, height,IMAGE_YUV);
#ifdef USE_YUV
            /*
             unsigned char *sorCursor, *destCursor;
             
             sorCursor = frame->data[0];
             destCursor = reImage->_yuvData;
             for(int i=0; i< height; i++){
             memcpy(destCursor,sorCursor,width);
             destCursor += i*width;
             sorCursor  += i*frame->linesize[0];
             }
             
             //get u data
             sorCursor = frame->data[1];
             int uHeight = height>>1;
             int uWidth  = width>>1;
             for(int i=0; i<uHeight; i++){
             memcpy(destCursor, sorCursor, uWidth);
             destCursor += i*uWidth;
             sorCursor += i*frame->linesize[1];
             }
             
             //get v data
             sorCursor = frame->data[2];
             int vHeight = height>>1;
             int uWidth  = width>>1;
             for(int i=0; i<uHeight; i++){
             memcpy(destCursor, sorCursor, vWidth);
             destCursor += i*vWidth;
             sorCursor += i*frame->linesize[2];
             }*/
            unsigned char *s,*d;
            s = frame->data[0];
            if(s){
                d = reImage->_yuvData[0];
                for(int i=0; i<height; i++){
                    memcpy(d, s, reImage->_yuvStride[0]);
                    s = s + frame->linesize[0];
                    d = d + reImage->_yuvStride[0];
                }
            }
            int uvHeight = height>>1;
            if(frame->data[1]&&frame->data[2]){
                for(int i=0; i<uvHeight; i++){
                    memcpy(reImage->_yuvData[1]+i*reImage->_yuvStride[1], frame->data[1]+i*frame->linesize[1], reImage->_yuvStride[1]);
                    memcpy(reImage->_yuvData[2]+i*reImage->_yuvStride[2], frame->data[2]+i*frame->linesize[2], reImage->_yuvStride[2]);
                }
            }
#elif USE_YUV_RGB
            //yuv423->yuv
            for(int i=0; i<height; i++){
                unsigned char *dstCur, *surCur1, *surCur2, *surCur3;
                dstCur = reImage->_yuv+i*width*3;
                surCur1 = frame->data[0]+i*frame->linesize[0];
                surCur2 = frame->data[1]+(i>>1)*frame->linesize[1];
                surCur3 = frame->data[2]+(i>>1)*frame->linesize[2];
                for(int j=0; j<width;){
                    int dstStep = j*3;
                    int surStep = j>>1;
                    *(dstCur+dstStep)   = *(surCur1+j);
                    *(dstCur+dstStep+1) = *(surCur2+surStep);
                    *(dstCur+dstStep+2) = *(surCur3+surStep);
                    j++;
                }
            }
#endif
            reImage->_pts = convertTime(frame->pkt_pts);
            av_frame_free(&frame);
        }
        
        return reImage;
    }
    VideoImage * VideoDecoderFFmpeg::yuvToRgb(AVFrame* frame){
        AVFrame *videoFrame = NULL;
        videoFrame = frame;
        VideoImage *reImage = NULL;
        
        if(videoFrame){
            PixelFormat srcPixFmt = _videoCodecCtx->pix_fmt;
            int result, bufSize;
            result = bufSize = 0;
            const int w = _videoCodecCtx->width;
            const int h = _videoCodecCtx->height;
            
            PixelFormat desPixFmt = PIX_FMT_RGB24;
            if(!_swsCtx){
                _swsCtx = sws_getContext(w, h, srcPixFmt, w, h, desPixFmt,
                                         SWS_BILINEAR, NULL, NULL, NULL);
            }
            if(!_swsCtx){
                cout<<__FILE__<<"("<<__LINE__<<")"<<"   Create the swsContext failed"<<endl;
                goto fail;
            }
            bufSize = avpicture_get_size(desPixFmt, w, h);
            reImage = new VideoImage(w, h, IMAGE_RGB);
            reImage->_pts = convertTime(videoFrame->pkt_pts);
            AVPicture picture;
            
            avpicture_fill(&picture, reImage->_data, desPixFmt, w, h);
            
            result = sws_scale(_swsCtx, videoFrame->data, videoFrame->linesize,
                               0, h, picture.data, picture.linesize);
            if(result == -1){
                cout<<"conver the image failed"<<endl;
                goto fail;
            }
            av_frame_free(&videoFrame);
            return reImage;
        }//if(videoFrame)
        else{
            cout<<"decoded video frame queue empty"<<endl;
            return reImage;
        }
    fail:
        av_frame_free(&videoFrame);
        return reImage;
        
    }
    auto_ptr<VideoImage> VideoDecoderFFmpeg::getVideoImage()  {
        
        auto_ptr<VideoImage> reImage;
        VideoDecodedFrame * nextFrame = popDecodedVideoFrame();
        if(nextFrame){
            reImage.reset(nextFrame->getImage());
            delete nextFrame; //free the VideoDecodedFrame
#if ENABLE_DEBUG
            _videoFrameCount--;
#endif
        }
        return reImage;
#if 0
        AVFrame *videoFrame = NULL;
        videoFrame = popDecodedVideoFrame();
        auto_ptr<VideoImage> reImage;
#if 1
        if(videoFrame){
            PixelFormat srcPixFmt = _videoCodecCtx->pix_fmt;
            int result, bufSize;
            result = bufSize = 0;
            const int w = _videoCodecCtx->width;
            const int h = _videoCodecCtx->height;
            
            PixelFormat desPixFmt = PIX_FMT_RGB24;
            if(!_swsCtx){
                _swsCtx = sws_getContext(w, h, srcPixFmt, w, h, desPixFmt,
                                         SWS_BILINEAR, NULL, NULL, NULL);
            }
            if(!_swsCtx){
                cout<<__FILE__<<"("<<__LINE__<<")"<<"   Create the swsContext failed"<<endl;
                goto fail;
            }
            bufSize = avpicture_get_size(desPixFmt, w, h);
            reImage.reset(new VideoImage(w, h, IMAGE_RGB));
            reImage->_pts = convertTime(videoFrame->pkt_pts);
            AVPicture picture;
            
            avpicture_fill(&picture, reImage->_data, desPixFmt, w, h);
            
            result = sws_scale(_swsCtx, videoFrame->data, videoFrame->linesize,
                               0, h, picture.data, picture.linesize);
            if(result == -1){
                cout<<"conver the image failed"<<endl;
                goto fail;
            }
            av_free(videoFrame);
            return reImage;
        fail:
            av_free(videoFrame);
            return reImage;
        }//if(videoFrame)
        else{
            cout<<"decoded video frame queue empty"<<endl;
            return reImage;}
#endif
#endif
    }
    //utility functions
    int64_t VideoDecoderFFmpeg::convertTime(double time) const {
        return (int64_t)(time*_videoTimeBase*1000.0);
    }
    
    int64_t VideoDecoderFFmpeg::nextVideoFrameTimestamp(){
        boost::mutex::scoped_lock lock(_framesQueueMutex);
        if(_videoDecodedFramesQueue.empty()){
            std::cout<<"video frame queue is Empty"<<std::endl;
            return -1;
        }
        VideoDecodedFrame *videoFrame = _videoDecodedFramesQueue.front();
        //int64_t res = convertTime(videoFrame->pkt_pts);
        int64_t res = videoFrame->getPTS();
        return res;
    }
    void VideoDecoderFFmpeg::clearVideoFrameQueue(){
        boost::mutex::scoped_lock lock(_framesQueueMutex);
        deque<VideoDecodedFrame*>::iterator iter, end;
        iter = _videoDecodedFramesQueue.begin();
        end = _videoDecodedFramesQueue.end();
        for(; iter!=end; iter++){
            delete((*iter)->getImage());
            delete(*iter);
#if ENABLE_DEBUG
            _videoFrameCount--;
#endif
        }
        _videoDecodedFramesQueue.clear();
        avcodec_flush_buffers(_videoCodecCtx);

    }
} // namespace
