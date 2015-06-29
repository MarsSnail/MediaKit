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

#ifndef PLAYCONTROL_H_
#define PLAYCONTROL_H_
#include "SnailConfig.h"
#include "SystemClock.h"
#include <iostream>
#include <memory>

using namespace std;

namespace MediaCore {

class PlayControl{
public:
	enum PlaybackStatus{
		PLAY_PLAYING = 1,
		PLAY_PAUSED = 2
	};
	PlayControl(VirtualClock *clock);

	///Set a video consumer as available
	//This should be completely fine to do during the playControl lifetime
	void setVideoConsumerAvailable(){
		_availableConsumers |= CONSUMER_VIDEO;
	}

	///Set a audio consumer as available
	// this should be complete fine to do during the playcontrol lifetime
	void setAudioConsumerAvailable(){
		_availableConsumers |= CONSUMER_AUDIO;
	}

	int64_t getPosition() const { return _position;}

	//Set playback state, returning old state
	PlaybackStatus setState(PlaybackStatus newState);
	PlaybackStatus getState(){return _state;}
	//toggle playback state, returning old state
	PlaybackStatus toggleState();

	///return true if video of current position have been consumed
	bool isVideoConsumed() const{
		return (_positionConsumers&CONSUMER_VIDEO);
	}
	//marked current position as being consumed by video
	void setVideoConsumed(){
		_positionConsumers |= CONSUMER_VIDEO;
	}

	bool isAudioConsumed() const{
		return (_positionConsumers&CONSUMER_AUDIO);
	}
	void setAudioConsumed(){
		_positionConsumers |= CONSUMER_AUDIO;
	}

	//@@change current position to the given time

	//Consume flag will be reset
	//@param position
	///Position timestamp(milliseconds)
	void seekTo(int64_t position);

	///@@ Advance position if all available consumers consumed
	//the current one, Clock source will be used to determined the
	//amount of milliseconds to advanced position to
	//consumer flag will be reset
	void advanceIfConsumed();
private:
	enum ConsumerFlag{
		CONSUMER_VIDEO = 1,
		CONSUMER_AUDIO = 2
	};

	//current playing position
	int64_t _position;
	//current playback state
	PlaybackStatus _state;
	//Binary OR of consumers representing
	//which consumers are active
	int _availableConsumers;

	//Binary OR of consumers representing
	//which consumers consumed current positon
	int _positionConsumers;

	VirtualClock *_clockSource;
	int64_t _clockOffset;

};//PlayControl
} // namespace

#endif /* PLAYCONTROL_H_ */
