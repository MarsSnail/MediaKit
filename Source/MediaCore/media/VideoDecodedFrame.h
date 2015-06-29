#ifndef VideoDecodedFrame_h
#define VideoDecodedFrame_h

#include <stdint.h>

using MediaCore::VideoImage;
namespace MediaCore {
class VideoDecodedFrame {
public:
	VideoDecodedFrame(VideoImage *image, int64_t pts):
		_image(image),
		_pts(pts){
			//do nothing
		}

	VideoImage *getImage() const { return _image;}
	int64_t getPTS() const {return _pts;}

private:
	VideoImage *_image;
	int64_t _pts;
};

} // namspace
#endif
