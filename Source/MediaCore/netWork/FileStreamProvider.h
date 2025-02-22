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

#ifndef fileStream_h
#define fileStream_h

#include "IOChannel.h"
#include <stdio.h>

namespace MediaCore {

class FileStreamProvider: public IOChannel{
public:
	FileStreamProvider(FILE* fp);
	virtual std::streamsize read(void* dst, std::streamsize num);
	virtual std::streampos tell() const;
	virtual bool seek(std::streampos p);
	virtual void go_to_end();
	virtual bool eof() const;
	virtual bool bad() const ;
	virtual size_t size() const;
private:
	FILE* _fp;
	int _pos;
	bool _error;
	long _size;
};

} // namespace
#endif
