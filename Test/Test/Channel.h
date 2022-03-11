#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include "muduo/base/noncopyable.h"
#include "muduo/base/Timestamp.h"

#include <functional>
#include <memory>

namespace muduo
{
	namespace net
	{

		class EventLoop;

		///
		/// A selectable I/O channel.
		///
		/// This class doesn't own the file descriptor.
		/// The file descriptor could be a socket,
		/// an eventfd, a timerfd, or a signalfd
		class Channel : noncopyable
		{
		public:
			typedef std::function<void()> EventCallback;
			typedef std::function<void(Timestamp)> ReadEventCallback;

			Channel(EventLoop* loop, int fd);
			~Channel();

			//设置回调，一般是把该Channel拥有者的回调相应绑定到Channenl中
			void handleEvent(Timestamp receiveTime);
			void setReadCallback(ReadEventCallback cb)
			{
				readCallback_ = std::move(cb);
			}
			void setWriteCallback(EventCallback cb)
			{
				writeCallback_ = std::move(cb);
			}
			void setCloseCallback(EventCallback cb)
			{
				closeCallback_ = std::move(cb);
			}
			void setErrorCallback(EventCallback cb)
			{
				errorCallback_ = std::move(cb);
			}

			/// Tie this channel to the owner object managed by shared_ptr,
			/// prevent the owner object being destroyed in handleEvent.
			/*Channel通常作为其它类的成员，比如TcpConnection，而Channel的回调函数通常和TcpConnection
			通过std::bind绑定,当Poller通知该Channel的回调时，Channel会调用TcpConnection对应的回调，
			而此时TcpConnection的生命周期尚未可知，此时tie_保存TcpConnection的this指针，通过将tie_
			的weak_ptr提升为shared_ptr成功与否判断TcpConnection是否健在。*/
			void tie(const std::shared_ptr<void>&);

			int fd() const { return fd_; }
			int events() const { return events_; }
			void set_revents(int revt) { revents_ = revt; } // used by pollers
			// int revents() const { return revents_; }
			bool isNoneEvent() const { return events_ == kNoneEvent; }

			//update()会将修改后的events_写入到Poller中，修改当前Channel的监听状态
			void enableReading() { events_ |= kReadEvent; update(); }
			void disableReading() { events_ &= ~kReadEvent; update(); }
			void enableWriting() { events_ |= kWriteEvent; update(); }
			void disableWriting() { events_ &= ~kWriteEvent; update(); }
			void disableAll() { events_ = kNoneEvent; update(); }
			bool isWriting() const { return events_ & kWriteEvent; }
			bool isReading() const { return events_ & kReadEvent; }

			// for Poller
			int index() { return index_; }
			void set_index(int idx) { index_ = idx; }

			// for debug
			string reventsToString() const;
			string eventsToString() const;

			void doNotLogHup() { logHup_ = false; }

			EventLoop* ownerLoop() { return loop_; }
			//remove移除Poller映射表保存的Channel地址，避免悬垂指针
			void remove();

		private:
			static string eventsToString(int fd, int ev);
			//更新Channel在Poller中的监听状态(增、删、改)
			void update();
			//事件回调
			void handleEventWithGuard(Timestamp receiveTime);

			static const int kNoneEvent;
			static const int kReadEvent;
			static const int kWriteEvent;

			EventLoop* loop_;    //Channel所属的loop_
			const int  fd_;      //Channel和fd_打包注册到Poller的映射表
			int        events_;  //设置需要监听的事件类型(读、写、错误)
			int        revents_; // it's the received event types of epoll or poll
			int        index_;   // used by Poller.
			bool       logHup_;  //是否打印挂起日志

			std::weak_ptr<void> tie_; //绑定该Channel拥有者的this指针
			bool tied_;               //是否已把tie_和其他类this指针绑定
			bool eventHandling_;      //是否正在处理回调
			bool addedToLoop_;        //是否注册到Poller中监听
			ReadEventCallback readCallback_;
			EventCallback writeCallback_;
			EventCallback closeCallback_;
			EventCallback errorCallback_;
		};

	}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_CHANNEL_H