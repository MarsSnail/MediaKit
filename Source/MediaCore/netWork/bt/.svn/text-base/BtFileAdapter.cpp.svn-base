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

namespace snail{
namespace netWork{
BtFileAdapter::BtFileAdapter():
	_cacheFile(0),
	_cachefd(0),
	_error(false),
	_cached(0){
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
	return std::fread(dst, 1, bytes, _cacheFile);
}

void BtFileAdapter::fillCache(std::streamsize size){
	int waitTimes = 0;
	while(1){
		if(_cached>=size || waitTimes>4)  break;
		snail::timer::snailSleep(1000000);
		waitTimes++;
	}
	if(waitTimes>4)
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
	if(!_cacheFile)		return false;
	if(pos<0){
		std::ostringstream os;
		os<<"BtFileAdapter: can't seek  to negative absolutely position"<<pos;
		std::cout<<os<<std::endl;
	}
	fillCache(pos);
	if(_cached<pos || _error)	return false;
	if(std::fseek(_cacheFile, pos, SEEK_SET)==-1){
		std::cout<<"warning: fseek failed"<<std::endl;
		return false;
	}
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

std::streamsize BtFileAdapter::write(const void* src, std::streamsize size){

	int64_t currPos = std::ftell(_cacheFile);

	//seek to the end of  the file
	std::fseek(_cacheFile, 0, SEEK_END);
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
}//namespace netWork
}//namespace snail
