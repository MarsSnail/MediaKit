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

#include "AudioDecoder.h"


namespace MediaCore {

AudioDecoder::AudioDecoder(MediaParser *mediaParser):
		_mediaParser(mediaParser),
		_audioDecoderThread(0),
		_audioDecoderThreadBarrier(2),
		_killThreadFlag(false){

}
AudioDecoder::~AudioDecoder(){
cout<<"in the function ~AudioDecoder"<<endl;
}
void AudioDecoder::startThread(){
	_audioDecoderThread.reset(new boost::thread(
			boost::bind(decoderLooperStarter,this)));
	_audioDecoderThreadBarrier.wait();
}
//static function
void AudioDecoder::decoderLooperStarter(AudioDecoder *audioDecoder){
	audioDecoder->decoderLoop();
}

void AudioDecoder::decoderLoop(){
	_audioDecoderThreadBarrier.wait();
	while(continueRun()){
		decodeAudioFrame();
	}
}

bool AudioDecoder::continueRun(){
	return !_killThreadFlag;
}

void AudioDecoder::setKillThread(){
	_killThreadFlag = true;
}


} // namespace
