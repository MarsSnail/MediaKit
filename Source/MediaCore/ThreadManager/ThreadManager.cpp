#include "ThreadManager.h"

ThreadManager::Init()
{
	boost::shared_ptr<MessageLoop> av_pipe_line(new MessageLoop(AV_PIPE_LINE));
	message_loops_.push_back(av_pipe_line);
}

void ThreadManager::PostTask(ThreadType type, AsyncTask task){
	
}

MessageLoop::MessageLoop(ThreadType type)
		: thread_type_(type)
{

}

MessageLoop::Init(){
	io_service_.reset(new boost::asio::io_service());
	io_service_work_.reset(new boost::asio::io_service::work(*_io_service.get()));
	task_thread_.reset(new boost::thread
		(boost::bind(&boost::asio::io_service::run, _io_service.get())));
}

void MessageLoop::PostTask(AsyncTask task){
	assert(_io_service.get());
	io_service_->post(task);
}