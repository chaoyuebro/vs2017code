#include "muduo/base/Logging.h"
#include "muduo/net/Channel.h"
#include "muduo/net/EventLoop.h"

#include <sstream>

#include <poll.h>

using namespace muduo;
using namespace muduo::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd__)
	: loop_(loop),
	fd_(fd__),
	events_(0),
	revents_(0),
	index_(-1),
	logHup_(true),
	tied_(false),
	eventHandling_(false),
	addedToLoop_(false)
{
}

Channel::~Channel()
{
	assert(!eventHandling_);
	assert(!addedToLoop_);
	if (loop_->isInLoopThread())
	{
		assert(!loop_->hasChannel(this));
	}
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
	/*tie_绑定Channel拥有者的this指针，tie_是weak_ptr类型，不会延长obj的生命周期,
	当tie_提升shared_ptr失败表示Channel拥有者已经销毁了
	*/
	tie_ = obj;
	tied_ = true;
}

void Channel::update()
{
	//需要通过loop_到Poller中修改Channel的监听状态
	addedToLoop_ = true;
	loop_->updateChannel(this);
}

void Channel::remove()
{
	//需要通过loop_到Poller的映射表中删除Channel的指针
	assert(isNoneEvent());
	addedToLoop_ = false;
	loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
	std::shared_ptr<void> guard;
	if (tied_)
	{
		guard = tie_.lock();
		if (guard)
		{
			//tie_提升shared_ptr成功，说明该Channel拥有者仍然健在
			handleEventWithGuard(receiveTime);
		}
		//该Channel拥有者销毁了，什么也不做...
	}
	else
	{
		//该Channel没有拥有者
		handleEventWithGuard(receiveTime);
	}
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
	//解析从Poller::poll()返回的revents_，执行相应的回调
	eventHandling_ = true;
	LOG_TRACE << reventsToString();
	if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
	{
		if (logHup_)
		{
			LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
		}
		if (closeCallback_) closeCallback_();
	}

	if (revents_ & POLLNVAL)
	{
		LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
	}

	if (revents_ & (POLLERR | POLLNVAL))
	{
		if (errorCallback_) errorCallback_();
	}
	if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
	{
		if (readCallback_) readCallback_(receiveTime);
	}
	if (revents_ & POLLOUT)
	{
		if (writeCallback_) writeCallback_();
	}
	eventHandling_ = false;
}

string Channel::reventsToString() const
{
	return eventsToString(fd_, revents_);
}

string Channel::eventsToString() const
{
	return eventsToString(fd_, events_);
}

string Channel::eventsToString(int fd, int ev)
{
	std::ostringstream oss;
	oss << fd << ": ";
	if (ev & POLLIN)
		oss << "IN ";
	if (ev & POLLPRI)
		oss << "PRI ";
	if (ev & POLLOUT)
		oss << "OUT ";
	if (ev & POLLHUP)
		oss << "HUP ";
	if (ev & POLLRDHUP)
		oss << "RDHUP ";
	if (ev & POLLERR)
		oss << "ERR ";
	if (ev & POLLNVAL)
		oss << "NVAL ";

	return oss.str();
}