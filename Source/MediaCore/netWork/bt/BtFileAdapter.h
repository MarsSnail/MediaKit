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

#ifndef BtFileAdapter_h
#define BtFileAdapter_h

#include "IOChannel.h"
#include <boost/thread/thread.hpp>
//#include <boost/thread/condition.hpp>
//#include <boost/thread/barrier.hpp>

namespace MediaCore {

class BtFileAdapter: public IOChannel{
public:
	BtFileAdapter();
	~BtFileAdapter();

	//implemention the abstract virtual functions of IOChannel
	virtual std::streamsize read(void *dst, std::streamsize bytes);
	virtual std::streamsize write(const void*,std::streamsize);
	virtual bool seek(std::streampos pos);
	void fillCache(std::streamsize size);
	virtual bool eof() const;
	virtual bool bad() const;
	virtual std::streampos tell() const ; //return the global position within the file
	virtual void go_to_end();
	virtual size_t size() const;
	virtual void reset();

private:
	void init();
private:
	FILE *_cacheFile;
	int _cachefd;
	bool _error;
	std::streamsize _cached;
	boost::mutex _mutex;
	bool _seekFlag;
	bool _seekFlag2;

};//class BtFileAdapter

} // namespace

#endif
