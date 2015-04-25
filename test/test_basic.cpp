/*
 * test_basic.cpp
 *
 *  Created on: Mar 10, 2015
 *      Author: netmind
 */

#define DBGTAG "TIMRT"
#define DBG_LEVEL DBG_DEBUG

#include <gtest/gtest.h>
#include <ednio/EdNio.h>
#include <chrono>

#include "tglobal.h"

using namespace testing;
using namespace edft;

TEST(basic, task)
{
	class MyTask : public EdTask
	{
		int OnEventProc(EdMsg& msg) override
		{
			if(msg.msgid == EDM_INIT)
			{
				postExit();
			}
			else if(msg.msgid == EDM_CLOSE)
			{

			}
			return 0;
		}

	};

	FDCHK_S()
	MyTask task;
	task.run();
	task.wait();
	FDCHK_E()
}


TEST(basic, timer)
{
	using namespace std::chrono;
	static int wtime = 500;
	static system_clock::time_point stp;
	static system_clock::time_point etp;

	class MyTask : public EdTask
	{
		EdTimer mTimer;
		int OnEventProc(EdMsg& msg) override
		{
			if(msg.msgid == EDM_INIT)
			{
				mTimer.setOnListener([this](EdTimer& timer){
					etp = system_clock::now();
					timer.kill();
					postExit();
				});

				stp = system_clock::now();
				mTimer.set(wtime);
			}
			else if(msg.msgid == EDM_CLOSE)
			{
				auto dur = duration_cast<milliseconds>(etp-stp);
				dbgd("timer expired, dur=%d", dur.count());
				if((dur.count() < wtime-20) || (dur.count() > wtime+20))
				{
					assert(0);
				}
			}
			return 0;
		}

	};

	FDCHK_S()
	MyTask task;
	task.run();
	usleep(wtime*1000+20);
	task.terminate();
	FDCHK_E()
}
