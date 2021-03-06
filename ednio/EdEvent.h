/**
 * @author netmind
 * @file EdEvent.h
 * @detail This is a base class for other event driven object(that is, EdSocket, EdTimer, EdPipe,...)
 */


#ifndef EDEVENT_H_
#define EDEVENT_H_

#include "ednio_config.h"

#include <fcntl.h>

#if USE_LIBEVENT
#include <event2/event.h>
#endif

#include "EdContext.h"
#include "EdObject.h"


namespace edft
{

/**
 * @enum EVT_READ read event
 */
enum
{
	EVT_READ = EPOLLIN,
	EVT_WRITE = EPOLLOUT,
	EVT_ONESHOT = EPOLLONESHOT,
	EVT_HANGUP = EPOLLRDHUP,
};

class EdTask;

/**
 * @class EdEvent
 * @brief Base Event Object class for monitoring events.
 */
class EdEvent : public EdObject
{
public:
	EdEvent();
	EdEvent(EdContext* ctx);
	virtual ~EdEvent();

protected:
	EdContext *mContext;
	int mFd;
	union {
		void *mUser;
		uint64_t mUserLong;
		uint32_t mUserInt;
	};
	bool mIsReg;
	EdTask* mTask;

private:
	edevt_t* mEvt;

#if USE_LIBEVENT
	struct event *mEvent;
#endif

private:
	void initMembers();

private:
	void setNonBlockMode(void);
	static void esevent_cb(edevt_t* pevt, int fd, int events);
#if USE_LIBEVENT
	static void libevent_cb(evutil_socket_t fd, short flags, void *arg);
#endif


public:
	/**
	 * @brief Called when read event is activated on epoll_wait.
	 * @detail When fd is monitored with EVT_READ and data is ready, this function is called. \n
	 * When this function is called, you should call recv().
	 */
	virtual void OnEventRead(void);

	/**
	 * @brief Called when write event is activated on epoll_wait.
	 * @detail When fd is monitored with EVT_WRITE and data is ready, this function is called.
	 */
	virtual void OnEventWrite(void);

	virtual void OnEventHangup(void);

	/**
	 * @brief Register fd with interested events on event monitor(epoll_wait)
	 * @param flag monitored event. some events can be bitwised OR(ex, EVT_READ|EVT_WRITE)
	 */
	void registerEvent(uint32_t flag = EVT_READ);

	/**
	 * @brief Deregister fd on event monitor.
	 */
	void deregisterEvent(void);

	/**
	 * @brief Change monitored events. fd must be registered in advance before using this function.
	 */
	void changeEvent(uint32_t flags);

	/**
	 * @brief Get user specific data. This data is set by setUser().
	 */
	void *getUser();
	/**
	 * @brief Set user specific data.
	 */
	void setUser(void *user);

	uint64_t getUserLong();
	void setUserLong(uint64_t data);
	uint32_t getUserInt();
	void setUserInt(uint32_t data);

	EdContext *getContext();
	EdTask* getTask() const { return mTask; };
	/**
	 * @brief set a file descriptor(fd) to monitor.
	 * @detail In here, fd can be a socket, pipe, eventfd, timerfd.
	 * @param fd File descriptor to monitor.
	 */
	void setFd(int fd);

	/**
	 * @brief Get fd to monitor.
	 * @return fd to be monitor currently.
	 */
	int getFd() const;
	void setContext(EdContext* ctx);
	void setDefaultContext();
};

} /* namespace edft */

#endif /* EDEVENT_H_ */
