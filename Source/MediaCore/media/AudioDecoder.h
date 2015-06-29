/*
 * Copyright (C) 2013, 2014 Mark Li (lixiaonan06@163.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef AUDIODECODER_H_
#define AUDIODECODER_H_
#include "SnailConfig.h"
#include "MediaParser.h"
#include <memory>
#include <stdint.h>
#include <iostream>
#include "AudioDecodedFrame.h"

using MediaCore::AudioDecodedFrame;
using std::cout;
using std::cin;
using std::endl
;
namespace MediaCore {
/*
class AudioDecodedFrame{
public:
	AudioDecodedFrame(unsigned char* data=NULL,uint64_t size=0, uint64_t pts=0):
		_data(data),
		_dataSize(size),
		_pts(pts)
		{

	}
	~AudioDecodedFrame(){
		if(_data){
			delete[] _data;
		}else{
		}
	}
	unsigned char* data() const {
		return _data;
	}
	void resetDataPtr() {
		_data = NULL;
	}
	uint64_t timestamp()const {
		return _pts;
	}
	uint64_t dataSize()const {
		return _dataSize;
	}
private:
	unsigned char * _data;
	uint64_t _dataSize;
	uint64_t _pts;
};//class AudioDecodedFrame
*/
class AudioDecoder{
public:
	AudioDecoder(MediaParser *mediaParser);
	virtual ~AudioDecoder();

	virtual AudioDecodedFrame* popAudioDecodedFrame()=0;
	virtual int64_t nextAudioFrameTimestamp() = 0;
	virtual void clearAudioFrameQueue() = 0;

protected:
	//the functions work for decoder thread
	void startThread();
	static void decoderLooperStarter(AudioDecoder *audioDecoder);
	void decoderLoop();
	virtual void decodeAudioFrame() =0; //main funtion for thread main loop
	//control the thread life time
	void  setKillThread();
	bool continueRun();

protected:
	//audio decoder thread
	MediaParser *_mediaParser;
	auto_ptr<boost::thread> _audioDecoderThread;
	boost::barrier _audioDecoderThreadBarrier;
	bool _killThreadFlag;
};//class AudioDecoder

} // namespace

#endif /* AUDIODECODER_H_ */
