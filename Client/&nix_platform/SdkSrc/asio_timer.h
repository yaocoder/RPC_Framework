#ifndef asio_timer_h__
#define asio_timer_h__

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread.hpp>
#include <time.h>

typedef void (*Func)(void* param);

class TimerImplByAsio
{

public:
	TimerImplByAsio(Func f, void* param, long seconds) :param_(param),heartBeatSeconds_(seconds), func_(f), timer_(io_, boost::posix_time::seconds(10))
	{
		try
		{
			timer_.async_wait(boost::bind(&TimerImplByAsio::update, this));
		}
		catch(boost::system::system_error& ex )
		{
			boost::system::error_code ec = ex.code();
			std::cerr << ec.value() << std::endl;
			std::cerr << ec.category().name() << std::endl;
		}

	}

	void Run()
	{
		try
		{
			io_.run();
		}
		catch(boost::system::system_error& ex )
		{
			boost::system::error_code ec = ex.code();
			std::cerr << ec.value() << std::endl;
			std::cerr << ec.category().name() << std::endl;
		}
	}

	void Stop()
	{
		try
		{
			io_.stop();
		}
		catch( boost::system::system_error& ex )
		{
			boost::system::error_code ec = ex.code();
			std::cerr << ec.value() << std::endl;
			std::cerr << ec.category().name() << std::endl;
		}
	}

private:
	void* 	param_;
	long	heartBeatSeconds_;
	Func	func_;
	boost::asio::io_service io_;
	boost::asio::deadline_timer timer_;
	void update()
	{
		try
		{
			func_(param_);
			timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(heartBeatSeconds_));
			timer_.async_wait(boost::bind(&TimerImplByAsio::update, this));
		}
		catch(boost::system::system_error& ex )
		{
			boost::system::error_code ec = ex.code();
			std::cerr << ec.value() << std::endl;
			std::cerr << ec.category().name() << std::endl;
		}
	}
};

class CImplTimer
{
public:

	void HeartBeatImpl(Func func, void* param, long seconds);

	void StopHeartBeatImpl();

private:

	TimerImplByAsio *timer_;
	static void HeartBeatImplThread(void* arg);
};

#endif

void CImplTimer::HeartBeatImpl( Func func, void* param, long seconds)
{
	timer_ = new TimerImplByAsio(func, param, seconds);

	boost::thread thread_obj(boost::bind(&HeartBeatImplThread, (void*)this));
}

void CImplTimer::HeartBeatImplThread( void* arg )
{
	CImplTimer *pThis = static_cast<CImplTimer*>(arg);

	pThis->timer_->Run();

}

void CImplTimer::StopHeartBeatImpl()
{
	timer_->Stop();
	if (timer_ != NULL)
	{
		delete timer_;
	}
}

//void print(void* param)
//{
//	std::cout << param << "Hello, world!\n";
//}
//
//void print2()
//{
//	std::cout << "hahahhahah!\n";
//}


//int main()
//{
//	CImpl *impl = new CImpl;
//	int* param = new int;
//	impl->HeartBeatImpl(print, (void*)param, 3);
//
//	sleep(10);
//
//	impl->StopHeartBeatImpl();
//
//	delete impl;
//
//	cout << "****end****" << endl;
//	CImpl *impl1 = new CImpl;
//	impl1->HeartBeatImpl(print2, 3);
//
//	sleep(10);
//
//	impl1->StopHeartBeatImpl();
//
//	delete impl1;
//
//	return 0;
//}
