/*
 * testmain.cpp
 *
 *  Created on: Jan 12, 2015
 *      Author: netmind
 */

#include <iostream>
#include <gtest/gtest.h>
#include "../ednio/EdNio.h"
#include "../ednio/EdTimerPool.h"

using namespace std;
using namespace edft;
using namespace std::chrono;

int main(int argc, char* argv[])
{
//	::testing::GTEST_FLAG(filter) = "timer.perf";
//	::testing::GTEST_FLAG(filter) = "ipc.mq";
//	::testing::GTEST_FLAG(filter) = "etc.*";
//	::testing::GTEST_FLAG(filter) = "event.eventfd";
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

