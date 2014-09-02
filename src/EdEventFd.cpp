/*
 * EdEventFd.cpp
 *
 *  Created on: Aug 28, 2014
 *      Author: netmind
 */
#define DBGTAG "evtfd"
#define DBG_LEVEL DBG_WARN

#include <sys/eventfd.h>
#include "EdEventFd.h"
#include "edslog.h"

namespace edft
{

EdEventFd::EdEventFd()
{
	mOnListener = NULL;

}

EdEventFd::~EdEventFd()
{
	close();
}

void EdEventFd::OnEventRead()
{
	uint64_t cnt;
	int rcnt = read(getFd(), &cnt, 8);
	if(rcnt == 8)
		OnEventFd((int)cnt);
	else
		dbge("### eventfd read error...");
}

int EdEventFd::open()
{
	int fd = eventfd(0, EFD_NONBLOCK);
	if (fd >= 0)
	{
		setFd(fd);
		registerEvent(EVT_READ);
		return 0;
	}
	else
	{
		dbge("### Fail: eventfd create error !!!");
		return -1;
	}
}

void EdEventFd::close()
{
	if (getFd() >= 0)
	{
		::close(mFd);
		mFd = -1;
	}
}

int EdEventFd::raise()
{
	uint64_t cnt = 1;
	ssize_t wcnt = write(getFd(), &cnt, 8);
	if(wcnt == 8) {
		return 0;
	} else {
		return -1;
	}

}

void EdEventFd::OnEventFd(int cnt)
{
	if (mOnListener != NULL)
	{
		mOnListener->IOnEventFd(this, (int) cnt);
	}
}

void EdEventFd::setOnListener(IEventFd* cb)
{
	mOnListener = cb;
}

} /* namespace edft */
