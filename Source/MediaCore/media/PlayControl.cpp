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

#include "PlayControl.h"
#include <stdio.h>
#include <iostream>
#include <memory>
#include <cassert>
#include "ClockTime.h"
using namespace std;

namespace MediaCore {

PlayControl::PlayControl(VirtualClock *clock):
		_position(0),
		_state(PLAY_PLAYING),
		_availableConsumers(0),
		_positionConsumers(0),
		_clockSource(clock),
		_clockOffset(0){
	_clockOffset = 0;
}
PlayControl::PlaybackStatus PlayControl::setState(PlaybackStatus newState){
	if(_state != newState){
		if(_state == PLAY_PAUSED){
			assert(newState == PLAY_PLAYING);
			_state = PLAY_PLAYING;
			int64_t now = _clockSource->elapsed();
			_clockOffset = (now - _position);

			assert(now-_clockOffset == _position);
			return PLAY_PAUSED;
		}else{
			assert(_state==PLAY_PLAYING);
			assert(newState==PLAY_PAUSED);
			_state = PLAY_PAUSED;
		}
	}
	return _state;
}

PlayControl::PlaybackStatus PlayControl::toggleState(){
	if(_state==PLAY_PAUSED)
		return setState(PLAY_PAUSED);
	else
		return setState(PLAY_PLAYING);
}

void PlayControl::seekTo(int64_t position){
	int64_t now =  _clockSource->elapsed();
	_position = position;
	_clockOffset = (now - _position);
	//check if we did the right thing
	assert(now - _clockOffset == _position);

	//Reset consumers state
	_positionConsumers = 0;
}

void PlayControl::advanceIfConsumed(){
	if( (_positionConsumers & _availableConsumers) !=
			_availableConsumers){
		return ;
	}
	int64_t now = _clockSource->elapsed();
	_position = now - _clockOffset;
	_positionConsumers = 0;
}

} // namespace
