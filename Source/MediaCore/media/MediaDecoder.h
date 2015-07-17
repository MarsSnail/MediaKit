//
//  MediaDecoder.h
//  MediaKit
//
//  Created by 李楠 on 15/7/6.
//
//

#ifndef MEDIA_DECODER_H
#define MEDIA_DECODER_H

#include "boost/shared_ptr.hpp"
#include <memory>
#include "VideoDecoder.h"

namespace MediaCore{
    class VideoDecodedFrame;
    class AudioDecodedFrame;
    
    class MediaDecoder{
    public:
        static MediaDecoder* CreateMediaDecoder(MediaDecoderDelegate* delegate);
        virtual ~MediaDecoder() {}
        virtual bool Init() = 0;
        virtual boost::shared_ptr<VideoDecodedFrame> GetNextDecodedVideoFrame(int64_t timestamp) = 0;
        virtual boost::shared_ptr<AudioDecodedFrame> GetNextDecodedAudioFrame(int64_t timestamp) = 0;
        virtual void ClearDeocdedFrameBuffer() = 0;
        
        //temp interface
        virtual int videoFrameQueueLength() = 0;
        virtual auto_ptr<VideoImage> getVideoImage() = 0;
        virtual int64_t nextVideoFrameTimestamp() = 0;
        virtual AudioDecodedFrame* popAudioDecodedFrame()=0;
        virtual int64_t nextAudioFrameTimestamp() = 0;
        
    };
} // namespace MediaCore

#endif /* defined(__MediaKit__MediaDecoder__) */
