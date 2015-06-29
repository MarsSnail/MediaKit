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

#ifndef ThreadTimersManager_h
#define ThreadTimersManager_h

#include <iostream>
#include <queue>
#include <vector>

using namespace std;

namespace MediaCore {

class TimerBase;
class SharedTimer;

class ThreadTimersManager{
public:
	//function object for priority_queue sort action
	class LittleFunc{
		public:
		bool operator()(TimerBase *t1,TimerBase *t2) const ;
	};
	
	typedef priority_queue<TimerBase*, vector<TimerBase*>, LittleFunc> PriorityHeap;

	ThreadTimersManager(bool isMainThread=false);
	void setSharedTimer(SharedTimer *sharedTimer);
	void updateSharedTimer();
	PriorityHeap& timersHeap(){return _timersHeap;}
	static void sharedTimerFired();
	void sharedTimerFiredInterval();	
private:
	double _pendingNextSharedTimerFireTime;
	bool _firingTimer;	
	PriorityHeap  _timersHeap;
	SharedTimer *_sharedTimer;	
};
} // namespace
#endif // ThreadTimersManager_h
