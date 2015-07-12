#ifndef VIDEO_DECODER_DELEGATE_H
#define VIDEO_DECODER_DELEGATE_H

namespace MediaCore{
class VideoDecoderDelegate{
public:
	virtual auto_ptr<AVPacket> GetNextEncodedVideoFrame() = 0;
    virtual auto_ptr<AVPacket> GetNextEncodedAudioFrame() = 0;
	virtual MediaParserState GetMediaParserState() = 0;
	virtual double GetVideoTimeBase() = 0;
    virtual double GetAudioTimeBase() = 0;
};

}
#endif