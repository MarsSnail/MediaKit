
#ifndef MEDIA_DECODER_IMPL_H
#define MEDIA_DECODER_IMPL_H

#include "MediaDecoder.h"
#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/condition.hpp"

namespace MediaCore{
    class VideoDecoder;
    class AudioDecoder;
    class MediaDecoderDelegate;
    
    class MediaDecoderImpl : public MediaDecoder{
    public:
        MediaDecoderImpl(MediaDecoderDelegate* delegate);
        ~MediaDecoderImpl();
        virtual bool Init() override;
        virtual boost::shared_ptr<VideoImage> GetNextDecodedVideoFrame(int64_t timestamp) override;
        virtual boost::shared_ptr<AudioDecodedFrame> GetNextDecodedAudioFrame(int64_t timestamp) override;
        virtual void ClearDeocdedFrameBuffer();
        
        //temp interface
        virtual int videoFrameQueueLength();
        virtual int64_t nextVideoFrameTimestamp();
        virtual AudioDecodedFrame* popAudioDecodedFrame();
        virtual int64_t nextAudioFrameTimestamp();
    
    private:
        void DecoderLoop();
        void DecodeAVFrame();
        void PauseDecoderIfNeeded();
        void ResumeDecoderIfNeeded();
        void PauseDecoder();
        void ResumeDecoder();
        
        bool run_;
        boost::scoped_ptr<VideoDecoder> video_decoder_;
        boost::scoped_ptr<AudioDecoder> audio_decoder_;
        MediaDecoderDelegate* delegate_;
        boost::scoped_ptr<boost::thread> media_decoder_thread;
        boost::mutex mutex_;
        boost::condition decoder_buffer_full_;
        
    };
}

#endif