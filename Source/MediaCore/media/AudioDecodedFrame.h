#ifndef AudioDecodedFrame_h
#define AudioDecodedFrame_h

namespace MediaCore {
class AudioDecodedFrame{
public:
	AudioDecodedFrame(unsigned char *data=NULL, int64_t size=0, int64_t pts=0):
		_data(data),
		_dataSize(size),
		_pts(pts){
		}
	~AudioDecodedFrame(){
		if(_data){
			delete[] _data;
		}else{
			// do nothing
		}
	}
	unsigned char* data() const { return _data; }
        void resetDataPtr() { _data = NULL; }
	int64_t getPTS() { return _pts; }
	int64_t dataSize() { return _dataSize; }
private:
	unsigned char *_data;
	int64_t _dataSize;
	int64_t _pts;	
};
} // namespace 
#endif // audioDecodedFrame_h
