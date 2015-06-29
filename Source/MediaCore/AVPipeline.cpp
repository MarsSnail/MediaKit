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

#include "SnailConfig.h"
#include "AVPipeline.h"
#include "Url.h"
#include "InputStreamProvider.h"
#include "MediaParserFFmpeg.h"
#include "VideoDecoderFFmpeg.h"
#include "AudioDecoderFFmpeg.h"
//#include "SoundHandlerSDL.h"
#include "ClockTime.h"
#include "SnailException.h"
#include "SnailSleep.h"
#include "AudioDecodedFrame.h"
#include "AVPipelineDelegate.h"

namespace MediaCore {

boost::scoped_ptr<SoundHandler>  AVPipeline::_soundHandler ;
	AVPipeline::AVPipeline():
			_mediaParser(NULL),
			_videoDecoder(NULL),
			_audioDecoder(NULL),
			_sysClock(),
			_playbackClock(_sysClock),
			_playHead(&_playbackClock),
			_bufferTime(100),
			_decodingState(DEC_DECODEPAUSE),
			_statusCode(bufferEmpty),
			_imageFrame(NULL),
			_playOver(false),
			_dropedFrameCnt(0),
			_seekFlag(false),
			_id(0)
{
		_id = 0;
		cout<<endl;
		cout<<"Instance ID="<<_id<<endl;
		cout<<endl;
#if ENABLE_SOUND
	if(!_soundHandler.get()){
		_soundHandler.reset(new SoundHandlerSDL());
	}
	_audioStreamer.setSoundHandler(_soundHandler.get());
#else
	cout<<"***************************************"<<endl;
	cout<<"***************************************"<<endl;
	cout<<"***********Sound Disabled***************"<<endl;
	cout<<"***************************************"<<endl;
	cout<<"***************************************"<<endl;
#endif
	}

AVPipeline::~AVPipeline(){
	Reset();
}

void AVPipeline::Init(){
	_io_service.reset(new boost::asio::io_service());
	_io_service_work.reset(new boost::asio::io_service::work(*_io_service.get()));
	_task_thread.reset(new boost::thread
		(boost::bind(&boost::asio::io_service::run, _io_service.get())));
}

void AVPipeline::PostTask(AsyncTask task){
	assert(_io_service.get());
	_io_service->post(task);
}

void AVPipeline::AsyncTaskTest()
{
	AsyncTask task = boost::bind(&AVPipeline::TestFunc, this);
	PostTask(task);
}

void AVPipeline::TestFunc(){
	static int  taskcount = 0;
	std::cout<<">>>>>>>>>>>TestFunc:"<<taskcount++<<std::endl;
}

void AVPipeline::SetDelegate(boost::shared_ptr<AVPipelineDelegate>  delegate){
	_avPipelineDelegate = delegate;
}

bool AVPipeline::createVideoDecoder(){
	_videoDecoder.reset(new VideoDecoderFFmpeg(this));
	_playHead.setVideoConsumerAvailable();
	return true;
}
bool AVPipeline::createAudioDecoder(){
	_audioDecoder.reset(new AudioDecoderFFmpeg(_mediaParser.get()));
	_playHead.setAudioConsumerAvailable();
	return true;
}
bool AVPipeline::startPlayback(string url){
    AsyncTask playbackTask = boost::bind(&AVPipeline::PlaybackAction, this, url);
    PostTask(playbackTask);
}
    
void AVPipeline::PlaybackAction(string url){
    
    Reset();
    _url = url;
    Url newUrl(url);
    
    try{
        _mediaParser.reset(new MediaParserFFmpeg(_url, this) );
    }catch(SnailException& ex){
        cout<<__FILE__<<"("<<__LINE__<<")"<<"---->"<<ex.what()<<endl;
        return ;
    }
    if(!_mediaParser.get()){
        cout<<__FILE__<<"("<<__LINE__<<")"<<"---->"<<"mediaParser create falied"<<endl;
        return ;
    }else{
        cout<<"OK: MediaParser create success"<<endl;
    }
    if(!_videoDecoder.get()){
        try{
            createVideoDecoder();
        }catch(SnailException& ex){
            cout<<__FILE__<<"("<<__LINE__<<")"<<"---->"<<ex.what()<<endl;
            return ;
        }
    }
#if ENABLE_SOUND
    if(!_audioDecoder.get()){
        try{
            createAudioDecoder();
        }catch(SnailException& ex){
            cout<<__FILE__<<":"<<ex.what()<<endl;
            return ;
        }
    }
#endif
    _playOver = false; //must be do this befor act the function of _playHead.seekTo(0)
    _mediaParser->setBufferTime(_bufferTime);
    cout<<"=========> BufferTimeLenght = "<<_bufferTime<<"<==========="<<endl;
    decodingState(DEC_DECODEPAUSE);
    _playbackClock.pause();
    
    _playHead.seekTo(0);
    _playHead.setState(PlayControl::PLAY_PLAYING);
    setStatus(playStart);
#if ENABLE_SOUND
    _audioStreamer.attachAuxStreamer();
#endif
    return ;
}
    
void AVPipeline::beginDisplay(){
	//sync with the function of close().
	boost::mutex::scoped_lock lock(_mutex);

	if(_seekFlag)	return ;
	if(!_mediaParser.get()||!_videoDecoder.get()) 	return ;
#if ENABLE_SOUND
	if(!_audioDecoder.get())		return ;
#endif
	if(decodingState()==DEC_STOP){
		_playOver = true;
		cout<<"now  play stop!"<<endl;
		return ;
	}
	size_t videoFrameQueueSize = _videoDecoder->videoFrameQueueLength();

	//switch from DEC_DECODING to DEC_DECPAUSE
	if(decodingState()==DEC_DECODING &&videoFrameQueueSize==0){
		decodingState(DEC_DECODEPAUSE);
		_playbackClock.pause();
		cout<<endl;
		cout<<"now  switch from DEC_DECODING to DEC_DECPAUSE and pause PlaybackClock"<<endl;
		cout<<endl;
		return ;
	}
    //when state is DEC_DECODEPAUSE,process the play actions
	if(decodingState()==DEC_DECODEPAUSE){
		if(videoFrameQueueSize==0){
			cout<<endl;
			cout<<"stay the state in DEC_DECODEPAUSE and bufferLen >=0"<<endl;
			cout<<endl;
			_playOver = _videoDecoder->IsDecodeComplete();
			return ;
		}else{
			decodingState(DEC_DECODING);
			_playbackClock.resume();
			cout<<endl;
			cout<<"now  switch from  DEC_DECPAUSE to DEC_DECODING and resume PlaybackClock"<<endl;
			cout<<endl;
		}				
	}

	uint64_t curPosition = _playHead.getPosition();
	if(0 == curPosition){
		_playHead.seekTo(StartPlayPosition());
	}
	refreshVideoFrame();
#if ENABLE_SOUND
	refreshAudioFrame();
#endif
	//Advance PlayControl position if current one decoded frame was
	// consumed by all available consumers
	_playHead.advanceIfConsumed();
}

uint64_t AVPipeline::StartPlayPosition(){
		uint64_t start_position;

#if ENABLE_SOUND
		uint64_t videoBeginPosition =  _videoDecoder->nextVideoFrameTimestamp();
		uint64_t audioBeginPosition = _audioDecoder->nextAudioFrameTimestamp();
		start_position = std::min(videoBeginPosition, audioBeginPosition);
#else
		start_position = _videoDecoder->nextVideoFrameTimestamp();
#endif
		return start_position;
}
void AVPipeline::refreshVideoFrame(bool paused){
	assert(_mediaParser.get());
	if(!paused && _playHead.getState()==PlayControl::PLAY_PAUSED){
		return ;
	}
	if(_playHead.isVideoConsumed())
	{
		return;
		}

	//get the current time
	uint64_t curPos = _playHead.getPosition();
	auto_ptr<VideoImage> videoImage(NULL);
	videoImage = getDecodedVideoFrame(curPos);
	if(videoImage.get()){
		if(_avPipelineDelegate.get()){
			_avPipelineDelegate->UpdateVideoFrame(videoImage);
		}
	}
	_playHead.setVideoConsumed();
}

void AVPipeline::refreshAudioFrame(bool paused){
	assert(_audioDecoder.get());
	if(_playHead.getState() == PlayControl::PLAY_PAUSED){
		return ;
	}
	if(_playHead.isAudioConsumed()){
		return;
	}
	uint64_t curPos = _playHead.getPosition();
	pushDecodedAudioFrames(curPos);
}
void AVPipeline::pushDecodedAudioFrames(int64_t ts){
	bool consumed = false;
	int64_t nextTimestamp = -1;
	while(1){
		boost::mutex::scoped_lock lock(_audioStreamer._audioQueueMutex);
		float tmpFps = 25;
		double msecsPreAdvance = 10000/tmpFps;
		const uint16_t bufferLimit = 20;
		uint16_t bufferSize = _audioStreamer._audioQueue.size();
		if(bufferSize > bufferLimit){
			_playbackClock.pause();
			return ;
		}
		lock.unlock();
		bool parsingComplete = _mediaParser->parseComplete();
		nextTimestamp = _audioDecoder->nextAudioFrameTimestamp();
		if(nextTimestamp==-1){
			if(parsingComplete){
				consumed = true;
				if(_mediaParser->isBufferEmpty()){
					decodingState(DEC_STOP);
					setStatus(playStop);
				}
				break;
			}//if(parsingComplete)
			break;
		}//if(!nextTimestamp)
		if(nextTimestamp > ts){
			consumed = true;
			if(nextTimestamp > ts+(int64_t)msecsPreAdvance){
				break;
			}
		}
		BufferedAudioStreamer::CursoredBuffer *audio = getNextAudioDecodedFrame();
		if(!audio){
			break;
		}
		if(!audio->_size){
			delete audio;
			continue;
		}
		_audioStreamer.push(audio);
		}//while(1)

	if(consumed){
		assert(decodingState() != DEC_DECODEPAUSE);
		_playbackClock.resume();
		_playHead.setAudioConsumed();
	}
	//_playHead.setAudioConsumed();
}
BufferedAudioStreamer::CursoredBuffer* AVPipeline::getNextAudioDecodedFrame(){
	assert(_audioDecoder.get());
	AudioDecodedFrame *nextAudioFrame=NULL;
	nextAudioFrame = _audioDecoder->popAudioDecodedFrame();
	if(nextAudioFrame){
		BufferedAudioStreamer::CursoredBuffer *raw =
				new BufferedAudioStreamer::CursoredBuffer();
		raw->_size = nextAudioFrame->dataSize();
		raw->_data = nextAudioFrame->data();
		raw->_ptr = raw->_data;
		//delete the audioDecodedframe
		nextAudioFrame->resetDataPtr();
		delete nextAudioFrame;

		return raw;
	}
	return NULL;
}
auto_ptr<VideoImage> AVPipeline::getDecodedVideoFrame(int64_t ts){
	assert(_videoDecoder.get());
	assert(_mediaParser.get());

	auto_ptr<VideoImage> videoImage;
	int64_t nextTimestamp;
	bool parseComplete = _mediaParser->parseComplete();
	nextTimestamp = _videoDecoder->nextVideoFrameTimestamp();

	if(nextTimestamp==-1){
		cout<<"the next Timestamp is -1"<<endl;
		if(parseComplete && _mediaParser->isBufferEmpty()){
			decodingState(DEC_STOP);
			setStatus(playStop);
		}
		return videoImage;
	}

	if(nextTimestamp > ts){
		return videoImage;
	}

	uint64_t prePts = 0;
	while(1){
		videoImage = _videoDecoder->getVideoImage();
		if(!videoImage.get()){
			printf("not get the video\n");
			break;
		}
		prePts = nextTimestamp;
		nextTimestamp = _videoDecoder->nextVideoFrameTimestamp();
		if(nextTimestamp==-1) {
			cout<<"nextTimeStamp is -1"<<endl;
			break;
		}
		if(nextTimestamp>ts) break;

		_dropedFrameCnt++;
	}

	return videoImage;
}

AVPipeline::DecodingState AVPipeline::decodingState(DecodingState state){
	if(state != DEC_NONE){
		_decodingState = state;
	}
	return _decodingState;

}

void AVPipeline::setStatus(StatusCode status){
	_statusCode = status;
}
AVPipeline::StatusCode AVPipeline::getStatus(){
	return _statusCode;
}
    
bool AVPipeline::parseComplete(){
    return _mediaParser->parseComplete();
}
    
double AVPipeline::videoTimeBase(){
        return _mediaParser->videoTimeBase();
}
    
auto_ptr<AVPacket> AVPipeline::nextVideoEncodedFrame(){
    return _mediaParser->nextVideoEncodedFrame();
}
    
void AVPipeline::notifyParserThread(){
        _mediaParser->notifyParserThread();
}

void AVPipeline::SetMediaInfo(MediaInfo& mediaInfo){
	_mediaInfo = mediaInfo;
std::cout<<"Get Media Info  through MediaParaerDelegate"<<std::endl;
}

void AVPipeline::SetParserState(MediaParserState state){
	_parserState = state;
	std::cout<<"Ge MediaParser State"<<std::endl;
}

