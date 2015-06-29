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

#ifndef IOCHANNEL_H_
#define IOCHANNEL_H_

#include <string>
#include <stdio.h>
#include <iostream>

#include "Url.h"

namespace MediaCore {

class IOChannel{
public:
	virtual ~IOChannel(){}

	static IOChannel* CreateIOChannel(Url url);

	unsigned int read_le32();
	unsigned char read_le16();

	unsigned char read_byte();
	virtual std::streamsize read(void* dst, std::streamsize num) =0;
	virtual std::streamsize readNonBlocking(void* dst, std::streamsize num){
		return read(dst,num);
	}

	virtual std::streamsize write(const void* src, std::streamsize num);

	int read_string(char* dst, int max_length);

	float read_float32();
	//return current stream position
	virtual std::streampos tell() const  = 0;
	virtual bool seek(std::streampos p) = 0;
	virtual void go_to_end() = 0;
	virtual bool eof() const =0;
	virtual bool bad() const = 0;
	virtual size_t size() const {return static_cast<size_t>(-1);}
	virtual void reset(){}
};


} // namespace
#endif /* IOCHANNEL_H_ */

