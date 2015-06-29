#ifndef AV_PIPELINE_VIDEO_DELEGATE_H
#define AV_PIPELINE_VIDEO_DELEGATE_H

#include "ImageType.h"

namespace MediaCore{

class AVPipelineDelegate{
public:
	void UpdateVideoFrame(std::auto_ptr<VideoImage> videoImage) { _videoImage = videoImage;}
	std::auto_ptr<VideoImage> GetVideoImage() {return _videoImage;}
private:
	std::auto_ptr<VideoImage>  _videoImage;
};

} //namespace MediaCore
#endif