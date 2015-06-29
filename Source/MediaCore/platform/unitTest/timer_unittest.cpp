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

#include "Timer.h"
#include "CurrentTime.h"
#include <iostream>
#include <unistd.h>
#include <stdio.h>
using namespace std;
using namespace timer;
class TimerTest{
public:
	TimerTest();
        void fireFunc(Timer<TimerTest> *timer);
	void addTimers(double t);
private:
	Timer<TimerTest> _timer;
}; 

TimerTest::TimerTest():
	_timer(this,&TimerTest::fireFunc){
//do nothing
}

void TimerTest::fireFunc(Timer<TimerTest> *timer){
	printf("begin to sleep\n");
	printf("ok: you fire me in %lf \n", monotonicallyIncreasingTime());
}

void TimerTest::addTimers(double t){
	
	_timer.start(t,1);
}


int main(int argc, char **argv){
	TimerTest t1, t2;
	t1.addTimers(1);
//	t2.addTimers(2);
	while(1){
	}
	printf("sleep 30 second is time to\n");
}
