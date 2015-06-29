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

#include "../SharedTimer.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

namespace MediaCore {

	static void (*func)(void);

	void signalCB(int id){
		func();
	}

	void setSharedTimerFiredFunction(void (*f)()){
		printf("OK: you have set the fired function\n");
		func = f;
	}	

	void setSharedTimerFiredInterval(double interval){
		signal(SIGALRM, signalCB);
		int  usecond = (interval-static_cast<int>(interval))*1000000;
		struct itimerval val;
		val.it_value.tv_sec = static_cast<int>(interval);
		val.it_value.tv_usec = usecond;
		val.it_interval.tv_sec = 0;
		val.it_interval.tv_usec = 0;
		setitimer(ITIMER_REAL, &val, NULL);
	}
	void stopSharedTimer(){
		struct itimerval val;
		val.it_value.tv_sec = 0;
		val.it_value.tv_usec = 0;
		val.it_interval = val.it_value;
		setitimer(ITIMER_REAL, &val, NULL);
	}	
} // namespace
