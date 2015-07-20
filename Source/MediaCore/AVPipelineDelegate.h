#ifndef AV_PIPELINE_VIDEO_DELEGATE_H
#define AV_PIPELINE_VIDEO_DELEGATE_H

#include "ImageType.h"
#include "boost/shared_ptr.hpp"

namespace MediaCore{

class AVPipelineDelegate{
public:
    void UpdateVideoFrame(boost::shared_ptr<VideoImage> videoImage) { _videoImage = videoImage;}
    boost::shared_ptr<VideoImage> GetVideoImage() {return _videoImage;}
private:
    boost::shared_ptr<VideoImage>  _videoImage;
};

} //namespace MediaCore
#endif