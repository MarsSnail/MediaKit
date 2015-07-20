#ifndef MEDIA_DECODER_DELEGATE_H
#define MEDIA_DECODER_DELEGATE_H

#include "boost/shared_ptr.hpp"
#include "CommonDef.h"

namespace MediaCore{
    class MediaDecoderDelegate{
    public:
        virtual boost::shared_ptr<EncodedAVFrame> GetNextEncodedAudioFrame() = 0;
        virtual boost::shared_ptr<EncodedAVFrame> GetNextEncodedVideoFrame() = 0;
        virtual double GetAudioTimeBase() = 0;
        virtual double GetVideoTimeBase() = 0;
        virtual bool IsParseComplete() = 0;
        virtual void NotifyMediaDecodeComplete() = 0;
    };
} // namespace MediaCore
#endif