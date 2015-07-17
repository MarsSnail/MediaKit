#ifndef MEDIA_DECODER_DELEGATE_H
#define MEDIA_DECODER_DELEGATE_H

#include <memory>

#include "CommonDef.h"

namespace MediaCore{
    class MediaDecoderDelegate{
    public:
        virtual std::auto_ptr<EncodedAVFrame> GetNextEncodedAudioFrame() = 0;
        virtual std::auto_ptr<EncodedAVFrame> GetNextEncodedVideoFrame() = 0;
        virtual double GetAudioTimeBase() = 0;
        virtual double GetVideoTimeBase() = 0;
        virtual bool IsParseComplete() = 0;
    };
} // namespace MediaCore
#endif