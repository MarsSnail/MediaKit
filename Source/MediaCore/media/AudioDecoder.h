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

#ifndef AUDIODECODER_H_
#define AUDIODECODER_H_
#include <stdint.h>

namespace MediaCore {

    class AudioDecodedFrame;
    
class AudioDecoder{
public:
    virtual ~AudioDecoder(){}
	virtual AudioDecodedFrame* popAudioDecodedFrame()=0;
	virtual int64_t nextAudioFrameTimestamp() = 0;
	virtual void clearAudioFrameQueue() = 0;
    virtual bool IsBufferFull() = 0;
    virtual bool IsBufferLowLevel() = 0;
    virtual void decodeAudioFrame() =0;
};//class AudioDecoder

} // namespace

#endif /* AUDIODECODER_H_ */
