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

#ifndef MEDIAHANDLER_H_
#define MEDIAHANDLER_H_

#include "SnailConfig.h"

namespace MediaCore {

class MediaHandler{
public:
	virtual auto_ptr<MediaParser> createMediaParser(auto_ptr<IOChannel> inputStream) = 0;
	virtual auto_ptr<VideoDecoder> createVideoDecoder() = 0;
	virtual auto_ptr<AudioDecoder> createAudioDecoder() = 0;

};

} // namespace

#endif /* MEDIAHANDLER_H_ */
