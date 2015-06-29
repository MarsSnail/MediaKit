#ifndef COMMON_DEF_H
#define COMMON_DEF_H

class MediaInfo{
public:
	int _videoWidth;
	int  _videoHeight;
	int64_t  _videoDuration; //millseconds
	double _videoFramerate;
	double _videoAudioSamplerate;
	AVRational _audioTimeBase;
	AVRational _videoTimeBase;
};

enum MediaParserState{
	PARSER_STATE_PARSING,
	PARSER_STATE_BUFFERING,
	PARSER_STATE_COMPLETE,
};

#endif