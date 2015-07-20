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

#ifndef AV_PIPELINE_H_
#define AV_PIPELINE_H_

#include <iostream>
#include <memory>

#include "boost/smart_ptr/scoped_ptr.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/function.hpp"

#include "AudioDecoder.h"
#include "ImageType.h"
#include "MediaParser.h"
#include "PlayControl.h"
#include "SystemClock.h"
#include "SoundHandler.h"
#include "SnailConfig.h"
#include "SoundHandlerSDL.h"
#include "SnailSleep.h"
#include "Timer.h"
#include "VideoDecoder.h"
#include "MediaParserDelegate.h"
#include "MediaDecoderDelegate.h"
#include "MediaDecoder.h"

namespace MediaCore {
    
    class AVPipelineDelegate;
    
    /*this class you create passing a sound handler, which will be used to
     * Implement attach/detach and eventally throw away buffers of sound
     * when no sound handler is given
     */
    class BufferedAudioStreamer {
    public:
        BufferedAudioStreamer();
        void setSoundHandler(SoundHandler *);
        
        // A Buffer with a cursor state
        class CursoredBuffer {
        public:
            CursoredBuffer() : _size(0), _data(NULL), _ptr(NULL) {}
            ~CursoredBuffer() { delete[] _data; }
            
            // number of smaples left in buffer starting from cursor
            uint32_t _size;
            // the data must be allocated with new [] as will be delete []
            uint8_t *_data;
            uint8_t *_ptr;  // cursor into the data
        };
        
        // typedef boost::ptr_deque<CursoredBuffer> AudioQueue;
        typedef deque<CursoredBuffer *> AudioQueue;
        
        void cleanAudioQueue();
        SoundHandler
        *_soundHandler;  // this object only exist one instance int the pid
        AudioQueue _audioQueue;
        // bytes in the audioQueue
        size_t _audioQueueSize;
        boost::mutex _audioQueueMutex;
        // id of an attached audio streamer, 0 if none
        InputStream *_auxStreamer;
        // on success, _auxStreamerAttached will be set to true.
        // won't attach again if already attached.
        void attachAuxStreamer();
        void detachAuxStreamer();
        
        // fetch samples from the audio queue
        uint16_t fetch(int16_t *samples, uint16_t nSamples, bool &eof);
        static uint16_t fetchWrapper(void *owner, int16_t *samples, uint16_t nSamples,
                                     bool &eof);
        void push(CursoredBuffer *audio);
    };
    class AVPipeline : public MediaParserDelegate,
    public MediaDecoderDelegate {
    public:
        AVPipeline();
        ~AVPipeline();
        
        void Init();
        void Reset();
        void SetDelegate(boost::shared_ptr<AVPipelineDelegate> avPipelineDelegate);
        void PostTask(AsyncTask task);
        void beginDisplay();
        
        // play control func
        bool startPlayback(string url = "bt://bt");
        void Seek(int32_t secPos);
        void pause();
        void resume();
        
        // MediaParserDelegate implementation
        virtual void SetMediaInfo(const MediaInfo &mediaInfo) override;
        virtual void SetMediaParserState(const MediaParserState &state) override;
        
        // MediaDecoderDelegate implementation
        virtual boost::shared_ptr<EncodedAVFrame> GetNextEncodedVideoFrame() override;
        virtual boost::shared_ptr<EncodedAVFrame> GetNextEncodedAudioFrame() override;
        virtual double GetVideoTimeBase() override;
        virtual double GetAudioTimeBase() override;
        virtual bool IsParseComplete() override;
        virtual void NotifyMediaDecodeComplete() override;
        
        // test Async Task Module
        void AsyncTaskTest();
        void TestFunc();
        
    private:
        // utils
        bool CreateMediaParser(std::string url, MediaParserDelegate *delegate);
        bool CreateMediaDecoder();
        
        void PlaybackAction(string url);
        void SeekAction(int32_t pos);
        void PauseAction();
        
        void SetPlayState(PlayState state);
        PlayState GetPlayState();
        
        void refreshVideoFrame(bool paused = false);
        void refreshAudioFrame(bool paused = false);
        void pushDecodedAudioFrames(int64_t ts);
        BufferedAudioStreamer::CursoredBuffer *getNextAudioDecodedFrame();
        uint64_t GetStartPlayTimestamp();
        
        boost::scoped_ptr<MediaParser> _mediaParser;
        boost::scoped_ptr<MediaDecoder> _mediaDecoder;
        static boost::scoped_ptr<MediaCore::SoundHandler> _soundHandler;
        SystemClock _sysClock;
        MediaCore::InterruptableVirtualClock _playbackClock;
        PlayControl _playHead;
        
        MediaInfo _mediaInfo;
        MediaParserState _parserState;
        
        // the time of lenght that bufferd video or audio packets
        uint32_t _bufferTime;
        PlayState play_state_;
        bool decode_complete_;
        // process audio data
        BufferedAudioStreamer _audioStreamer;
        string _url;
        int _dropedFrameCnt;
        bool _seekFlag;
        bool _seekFlagForRefresh;
        int _id;
        // when as call Close() function(this function wiil destruct the _aduioDecoder
        // ..),
        // but the beginDisplay is excutting ,,it will use the _audioDecoder.so this
        // will cause the crash
        boost::mutex _mutex;
        int64_t _seekPos;
        boost::shared_ptr<AVPipelineDelegate> _avPipelineDelegate;
        boost::scoped_ptr<boost::thread> _task_thread;
        boost::scoped_ptr<boost::asio::io_service::work> _io_service_work;
        boost::scoped_ptr<boost::asio::io_service> _io_service;
    };
    
}  // namespace

#endif /* PLAYERSTREAM_H_ */
