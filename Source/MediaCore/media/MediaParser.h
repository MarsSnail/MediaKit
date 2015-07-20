#ifndef MEDIA_PARSER_H
#define MEDIA_PARSER_H

#include "CommonDef.h"
#include <memory>

class MediaParser{
public:
	virtual ~MediaParser(){}
	virtual bool Init()						= 0;
    virtual boost::shared_ptr<EncodedAVFrame> GetNextEncodedVideoFrame() = 0;
    virtual boost::shared_ptr<EncodedAVFrame> GetNextEncodedAudioFrame() = 0;
	virtual uint64_t GetByteLoaded() const  = 0;
	virtual void 	 PauseMediaParser() 	= 0;
	virtual void 	 ContinueMediaParser() 	= 0;
	virtual uint64_t GetBufferTime() const  = 0;
	virtual void 	 SetBufferTime(uint64_t bufferTime) = 0;
	virtual bool 	 Seek(int64_t millPos) 	= 0;
	virtual int64_t  GetRealSeekPos() const = 0;
	virtual int64_t  GetKeyFrameFilePosByIndex(int index)=0;
};


#endif