#ifndef HEADER_THREADOBJECT
#define HEADER_THREADOBJECT

#include <thread>
#include <atomic>

class ThreadObject
{
	private:
	
	virtual void run() = 0;
	
	protected:
	
	std::thread executorThread;

	std::atomic<bool> running;
	
	public:
	
	ThreadObject();
	virtual ~ThreadObject();
	
	virtual void Start();
	
	virtual void Stop();
};

#endif