 void SaveVideoPacket(AVPacket* packet){
 	_videoPacketQueue.push_back(packet);
 }

void SaveAudioPacket(AVPacket* packet){
	_audioPacketQueue.push_back(packet);
}

uint64_t GetPacketBufferedLength(){

}


/*-------------------------bufferedAudioStreamer---------------------*/
void BufferedAudioStreamer::attachAuxStreamer(){
	if(!_soundHandler)
		return;
	if(_auxStreamer){
		_soundHandler->unplugInputStream(_auxStreamer);
		_auxStreamer = 0;
	}
	_auxStreamer = _soundHandler->attachAuxStreamer(
			BufferedAudioStreamer::fetchWrapper, (void*)this);

}
void BufferedAudioStreamer::setSoundHandler(SoundHandler *handler){
	_soundHandler = handler;
}
void BufferedAudioStreamer::detachAuxStreamer(){
	if(!_soundHandler) return ;
	if(!_auxStreamer){
		cout<<__FILE__<<"->: the _auxstream not attached"<<endl;
		return ;
	}
	_soundHandler->unplugInputStream(_auxStreamer);
	_auxStreamer = 0;
}

uint16_t BufferedAudioStreamer::fetchWrapper(void * owner, int16_t*
		samples, uint16_t nSamples, bool& eof){
	BufferedAudioStreamer *streamer = static_cast<BufferedAudioStreamer*>(owner);
	return streamer->fetch(samples, nSamples, eof);
}

BufferedAudioStreamer::BufferedAudioStreamer():
		_audioQueue(),
		_audioQueueSize(0),
		_auxStreamer(0)
{}
uint16_t BufferedAudioStreamer::fetch(int16_t *samples, uint16_t nSamples, bool& eof){
	uint8_t *stream = reinterpret_cast<uint8_t*>(samples);
	int len = nSamples*2;
	boost::mutex::scoped_lock lock(_audioQueueMutex);

	while(len){
		if(_audioQueue.empty()){
			break;
		}
		CursoredBuffer& samples = *_audioQueue.front();
		assert(!(samples._size%2));
		int n = std::min<int>(samples._size,len);
		std::copy(samples._ptr,samples._ptr+n,stream);
		stream +=n;
		samples._ptr +=n;
		samples._size -=n;
		len -=n;
		if(samples._size == 0){
			_audioQueue.pop_front();
		}
		_audioQueueSize -=n;
	}//while(len)
	assert(!(len%2));

	eof = false;
	return nSamples-(len/2);
}

void BufferedAudioStreamer::push(CursoredBuffer *audio){
	boost::mutex::scoped_lock lock(_audioQueueMutex);
	if(_auxStreamer){
		_audioQueue.push_back(audio);
		_audioQueueSize += audio->_size;
	}else{
		delete audio;
	}
}

void BufferedAudioStreamer::cleanAudioQueue(){
	boost::mutex::scoped_lock lock(_audioQueueMutex);
	_audioQueue.clear();
}

/***************interface for action script*******************/
    
void AVPipeline::Seek(int32_t secPos){
    AsyncTask seekTask = boost::bind(&AVPipeline::SeekAction, this, secPos);
    PostTask(seekTask);
}

void  AVPipeline::SeekAction(int32_t secPos){
#if 1	
	_seekPos = secPos;
	if(_playOver)  return ;
	if(_seekFlag){
		return  ;
	}

	if(!_mediaParser.get()){
		return  ;
	}
	if(!_videoDecoder.get()){
		return ;
	}
	_seekFlag = true;
	//In the BT mode:seek is not same whith the custom mode
		int64_t millPos = secPos*1000;
		cout<<"seekPos = "<<secPos<<endl;
		if(!_mediaParser->seek(millPos)){
			setStatus(invalidTime);
			_playbackClock.resume();
			cout<<__FILE__<<__LINE__<<"seek failed"<<endl;
			_seekFlag = false;
			return ;
		}
	
	_videoDecoder->clearVideoFrameQueue();
#if ENABLE_SOUND
	_audioDecoder->clearAudioFrameQueue();
	_audioStreamer.cleanAudioQueue();
#endif
	_playHead.seekTo(_mediaParser->getRealSeekPos()+2);
	//decodingState(DEC_BUFFERING);//droped by lxn 20131118. after add the sound model.
	//videoQueue and audioQueue were cleared ,so they need time to get the new decoded frames.
	snailSleep(60000);//macoSecond
	refreshVideoFrame(true);
	_seekFlag = false;
	return ;
#else
	cout<<"now to seek the seekPos = "<<secPos<<endl;
	cout<<"current pos = "<<_playHead.getPosition()/1000;
	if(_playOver)  return false ;
	if(_seekFlag){
		return ;
	}

	if(!_mediaParser.get()){
		return ;
	}
	if(!_videoDecoder.get()){
		return ;
	}
	_seekFlag = true;
	//In the BT mode:seek is not same whith the custom mode
	if(_btFlag){
		_mediaParser->seek(0);
	}else{
		int64_t millPos = secPos*1000;
		if(!_mediaParser->seek(millPos)){
			setStatus(invalidTime);
			_playbackClock.resume();
			cout<<__FILE__<<__LINE__<<"seek failed"<<endl;
			_seekFlag = false;
			return ;
		}
	}
	_videoDecoder->clearVideoFrameQueue();
#if ENABLE_SOUND
	_audioDecoder->clearAudioFrameQueue();
	_audioStreamer.cleanAudioQueue();
#endif
	_playHead.seekTo(_mediaParser->getRealSeekPos()+2);
	//decodingState(DEC_BUFFERING);//droped by lxn 20131118. after add the sound model.
	//videoQueue and audioQueue were cleared ,so they need time to get the new decoded frames.
	snail::timer::snailSleep(60000);//macoSecond
	refreshVideoFrame(true);
	_seekFlag = false;
	return ;
#endif
}

void AVPipeline::pause(){
    AsyncTask pauseTask = boost::bind(&AVPipeline::PauseAction, this);
    PostTask(pauseTask);
}

void AVPipeline::PauseAction(){
    cout<<endl;
    cout<<"=============================="<<endl;
    cout<<"now  to pause"<<endl;
    cout<<"=============================="<<endl;
    cout<<endl;
    PlayControl::PlaybackStatus  curStatus = _playHead.getState();
    if(curStatus == PlayControl::PLAY_PLAYING){
        _playHead.setState(PlayControl::PLAY_PAUSED);
#if ENABLE_SOUND
        _audioStreamer.detachAuxStreamer();
#endif
    }else{
        _playHead.setState(PlayControl::PLAY_PLAYING);
#if ENABLE_SOUND
        _audioStreamer.attachAuxStreamer();
#endif
    }
}
    
void AVPipeline::Reset(){
	boost::mutex::scoped_lock lock(_mutex);
#if ENABLE_SOUND
	_audioStreamer.cleanAudioQueue();
	_audioStreamer.detachAuxStreamer();
	_audioDecoder.reset();
#endif
	_videoDecoder.reset(); //this must before the mediaParser.reset(), beacause use the pointer of the _mediaParser
	_mediaParser.reset();
	_imageFrame.reset();
	_playOver = true;
	_seekFlag = false;
	decodingState(DEC_STOP);
	setStatus(playStop);

}
void AVPipeline::resume(){
	if(_playHead.getState()==PlayControl::PLAY_PAUSED){
		pause();
	}
}

} // namespace
