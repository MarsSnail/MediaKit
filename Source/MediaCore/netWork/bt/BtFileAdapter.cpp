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

#include "BtFileAdapter.h"

#include <iostream>
#include <string>
#include <cstdio>
#include <iosfwd>
#include<sstream>
#include "SnailException.h"
#include "SnailSleep.h"
#include "NetworkAdapter.h"
#include <stdint.h>

namespace MediaCore {

BtFileAdapter::BtFileAdapter():
	_cacheFile(0),
	_cachefd(0),
	_error(false),
	_cached(0),
	_seekFlag(false),
	_seekFlag2(false){
	init();
}
BtFileAdapter::~BtFileAdapter(){
	std::fclose(_cacheFile);
}
 void BtFileAdapter::init(){
	_cacheFile = std::tmpfile();
	if(!_cacheFile){
		throw SnailException("Could not create  tempprary cache file,int the construct function of BtFileAdapter");
	}
	_cachefd = fileno(_cacheFile);
}//init

std::streamsize BtFileAdapter::read(void *dst, std::streamsize bytes){
	if(!_cacheFile || eof() || _error)	return 0;
	fillCache(bytes+tell());
	if(_error)	return 0;
	boost::mutex::scoped_lock lock(_mutex);
	return std::fread(dst, 1, bytes, _cacheFile);
}

void BtFileAdapter::fillCache(std::streamsize size){
	int waitTimes = 0;
	while(1){
		if(_cached>=size || waitTimes>40000)  break;
		snailSleep(100);
		waitTimes++;
	}
	if(waitTimes>40000)
		_error = 1;
}
std::streampos BtFileAdapter::tell() const{
		std::streampos pos=0;
	if(_cacheFile)
			pos = std::ftell(_cacheFile);
	return pos;
}
bool BtFileAdapter::eof() const{
	bool ret = false;
	if(_cacheFile)
		ret = feof(_cacheFile);
	return ret;
}
bool BtFileAdapter::bad() const{
	return _error;
}

bool BtFileAdapter::seek(std::streampos pos){
	std::cout<<"LineNu(68):seek position is "<<pos<<"; cached="<<_cached<<std::endl;
	if(!_cacheFile)		return false;
	if(pos<0){
		std::ostringstream os;
		os<<"BtFileAdapter: can't seek  to negative absolutely position"<<pos;
		std::cout<<os<<std::endl;
	}
	if(_seekFlag2){
		pos = 0;
		_seekFlag2 = false;
	}
	fillCache(pos);
	if(_cached<pos || _error)	return false;
	if(std::fseek(_cacheFile, pos, SEEK_SET)==-1){
		std::cout<<"warning: fseek failed"<<std::endl;
		return false;
	}
	std::cout<<"seek success, cached = "<<_cached<<std::endl;
	return true;
}
void BtFileAdapter::go_to_end(){
	if(_cacheFile)
		std::fseek(_cacheFile, 0 , SEEK_END);
}

size_t BtFileAdapter::size() const {
	std::cout<<__FILE__<<": "<<__LINE__<<"this function is not complemention"<<std::endl;
	return 0;
}

void BtFileAdapter::reset(){
	std::fseek(_cacheFile, 0, SEEK_SET);
	_cached = 0;
	_seekFlag = true;
	_seekFlag2 = true;
	std::cout<<"now call Reset "<<std::endl;
}
std::streamsize BtFileAdapter::write(const void* src, std::streamsize size){
//sync for read and write
	boost::mutex::scoped_lock lock(_mutex);
	int64_t currPos = std::ftell(_cacheFile);

	//seek to the end of  the file
	std::fseek(_cacheFile, 0, SEEK_END);
	//when mediaPlayer call seek operation.
	if(_seekFlag){
		_seekFlag = false;
		std::fseek(_cacheFile, 0, SEEK_SET);
	}
	int actualWrite = 0;
	if(src!=NULL&& !_error){
		actualWrite = fwrite(src, size, 1, _cacheFile);
		if(actualWrite<1)
			throw SnailException(std::string("writting to cache file failed"));
		_cached = std::ftell(_cacheFile);

		//reset the position for the next read operation
		std::fseek(_cacheFile, currPos, SEEK_SET);
	}
	return actualWrite;
}

} // namespace
