#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include "CommonDef.h"

class ThreadManager{
public:
	enum ThreadType{
		AV_PIPE_LINE,
	};
	bool Init();
	static void PostTask(ThreadType thread_type, AsyncTask task);
	static void PostDelayTask(ThreadType thread_type, AsyncTask task, int32_t delay_internal);
private:
	std::vector< boost::shared_ptr<MessageLoop> >  message_loops_;
};

class MessageLoop : public enable_shared_from_this<MessageLoopBase>{
public:
	MessageLoop(ThreadType type);
	bool Init();
	void PostTask(AsyncTask task);
	void PostDelayTask(AsyncTask task, int32_t delay_internal){}
private:
	boost::scoped_ptr<boost::thread>					task_thread_;
	boost::scoped_ptr<boost::asio::io_service::work>  	io_service_work_; 
	boost::scoped_ptr<boost::asio::io_service>   		io_service_;
	ThreadType 											thread_type_;
};

#endif