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
#include "CommonDef.h"
#include "AVPipeline.h"
#include "Url.h"
#include "InputStreamProvider.h"
#include "MediaParserFFmpeg.h"
#include "VideoDecoderFFmpeg.h"
#include "AudioDecoderFFmpeg.h"
#include "SoundHandlerSDL.h"
#include "ClockTime.h"
#include "SnailException.h"
#include "SnailSleep.h"
#include "AudioDecodedFrame.h"
#include "AVPipelineDelegate.h"

namespace MediaCore {

boost::scoped_ptr<SoundHandler> AVPipeline::_soundHandler;
AVPipeline::AVPipeline()
    : _mediaParser(NULL),
      _sysClock(),
      _playbackClock(_sysClock),
      _playHead(&_playbackClock),
      _bufferTime(100),
      _dropedFrameCnt(0),
      _seekFlag(false),
      _id(0) {
  _id = 0;
  cout << endl;
  cout << "Instance ID=" << _id << endl;
  cout << endl;
  if (!_soundHandler.get()) {
    _soundHandler.reset(new SoundHandlerSDL());
  }
  _audioStreamer.setSoundHandler(_soundHandler.get());
}

AVPipeline::~AVPipeline() { Reset(); }

void AVPipeline::Init() {
  _io_service.reset(new boost::asio::io_service());
  _io_service_work.reset(new boost::asio::io_service::work(*_io_service.get()));
  _task_thread.reset(new boost::thread(
      boost::bind(&boost::asio::io_service::run, _io_service.get())));
}

void AVPipeline::PostTask(AsyncTask task) {
  assert(_io_service.get());
  _io_service->post(task);
}

void AVPipeline::AsyncTaskTest() {
  AsyncTask task = boost::bind(&AVPipeline::TestFunc, this);
  PostTask(task);
}

void AVPipeline::TestFunc() {
  static int taskcount = 0;
  std::cout << ">>>>>>>>>>>TestFunc:" << taskcount++ << std::endl;
}

void AVPipeline::SetDelegate(boost::shared_ptr<AVPipelineDelegate> delegate) {
  _avPipelineDelegate = delegate;
}

bool AVPipeline::CreateMediaDecoder() {
  _mediaDecoder.reset(MediaDecoder::CreateMediaDecoder(this));
  _mediaDecoder->Init();
  return true;
}

bool AVPipeline::startPlayback(string url) {
  AsyncTask playbackTask = boost::bind(&AVPipeline::PlaybackAction, this, url);
  PostTask(playbackTask);
  return true;
}

void AVPipeline::PlaybackAction(string url) {
  Reset();
  _url = url;
  Url newUrl(url);

  if (!CreateMediaParser(url, this)) {
    cout << "Create MediaParser failed" << endl;
    return;
  }
    CreateMediaDecoder();
    _mediaParser->SetBufferTime(_bufferTime);
  _playbackClock.pause();
  SetPlayState(PLAY_STATE_PAUSE);
  _playHead.seekTo(0);
  _playHead.setState(PlayControl::PLAY_PLAYING);
  _audioStreamer.attachAuxStreamer();
  return;
}

void AVPipeline::beginDisplay() {
  // sync with the function of close().
  boost::mutex::scoped_lock lock(_mutex);

  if (_seekFlag) return;
  if (!_mediaParser.get() || !_mediaDecoder.get()) return;
  size_t videoFrameQueueSize = _mediaDecoder->videoFrameQueueLength();

  // switch from DEC_DECODING to DEC_DECPAUSE
  if (GetPlayState() == PLAY_STATE_PLAYING && videoFrameQueueSize == 0) {
    SetPlayState(PLAY_STATE_BUFFERING);
    _playbackClock.pause();
    cout << endl;
    cout << "now  switch from DEC_DECODING to DEC_DECPAUSE and pause "
            "PlaybackClock" << endl;
    cout << endl;
    return;
  }
  // when state is DEC_DECODEPAUSE,process the play actions
  if (GetPlayState() == PLAY_STATE_BUFFERING) {
    if (videoFrameQueueSize == 0 && IsParseComplete()) {
      cout << endl;
      cout << "stay the state in DEC_DECODEPAUSE and bufferLen >=0" << endl;
      cout << endl;
      SetPlayState(PLAY_STATE_OVER);
      return;
    } else {
      SetPlayState(PLAY_STATE_PLAYING);
      _playbackClock.resume();
      cout << endl;
      cout << "now  switch from  DEC_DECPAUSE to DEC_DECODING and resume "
              "PlaybackClock" << endl;
      cout << endl;
    }
  }

  uint64_t curPosition = _playHead.getPosition();
  if (0 == curPosition) {
    _playHead.seekTo(GetStartPlayTimestamp());
  }
  refreshVideoFrame();
  refreshAudioFrame();
  // Advance PlayControl position if current one decoded frame was
  // consumed by all available consumers
  _playHead.advanceIfConsumed();
}

uint64_t AVPipeline::GetStartPlayTimestamp() {
  uint64_t start_position;

  uint64_t videoBeginPosition = _mediaDecoder->nextVideoFrameTimestamp();
  uint64_t audioBeginPosition = _mediaDecoder->nextAudioFrameTimestamp();
  start_position = std::min(videoBeginPosition, audioBeginPosition);
  return start_position;
}
    
void AVPipeline::refreshVideoFrame(bool paused) {
  assert(_mediaParser.get());

  if (!paused && _playHead.getState() == PlayControl::PLAY_PAUSED) return;
  if (_playHead.isVideoConsumed()) return;

  uint64_t curPos = _playHead.getPosition();
  auto_ptr<VideoImage> videoImage = getDecodedVideoFrame(curPos);

  if (videoImage.get() && _avPipelineDelegate.get()) {
    _avPipelineDelegate->UpdateVideoFrame(videoImage);
  }
  _playHead.setVideoConsumed();
}

void AVPipeline::refreshAudioFrame(bool paused) {
  assert(_mediaDecoder.get());
  if (_playHead.getState() == PlayControl::PLAY_PAUSED) {
    return;
  }
  if (_playHead.isAudioConsumed()) {
    return;
  }
  uint64_t curPos = _playHead.getPosition();
  pushDecodedAudioFrames(curPos);
}

void AVPipeline::pushDecodedAudioFrames(int64_t ts) {
  bool consumed = false;
  int64_t nextTimestamp = -1;
  while (1) {
    boost::mutex::scoped_lock lock(_audioStreamer._audioQueueMutex);
    float tmpFps = 25;
    double msecsPreAdvance = 10000 / tmpFps;
    const uint16_t bufferLimit = 20;
    uint16_t bufferSize = _audioStreamer._audioQueue.size();
    if (bufferSize > bufferLimit) {
      _playbackClock.pause();
      return;
    }
    lock.unlock();
    bool parsingComplete = IsParseComplete();
    nextTimestamp = _mediaDecoder->nextAudioFrameTimestamp();
    if (nextTimestamp == -1) {
      if (parsingComplete) {
        consumed = true;
        if (false /*_mediaParser->isBufferEmpty()*/) {
          SetPlayState(PLAY_STATE_OVER);
        }
        break;
      }  // if(parsingComplete)
      break;
    }  // if(!nextTimestamp)
    if (nextTimestamp > ts) {
      consumed = true;
      if (nextTimestamp > ts + (int64_t)msecsPreAdvance) {
        break;
      }
    }
    BufferedAudioStreamer::CursoredBuffer* audio = getNextAudioDecodedFrame();
    if (!audio) {
      break;
    }
    if (!audio->_size) {
      delete audio;
      continue;
    }
    _audioStreamer.push(audio);
  }  // while(1)

  if (consumed) {
    _playbackClock.resume();
    _playHead.setAudioConsumed();
  }
  //_playHead.setAudioConsumed();
}
BufferedAudioStreamer::CursoredBuffer* AVPipeline::getNextAudioDecodedFrame() {
  assert(_mediaDecoder.get());
  AudioDecodedFrame* nextAudioFrame = NULL;
  nextAudioFrame = _mediaDecoder->popAudioDecodedFrame();
  if (nextAudioFrame) {
    BufferedAudioStreamer::CursoredBuffer* raw =
        new BufferedAudioStreamer::CursoredBuffer();
    raw->_size = nextAudioFrame->dataSize();
    raw->_data = nextAudioFrame->data();
    raw->_ptr = raw->_data;
    // delete the audioDecodedframe
    nextAudioFrame->resetDataPtr();
    delete nextAudioFrame;

    return raw;
  }
  return NULL;
}
auto_ptr<VideoImage> AVPipeline::getDecodedVideoFrame(int64_t ts) {
  assert(_mediaDecoder.get());
  assert(_mediaParser.get());

  auto_ptr<VideoImage> videoImage;
  int64_t nextTimestamp;
  nextTimestamp = _mediaDecoder->nextVideoFrameTimestamp();
  if (nextTimestamp == -1) {
    cout << "the next Timestamp is -1" << endl;
    if (IsParseComplete()) {
        SetPlayState(PLAY_STATE_OVER);
    }
    return videoImage;
  }

  if (nextTimestamp > ts) {
    return videoImage;
  }

  uint64_t prePts = 0;
  while (1) {
    videoImage = _mediaDecoder->getVideoImage();
    std::cout << "drop frames" << _dropedFrameCnt << std::endl;
    if (!videoImage.get()) {
      printf("not get the video\n");
      break;
    }
    prePts = nextTimestamp;
    nextTimestamp = _mediaDecoder->nextVideoFrameTimestamp();
    if (nextTimestamp == -1) {
      cout << "nextTimeStamp is -1" << endl;
      break;
    }
    if (nextTimestamp > ts) break;

    _dropedFrameCnt++;
  }

  return videoImage;
}

void AVPipeline::SetPlayState(PlayState state) { play_state_ = state; }

PlayState AVPipeline::GetPlayState() { return play_state_; }

auto_ptr<AVPacket> AVPipeline::nextVideoEncodedFrame() {
  return _mediaParser->GetNextEncodedVideoFrame();
}

// Utilies for control MediaParser
/***********Utilies for Mediaparser**************************/
bool AVPipeline::CreateMediaParser(std::string url,
                                   MediaParserDelegate* delegate) {
  _mediaParser.reset(new MediaParserFFmpeg(_url, this));

  if (!_mediaParser.get() || !_mediaParser->Init()) {
    return false;
  }
  return true;
}

bool AVPipeline::IsParseComplete() {
  return _parserState == PARSER_STATE_COMPLETE;
}

/****************end*******************/

// MediaParser Delegate Implementation
void AVPipeline::SetMediaInfo(const MediaInfo& mediaInfo) {
  _mediaInfo = mediaInfo;
  std::cout << "Get Media Info  through MediaParaerDelegate" << std::endl;
}

void AVPipeline::SetMediaParserState(const MediaParserState& state) {
  _parserState = state;
  std::cout << "Ge MediaParser State" << std::endl;
}

auto_ptr<AVPacket> AVPipeline::GetNextEncodedVideoFrame() {
  return _mediaParser->GetNextEncodedVideoFrame();
}

MediaParserState AVPipeline::GetMediaParserState() { return _parserState; }

double AVPipeline::GetVideoTimeBase() {
  return av_q2d(_mediaInfo._videoTimeBase);
}

double AVPipeline::GetAudioTimeBase() {
  return av_q2d(_mediaInfo._audioTimeBase);
}

auto_ptr<AVPacket> AVPipeline::GetNextEncodedAudioFrame() {
  return _mediaParser->GetNextEncodedAudioFrame();
}
/*-------------------------bufferedAudioStreamer---------------------*/
void BufferedAudioStreamer::attachAuxStreamer() {
  if (!_soundHandler) return;
  if (_auxStreamer) {
    _soundHandler->unplugInputStream(_auxStreamer);
    _auxStreamer = 0;
  }
  _auxStreamer = _soundHandler->attachAuxStreamer(
      BufferedAudioStreamer::fetchWrapper, (void*)this);
}
void BufferedAudioStreamer::setSoundHandler(SoundHandler* handler) {
  _soundHandler = handler;
}
void BufferedAudioStreamer::detachAuxStreamer() {
  if (!_soundHandler) return;
  if (!_auxStreamer) {
    cout << __FILE__ << "->: the _auxstream not attached" << endl;
    return;
  }
  _soundHandler->unplugInputStream(_auxStreamer);
  _auxStreamer = 0;
}

uint16_t BufferedAudioStreamer::fetchWrapper(void* owner, int16_t* samples,
                                             uint16_t nSamples, bool& eof) {
  BufferedAudioStreamer* streamer = static_cast<BufferedAudioStreamer*>(owner);
  return streamer->fetch(samples, nSamples, eof);
}

BufferedAudioStreamer::BufferedAudioStreamer()
    : _audioQueue(), _audioQueueSize(0), _auxStreamer(0) {}
uint16_t BufferedAudioStreamer::fetch(int16_t* samples, uint16_t nSamples,
                                      bool& eof) {
  uint8_t* stream = reinterpret_cast<uint8_t*>(samples);
  int len = nSamples * 2;
  boost::mutex::scoped_lock lock(_audioQueueMutex);

  while (len) {
    if (_audioQueue.empty()) {
      break;
    }
    CursoredBuffer& samples = *_audioQueue.front();
    assert(!(samples._size % 2));
    int n = std::min<int>(samples._size, len);
    std::copy(samples._ptr, samples._ptr + n, stream);
    stream += n;
    samples._ptr += n;
    samples._size -= n;
    len -= n;
    if (samples._size == 0) {
      _audioQueue.pop_front();
    }
    _audioQueueSize -= n;
  }  // while(len)
  assert(!(len % 2));

  eof = false;
  return nSamples - (len / 2);
}

void BufferedAudioStreamer::push(CursoredBuffer* audio) {
  boost::mutex::scoped_lock lock(_audioQueueMutex);
  if (_auxStreamer) {
    _audioQueue.push_back(audio);
    _audioQueueSize += audio->_size;
  } else {
    delete audio;
  }
}

void BufferedAudioStreamer::cleanAudioQueue() {
  boost::mutex::scoped_lock lock(_audioQueueMutex);
  _audioQueue.clear();
}

/***************interface for action script*******************/

void AVPipeline::Seek(int32_t secPos) {
  AsyncTask seekTask = boost::bind(&AVPipeline::SeekAction, this, secPos);
  PostTask(seekTask);
}

void AVPipeline::SeekAction(int32_t secPos) {
  _seekPos = secPos;
  if (_seekFlag) {
    return;
  }

  if (!_mediaParser.get()) {
    return;
  }
  if (!_mediaDecoder.get()) {
    return;
  }
  _seekFlag = true;
  // In the BT mode:seek is not same whith the custom mode
  int64_t millPos = secPos * 1000;
  cout << "seekPos = " << secPos << endl;
  if (!_mediaParser->Seek(millPos)) {
    _playbackClock.resume();
    cout << __FILE__ << __LINE__ << "seek failed" << endl;
    _seekFlag = false;
    return;
  }

  _mediaDecoder->ClearDeocdedFrameBuffer();
  _audioStreamer.cleanAudioQueue();
  _playHead.seekTo(_mediaParser->GetRealSeekPos() + 2);
  // decodingState(DEC_BUFFERING);//droped by lxn 20131118. after add the sound
  // model.
  // videoQueue and audioQueue were cleared ,so they need time to get the new
  // decoded frames.
  snailSleep(60000);  // macoSecond
  refreshVideoFrame(true);
  _seekFlag = false;
  return;
}

void AVPipeline::pause() {
  AsyncTask pauseTask = boost::bind(&AVPipeline::PauseAction, this);
  PostTask(pauseTask);
}

void AVPipeline::PauseAction() {
  cout << endl;
  cout << "==============================" << endl;
  cout << "now  to pause" << endl;
  cout << "==============================" << endl;
  cout << endl;
  PlayControl::PlaybackStatus curStatus = _playHead.getState();
  if (curStatus == PlayControl::PLAY_PLAYING) {
    _playHead.setState(PlayControl::PLAY_PAUSED);
    _audioStreamer.detachAuxStreamer();
  } else {
    _playHead.setState(PlayControl::PLAY_PLAYING);
    _audioStreamer.attachAuxStreamer();
  }
}

void AVPipeline::Reset() {
  boost::mutex::scoped_lock lock(_mutex);
  _audioStreamer.cleanAudioQueue();
  _audioStreamer.detachAuxStreamer();
  _mediaDecoder.reset();  // this must before the mediaParser.reset(), beacause
                          // use the pointer of the _mediaParser
  _mediaParser.reset();
  _seekFlag = false;
}
void AVPipeline::resume() {
  if (_playHead.getState() == PlayControl::PLAY_PAUSED) {
    pause();
  }
}

}  // namespace
