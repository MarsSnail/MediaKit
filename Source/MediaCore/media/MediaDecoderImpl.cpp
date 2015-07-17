#include "MediaDecoderImpl.h"
#include "VideoDecoder.h"
#include "AudioDecoder.h"
#include "AudioDecoderFFmpeg.h"
#include "VideoDecoderFFmpeg.h"

namespace MediaCore{
    
    //static
    MediaDecoder* MediaDecoder::CreateMediaDecoder(MediaCore::MediaDecoderDelegate *delegate){
        return new MediaDecoderImpl(delegate);
    }
    
    MediaDecoderImpl::MediaDecoderImpl(MediaDecoderDelegate* delegate)
    : delegate_(delegate)
    , run_(false){
        
    }
    
    MediaDecoderImpl::~MediaDecoderImpl(){
        run_ = false;
        media_decoder_thread->join();
    }
    
    bool MediaDecoderImpl::Init(){
        video_decoder_.reset(new VideoDecoderFFmpeg(delegate_));
        audio_decoder_.reset(new AudioDecoderFFmpeg(delegate_));
        
        if(!video_decoder_->Init())
            return false;
        run_ = true;
        media_decoder_thread.reset(new boost::thread(boost::bind(&MediaDecoderImpl::DecoderLoop, this)));
        
        return true;
    }
    
    boost::shared_ptr<VideoDecodedFrame> MediaDecoderImpl::GetNextDecodedVideoFrame(int64_t timestamp){
        boost::shared_ptr<VideoDecodedFrame> result;
        VideoDecodedFrame* video_decoded_frame = video_decoder_->GetDecodedVideoFrame(timestamp);
        if(video_decoded_frame != 0)
            result.reset(video_decoded_frame);
        
        ResumeDecoderIfNeeded();
        
        return result;
    }
    
    boost::shared_ptr<AudioDecodedFrame> MediaDecoderImpl::GetNextDecodedAudioFrame(int64_t timestamp){
        boost::shared_ptr<AudioDecodedFrame> result;
        AudioDecodedFrame* audio_decoded_frame = audio_decoder_->popAudioDecodedFrame();
        if(audio_decoded_frame != 0)
            result.reset(audio_decoded_frame);
        
        ResumeDecoderIfNeeded();
        
        return result;
    }
    
    void MediaDecoderImpl::ClearDeocdedFrameBuffer(){
        audio_decoder_->clearAudioFrameQueue();
        video_decoder_->clearVideoFrameQueue();
    }
    
    //temp interface
    int MediaDecoderImpl::videoFrameQueueLength(){
        return video_decoder_->videoFrameQueueLength();
    }
    
    auto_ptr<VideoImage> MediaDecoderImpl::getVideoImage(){
        return video_decoder_->getVideoImage();
    }
    
    int64_t MediaDecoderImpl::nextVideoFrameTimestamp(){
        ResumeDecoderIfNeeded();
        return video_decoder_->nextVideoFrameTimestamp();
    }
    
    AudioDecodedFrame* MediaDecoderImpl::popAudioDecodedFrame(){
        return audio_decoder_->popAudioDecodedFrame();
    }
    
    int64_t MediaDecoderImpl::nextAudioFrameTimestamp(){
        ResumeDecoderIfNeeded();
        return audio_decoder_->nextAudioFrameTimestamp();
    }
    
    void MediaDecoderImpl::PauseDecoder(){
        boost::mutex::scoped_lock lock(mutex_);
        decoder_buffer_full_.wait(lock);
        
    }
    
    void MediaDecoderImpl::ResumeDecoder(){
        decoder_buffer_full_.notify_all();
    }
    
    void MediaDecoderImpl::PauseDecoderIfNeeded()
    {
        if(audio_decoder_->IsBufferFull() && video_decoder_->IsBufferFull())
            PauseDecoder();
    }
    
    void MediaDecoderImpl::ResumeDecoderIfNeeded(){
        if(audio_decoder_->IsBufferLowLevel()
           || video_decoder_->IsBufferLowLevel()){
         ResumeDecoder();   
        }
    }
    
    void MediaDecoderImpl::DecodeAVFrame(){
        audio_decoder_->decodeAudioFrame();
        video_decoder_->DecodeVideoFrame();
        PauseDecoderIfNeeded();
    }
    
    void MediaDecoderImpl::DecoderLoop(){
        while (run_) {
            DecodeAVFrame();
        }
    }
    
}