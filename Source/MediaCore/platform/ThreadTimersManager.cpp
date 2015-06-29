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

#include "CurrentTime.h"
#include "SharedTimer.h"
#include "ThreadTimersManager.h"
#include "ThreadGlobalData.h"
#include "Timer.h"
#include <iostream>
#include <stdio.h>
using namespace std;

namespace MediaCore {

const double maxSliceOfFireTimers = 0.05; //second
SharedTimer* mainThreadSharedTimer(){
	return new MainThreadSharedTimer();
}
bool ThreadTimersManager::LittleFunc::operator()(TimerBase *t1, TimerBase *t2) const {
	if(t1->nextFireTime() != t2->nextFireTime())
		return (t1->nextFireTime() >= t2->nextFireTime());
	return (t1->order() >= t2->order());
}

ThreadTimersManager::ThreadTimersManager(bool isMainThread):
	_pendingNextSharedTimerFireTime(0),
	_firingTimer(false),
	_sharedTimer(NULL){
		if(1/*isMainThread()*/){
			setSharedTimer(mainThreadSharedTimer());
		}
}

void ThreadTimersManager::setSharedTimer(SharedTimer *sharedTimer){
	if(_sharedTimer){
		_sharedTimer->setFiredFunction(0);
		_sharedTimer->setFiredInterval(0);
		_sharedTimer->stop();
		_sharedTimer = NULL;
	}

	_sharedTimer = sharedTimer;
	if(_sharedTimer){
		_sharedTimer->setFiredFunction(ThreadTimersManager::sharedTimerFired);
	updateSharedTimer();
	}
}
void ThreadTimersManager::updateSharedTimer(){
	if(!_sharedTimer) return ;

	if(_firingTimer || _timersHeap.empty()){
		_pendingNextSharedTimerFireTime = 0;
		_sharedTimer->stop();	
	}else{
		double nextFireTime = _timersHeap.top()->nextFireTime();
		//[mɒnə'tɒnɪklɪ]
		double currentMonotonicTime = monotonicallyIncreasingTime();
		if(_pendingNextSharedTimerFireTime){
			if(_pendingNextSharedTimerFireTime <= currentMonotonicTime && nextFireTime <= currentMonotonicTime)
				return ;
		}
		_pendingNextSharedTimerFireTime = nextFireTime;
		_sharedTimer->setFiredInterval(max(nextFireTime-currentMonotonicTime, 0.0));
	}
}
//static
void ThreadTimersManager::sharedTimerFired(){
		threadGlobalData()->threadTimersManager()->sharedTimerFiredInterval();
	}
void ThreadTimersManager::sharedTimerFiredInterval(){
	if(_firingTimer) return ;
	std::cout<<"now to fire the timers\n"<<std::endl;
	_firingTimer = true;
	_pendingNextSharedTimerFireTime = 0;
	
	double fireTime = monotonicallyIncreasingTime();
	printf("%s-->fireTime = %lf  \n", __FUNCTION__, fireTime);
	double timeToQuit = fireTime + maxSliceOfFireTimers;
	while(!_timersHeap.empty()&&_timersHeap.top()->nextFireTime()<=fireTime){
		TimerBase *timer = _timersHeap.top();
		_timersHeap.pop();
		timer->fire();
		double repeatInterval = timer->repeatInterval();
		timer->setFireTime(repeatInterval?fireTime+repeatInterval:0);
		printf("_timerHeas.size()=%d\n", _timersHeap.size());
		if(monotonicallyIncreasingTime()>=timeToQuit){
			printf("%s --> time to quit fire\n", __FUNCTION__);
			break;
		}
	}
	_firingTimer = false;	
	updateSharedTimer();
}

} // namepsace
