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

#include "InputStreamProvider.h"
#include "NetworkAdapter.h"
#include <iostream>
#include <memory>
#include "IOChannel.h"
#include "FileStreamProvider.h"
using namespace std;

namespace MediaCore {

InputStreamProvider::InputStreamProvider(Url& url):
	_url(url){
	//here you don't need do nothing
}
auto_ptr<IOChannel> InputStreamProvider::getStream() const{

	auto_ptr<IOChannel> stream;
	if(_url.protocol() == "file"){
		string path = _url.path();
		if(0){

		}else{
			FILE *inFp = fopen(path.c_str(),"rb");
			if(!inFp){
				printf("<lxn>%s(%d)-->fopen the file of %s failed\n",__FILE__,__LINE__,path.c_str());
				printf("fopen->ERRInfo:%s\n",strerror(errno));
				return stream;
			}
			auto_ptr<IOChannel> fs(new FileStreamProvider(inFp));
			stream = fs;
		}
	}else if(_url.protocol() == "http"){
		stream = NetworkAdapter::makeStream(_url.str(),"");
	}
	return stream;
}
bool InputStreamProvider::allowAccess() const{
		return 1;
}

} // namespace
