//============================================================================
// Name        : testednio.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Test Driven Development for ednio
//============================================================================

#define DBGTAG "main0"
#define DBG_LEVEL DBG_DEBUG

#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <list>

#include "EdNio.h"
#include "mariadb/EdMdb.h"
#include "mariadb/EdMdbCnn.h"
#include "mariadb/EdMdbQueryStore.h"
#include "mariadb/EdMdbQuery.h"

void levlog(int lev, const char *tagid, int line, const char *fmtstr, ...)
{
	struct timeval tm;
	va_list ap;

	gettimeofday(&tm, NULL);
	struct tm* ptr_time = localtime(&tm.tv_sec);

	char buf[2048];

	int splen = 2 * lev;
	char spbuf[splen + 1];
	memset(spbuf, ' ', splen);
	spbuf[splen] = 0;

	va_start(ap, fmtstr);
	vsnprintf(buf, 4096 - 1, fmtstr, ap);
	va_end(ap);

	printf("%02d:%02d:%02d.%02d [%s]:%-5d %s%s\n", ptr_time->tm_hour, ptr_time->tm_min, ptr_time->tm_sec, (int) (tm.tv_usec / 10000), tagid, line, spbuf, buf);
}

#define logm(...) {levlog(0, "MTEST", __LINE__, __VA_ARGS__); }
#define logs(...) {levlog(1, "SUB  ", __LINE__, __VA_ARGS__); }
#define logss(...) { levlog(2, "SUB2 ", __LINE__, __VA_ARGS__);  }

using namespace std;
using namespace edft;

long _gStartFds;

void init_test()
{
#if USE_SSL
	char gTestKey[] = ""
			"-----BEGIN RSA PRIVATE KEY-----\n"
			"Proc-Type: 4,ENCRYPTED\n"
			"DEK-Info: DES-EDE3-CBC,530F6BF4FC4863E5\n"
			"\n"
			"ompWbbl2zkVdwPVzPbe6YCLxOWjLld+JXrb2uDLIVKU6yc4YraUm7S10KXeWp2Ff\n"
			"WwDR6IGrloliZTCmsBK/Ol93A+awAdftEW95bmMHnrga0ErJ905U7ijzkrlfPOgl\n"
			"obW8nO24z6XrRPYruxUKizTA9ZGH02Ds4MtbmaL7lGZlPcL/Vm36mJPSslQA08Sv\n"
			"gsfitXnGWZLENkKr4ThYRpQtrd+NM0KEvXoHU/juc4AjMg+P3nMnwHk1HZuD7mH5\n"
			"Ks0ulh6w5BVJGvnRFKfiTrNItWJF4zWszc33f/i5lo1yMGMX2qnZyR+Q2WnyAEpz\n"
			"+8wxqA9Ck4gz5SUIoE6GUGjnV9WsEO+I/0DkD5Dd49qcVSYRG5XPyXUJ+LO+ilAF\n"
			"HT1G1NHIQ3PVLeVlHUaJW97tdH8iXhG57onVlak/8Jj7l0P6LFmijp1PWaNJX7mh\n"
			"gXRwkaReAGvC8YbcX99SLwqZvMzAxWEV9y0Ro0VF/qZX5rhnL/4zgMeuE7Ee7wz2\n"
			"KNcpXzOYQNxKYww66HPBeKgsbMjjA67JD4+5QaaAdK3ENtZMTjhnOk5NkLyEn6cB\n"
			"20426Pq6e7CJd7Crz+c0Ghev7SmEadLxw1AI3zISxnScWMff4SsFD5vK29Xmj/vx\n"
			"XqX9IYIWQtEqnzZO1f31wmcO4qrugJB/xbtsLJ1VDuqKPgz7jzOy1CI3H+Z7CrJG\n"
			"iqj8kTUc8rNDU7GEcFBGSI4FWZsu8Km81x7u62tMin8xuNkw3yGVmnG8ULlpMHe4\n"
			"fwED4zDbfwWGoBLIKXvH0E1yaQ2DiRci2DiDPMqw5ZV/N7bha3RxkQ==\n"
			"-----END RSA PRIVATE KEY-----\n";

	char gTestCrt[] = ""
			"-----BEGIN CERTIFICATE-----\n"
			"MIICkjCCAfugAwIBAgIJAOa4Z0DKS6HuMA0GCSqGSIb3DQEBBQUAMGIxCzAJBgNV\n"
			"BAYTAktSMRAwDgYDVQQIDAdLZW9uZ0tpMQ8wDQYDVQQHDAZZb25nSW4xCzAJBgNV\n"
			"BAoMAktUMREwDwYDVQQLDAhSZXNlYXJjaDEQMA4GA1UEAwwHbmV0bWluZDAeFw0x\n"
			"NDEwMTAxMTQzMDJaFw0xNTEwMTAxMTQzMDJaMGIxCzAJBgNVBAYTAktSMRAwDgYD\n"
			"VQQIDAdLZW9uZ0tpMQ8wDQYDVQQHDAZZb25nSW4xCzAJBgNVBAoMAktUMREwDwYD\n"
			"VQQLDAhSZXNlYXJjaDEQMA4GA1UEAwwHbmV0bWluZDCBnzANBgkqhkiG9w0BAQEF\n"
			"AAOBjQAwgYkCgYEAwwlc2e0e15t8jHZer50VCPGT6K/9AOw7XISzglc++eQjZWUT\n"
			"ndnK6Den/YJQ/DNPDr+wNzftDtLmxElNjBq8y2KuUvzf4KsBCI3prxZ0GoXO5jS4\n"
			"jXCiZsl6tcnoI18KKFZBFAivxwreGC7Fp91la9qpWR4c7Xnlx1XFa+KEJbUCAwEA\n"
			"AaNQME4wHQYDVR0OBBYEFFwYCLFukaxA1rHdhZEO1opHr3kkMB8GA1UdIwQYMBaA\n"
			"FFwYCLFukaxA1rHdhZEO1opHr3kkMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQEF\n"
			"BQADgYEAtNSgLL7bsRZk4rIF0Ns4I3cGzjtyq356r2ziTtu54NOGygQPu/YPeP9O\n"
			"G01wGyb4oXDz+waHQUgF/nsP9Z/jO001ca6+vID3zdnYImGvrFo86RV9yDEANvxI\n"
			"DwuB0mO5Hbd2zf5Q8fqe/BdJCLukaDS4H87gUL5h6ejUoBsjXmc=\n"
			"-----END CERTIFICATE-----\n";
	EdFile file;
	file.openFile("/tmp/test.key", EdFile::OPEN_RWTC);
	file.writeStr(gTestKey);
	file.closeFile();

	file.openFile("/tmp/test.crt", EdFile::OPEN_RWTC);
	file.writeStr(gTestCrt);
	file.closeFile();
#endif
}

int get_num_fds()
{
	int fd_count;
	char buf[300];
	struct dirent *dp;

	snprintf(buf, 256, "/proc/%i/fd/", getpid());

	fd_count = 0;
	DIR *dir = opendir(buf);
	while ((dp = readdir(dir)) != NULL)
	{
		//if(!(dp->d_type & DT_DIR))	logs("file = %s", dp->d_name);
		fd_count++;
	}
	closedir(dir);
	return fd_count;
}

void fdcheck_start()
{
	_gStartFds = get_num_fds();

}

void fdcheck_end()
{
	long fdn = get_num_fds();
	if (_gStartFds != fdn)
	{
		logm("### Fail: fd count check error, start=%ld, end=%ld", _gStartFds, fdn);
		assert(0);
	}
}

class TestTask: public EdTask
{
	std::list<int> mTestList;
public:
	void nextTest()
	{
		if (mTestList.size() > 0)
		{
			int s = mTestList.front();
			mTestList.pop_front();
			postMsg(s);
		}
		else
		{
			postExit();
		}
	}

	void addTest(int t)
	{
		mTestList.push_back(t);
	}
};

// test task
void testtask(int mode)
{
	enum
	{
		TS_NORMAL = EDM_USER + 1,
	};
	class MainTask: public TestTask
	{
		int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				addTest(TS_NORMAL);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

			}
			else if (pmsg->msgid == TS_NORMAL)
			{
				logs("== Start normal test...");
				logs("== Normal test ok...\n");
				nextTest();
			}
			return 0;
		}
	};

	logm(">>>> Test: Task, mode=%d", mode);
	fdcheck_start();
	auto task = new MainTask;
	task->run(mode);
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< Task test OK\n");
}

// message exchanging test
void testmsg(int mode)
{

	class MsgChildTestTask: public EdTask
	{
		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				logs("child init");
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				logs("child close task");
			}
			else
			{
				logs("child received msg. id=%d, p1=%x, p2=%x", pmsg->msgid, pmsg->p1, pmsg->p2);
			}
			return 0;
		}
	};

	class MsgTestTask: public TestTask
	{
		enum
		{
			TS_BASIC_MSG = EDM_USER + 1, TS_OBJECT_MSG, TS_BETWEEN_TASK,

			BASIC_MSG = 10000, OBJECT_MSG, CHILD_TASK_END_MSG,
		};
		MsgChildTestTask* pchild;

		EdTimer *mTimer;

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				addTest(TS_BASIC_MSG);
				addTest(TS_OBJECT_MSG);
				addTest(TS_BETWEEN_TASK);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				logs("msg task closing...");
			}
			else if (pmsg->msgid == TS_BASIC_MSG)
			{
				logs("== Start message parameter check...");
				postMsg(BASIC_MSG, 0xfacdab58, 0x90bc23aa);
			}
			else if (pmsg->msgid == BASIC_MSG)
			{
				if (pmsg->p1 != 0xfacdab58 || pmsg->p2 != 0x90bc23aa)
				{
					logs("### Fail: mesasge parameter check fail");
					assert(0);
				}
				logs("== End message parameter check...OK");
				nextTest();
			}
			else if (pmsg->msgid == TS_OBJECT_MSG)
			{
				logs("== Start object message check...");
				mTimer = new EdTimer;
				postObj(OBJECT_MSG, (void*) mTimer);
			}
			else if (pmsg->msgid == OBJECT_MSG)
			{
				if (pmsg->obj != (void*) mTimer)
				{
					logs("### Fail: object message check error");
					assert(0);
				}
				delete mTimer;
				logs("== End object message check...");
				nextTest();
			}
			else if (pmsg->msgid == TS_BETWEEN_TASK)
			{
				logs("== Start mesage test between tasks...");
				pchild = new MsgChildTestTask;
				logs("start child task...");
				pchild->run();
				postMsg(CHILD_TASK_END_MSG);
				pchild->postMsg(EDM_USER + 1);

			}
			else if (pmsg->msgid == CHILD_TASK_END_MSG)
			{
				logs("post terminating child task");
				pchild->postExit();
				usleep(10000);
				pchild->sendMsg(EDM_USER + 2);
				logs("waiting child task");
				pchild->wait();
				delete pchild;
				nextTest();
			}
			else
			{
				logs("### invalid test scenario");
				assert(0);
			}
			return 0;
		}
	};

	fdcheck_start();
	logm(">>>> Test: Message, mode=%d", mode);
	MsgTestTask msgtask;
	msgtask.run(mode);
	msgtask.wait();
	logm("<<<< Message Test OK\n");
	fdcheck_end();
}

void testSocket(int mode)
{
	enum
	{
		TS_TCP_SOCKET = EDM_USER + 1,
		TS_UNIX_SOCKET,
	};

	class SocketTestTask: public TestTask, public EdSocket::ISocketCb
	{
		EdSocket* mSvrSock;
		EdSocket* mChildSock;


		int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				mChildSock = NULL, mSvrSock = NULL;
				mSvrSock = new EdSocket;
				mSvrSock->setOnListener(this);
				mSvrSock->listenSock(9090);

				addTest(TS_TCP_SOCKET);
				addTest(TS_UNIX_SOCKET);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				mSvrSock->close();
				delete mSvrSock;
				mSvrSock = NULL;
			}
			else if (pmsg->msgid == TS_TCP_SOCKET)
			{
				tcpclient_subtest();
			}
			else if(pmsg->msgid == TS_UNIX_SOCKET)
			{
				udpclient_subtest();
			}
			return 0;
		}

		void tcpclient_subtest()
		{
			static const char *str = "send socket data...";
			static string recvStr;
			class _Client: public EdSocket
			{
				void OnConnected()
				{
					logs("connected...");
					recvStr.clear();
					this->send(str, strlen(str));
				}
				void OnDisconnected()
				{
					logs("disconnected...");
					logs("  unexpected...");
					assert(0);
				}
				void OnRead()
				{
					char buf[100];
					int rcnt = recv(buf, 100);
					if (rcnt > 0)
					{
						recvStr.append(buf, rcnt);
						if (recvStr == str)
						{
							logs("tcp client send/recv ok...");
							logs("== Tcp client test ok.../n");
							delete this;
							((TestTask*) getCurrentTask())->nextTest();
						}
					}
				}

			};
			logs("== Start tcp client test ...");
			auto client = new _Client;
			client->connect("127.0.0.1", 9090);

		}

		void udpclient_subtest()
		{
			logs("== Start unix socket test ...");
			static char *_msg = "client send msg";
			class _UnixDgramServer : public EdSocket {

				void OnRead()
				{
					char buf[100];
					string addr;
					int rcnt = recvFromUnix(buf, 100, &addr);
					logs("unix dgram server on read, cnt=%d, from=%s", rcnt, addr.c_str());
					sendto(addr.c_str(), buf, rcnt);
					close();
					delete this;
				}
			};

			class _UnixDgramClient : public EdSocket {
				void OnConnected() {
					logs("unix connected...");
				}
				void OnDisconnected() {
					logs("unix disconnected ...");
				}
				void OnRead() {
					logs("unix on read");
					string addr;
					char buf[100];
					int rcnt = recvFromUnix(buf, 100, &addr);
					buf[rcnt] = 0;
					if(!strcmp(buf, _msg)) {
						logs("== Unix DGram test ok...\n");
						close();
						delete this;
						((TestTask*)getCurrentTask())->nextTest();
					}
				}
			};

			auto _svrx = new _UnixDgramServer;
			unlink("/tmp/ts.socket");
			_svrx->openSock(SOCK_TYPE_UNIXDGRAM);
			_svrx->bindSock(0, "/tmp/ts.socket");

			auto _client = new _UnixDgramClient;
			unlink("/tmp/tc.socket");
			_client->openSock(SOCK_TYPE_UNIXDGRAM);
			_client->bindSock(0, "/tmp/tc.socket");
			int cret;
			cret = _client->connect("/tmp/ts.socket", 0);

			//int wcnt = _client->sendto("/tmp/ts.socket", _msg, strlen(_msg));
			int wcnt = _client->send(_msg, strlen(_msg));
			logs("wcnt=%d, errno=%d, cret=%d", wcnt, errno, cret);
		}

		void IOnSocketEvent(EdSocket *psock, int event)
		{
			if (psock == mSvrSock)
			{
				if (event == SOCK_EVENT_INCOMING_ACCEPT)
				{
					if (mChildSock == NULL)
					{
						mChildSock = new EdSocket;
						psock->acceptSock(mChildSock, this);
					}
					else
					{
						logs("### Fail: child socket already exists...");
						assert(0);
					}
				}
				else
				{
					logs("### Fail: unexpected socket event=%x", event);
					assert(0);
				}
			}
			else if (psock == mChildSock)
			{
				if (event == SOCK_EVENT_DISCONNECTED)
				{
					logs("child disconnected...");
					psock->close();
					delete psock;
				}
				else if (event == SOCK_EVENT_READ)
				{
					char buf[100];
					int rcnt = psock->recv(buf, 100);
					if (rcnt > 0)
					{
						buf[rcnt] = 0;
						logs("  recv cnt=%d, str=%s", rcnt, buf);
						psock->send(buf, rcnt);
					}
				}
			}

		}

	};

	logm(">>>> Test: Socket, mode=%d", mode);
	fdcheck_start();
	auto task = new SocketTestTask;
	task->run(mode);
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< Timer Socket OK\n");
}

void testtimer(int mode)
{
#define NORMAL_TIMER_INTERVAL 50 // ms
#define USEC_TIMER_INTERVAL 1 // 1 us
	enum
	{
		TS_NORMAL_TIMER = EDM_USER + 1, TS_USEC_TIMER,
	};
	class TimerTest: public TestTask, public EdTimer::ITimerCb
	{
		EdTimer *mTimer, *mTimerUsec;
		u32 period;
		int mExpCnt;
		int mMsecTargetCnt;
		u64 mUsecCnt, mHitCount;
		u32 usec_starttime;

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				logs("  timer test init");
				addTest(TS_NORMAL_TIMER);
				addTest(TS_USEC_TIMER);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				logs("  timer test closing");
				assert(mTimer == NULL);
			}
			else if (pmsg->msgid == TS_NORMAL_TIMER)
			{
				mExpCnt = 0;

				mTimer = new EdTimer;
				mTimer->setOnListener(this);
				mTimer->set(NORMAL_TIMER_INTERVAL);
				period = EdTime::msecTime();
			}
			else if (pmsg->msgid == TS_USEC_TIMER)
			{
				mUsecCnt = 0;
				mHitCount = 0;
				mTimerUsec = new EdTimer;
				mTimerUsec->setOnListener(this);
				usec_starttime = EdTime::msecTime();
				logs("== start usec timer test, period=1 sec, interval=%d usec", USEC_TIMER_INTERVAL);
				mTimerUsec->setUsec(USEC_TIMER_INTERVAL);
			}
			else if (pmsg->msgid == EDM_TIMER)
			{
				if (pmsg->p1 == 1)
				{

				}

			}
			return 0;
		}

		virtual void IOnTimerEvent(EdTimer* ptimer)
		{

			if (ptimer == mTimer)
			{
				mExpCnt++;
				if (mExpCnt == 1000 / NORMAL_TIMER_INTERVAL)
				{
					int dt = EdTime::msecTime() - period;
					logs("    task timer expire, duration=%d", dt);
					if (dt > 1000 + 10)
					{
						logs("### Fail: timer delayed, duration=%d", dt);
						assert(0);
					}

					mTimer->kill();
					logs(" normal timer test OK, duration=%d", dt);
					delete mTimer;
					mTimer = NULL;
					nextTest();
				}
				else if (mExpCnt > 1000 / NORMAL_TIMER_INTERVAL)
				{
					logs("### Fail : timer expire count error, expect=%d, real=%d", 1000/NORMAL_TIMER_INTERVAL, mExpCnt);
					assert(0);
				}
			}
			else if (ptimer == mTimerUsec)
			{
				mUsecCnt++;
				mHitCount += ptimer->getHitCount();
				u32 targetcnt = 1000000 / USEC_TIMER_INTERVAL;
				if (mHitCount >= targetcnt)
				{
					u32 t = EdTime::msecTime();
					if (t - usec_starttime > 1000 + 10)
					{
						logs("### Fail: usec timer time over!!!, period=%d", t - usec_starttime);
						assert(0);
					}
					logs("usec timer test OK, durationt=%d msec, hit=%d, usec_count=%d", t - usec_starttime, mHitCount, mUsecCnt);
					mTimerUsec->kill();
					delete mTimerUsec;
					mTimerUsec = NULL;
					nextTest();
				}

			}
			else
			{
				assert(0);
			}
		}
	};

	logm(">>>> Test: Timer, mode=%d", mode);
	fdcheck_start();
	auto task = new TimerTest;
	task->run(mode);
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< Timer test OK\n");
}

/*
 * Test scenario for Curl
 */
void testcurl(int mode)
{
#define CONNECT_TIMEOUT 5
#define TEST_DURATION (CONNECT_TIMEOUT+1)
#define LOAD_COUNT 100

	enum
	{
		TS_NORMAL = EDM_USER + 1, TS_TIMEOUT, TS_NOTFOUND, TS_REUSE, TS_LOAD, LOAD_RESULT,
	};
	class CurlTest;
	class LoadCurl: public EdEasyCurl
	{
	public:
		int curlid;
		long recvLen;

		EdTask *mTask;
		LoadCurl(EdTask *task)
		{
			curlid = -1;
			mTask = task;
			recvLen = 0;
		}
		virtual void OnCurlEnd(int status)
		{

			mTask->postMsg(LOAD_RESULT, curlid, 0);
			if (status != 0)
			{
				logs("%d: ### Fail: status error, status=%d", curlid, status);
				assert(0);
			}
			logs("%d: status check ok", curlid);

			long t = getContentLength();
			if (t != recvLen)
			{
				logs("%d: recv data len check error...., recvlen=%ld, content-len=%ld", recvLen, t);
				assert(0);
			}
			logs("%d: body len check Ok", curlid);
			close();
			delete this;
		}

		virtual void OnCurlBody(void *buf, int len)
		{
			recvLen += len;
		}

	};

	class CurlTest: public TestTask, public EdEasyCurl::ICurlResult, public EdEasyCurl::ICurlBody
	{

		EdMultiCurl *mMainCurl;
		EdEasyCurl *mLocalCurl;
		EdEasyCurl mAbnormalCurl;
		EdEasyCurl *mCurlNotFound;
		EdEasyCurl *mReuseCurl;
		LoadCurl *mLoadCurl[1000];
		int mLoadEndCnt;

		int mReuseCnt;
		long mRecvDataSize;

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				mCurlNotFound = NULL;
				mLocalCurl = NULL;

				mMainCurl = new EdMultiCurl;
				mMainCurl->open();

				addTest(TS_NORMAL);
				addTest(TS_NOTFOUND);
				addTest(TS_REUSE);
				addTest(TS_TIMEOUT);
				addTest(TS_LOAD);

//				mTestList.push_back(TS_NORMAL);
//				mTestList.push_back(TS_NOTFOUND);
//				mTestList.push_back(TS_REUSE);
//				mTestList.push_back(TS_TIMEOUT);
//				mTestList.push_back(TS_LOAD);

				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				assert(mLocalCurl == NULL);
				mMainCurl->close();
				delete mMainCurl;
				logs("curl test closed...");
			}
			else if (pmsg->msgid == EDM_TIMER)
			{
				killTimer(pmsg->p1);
				postExit();
			}
			else if (pmsg->msgid == TS_NORMAL)
			{
				logs("== Start normal curl test.........");
				mRecvDataSize = 0;
				mLocalCurl = new EdEasyCurl;
				mLocalCurl->setOnCurlListener(this, this);
				mLocalCurl->open(mMainCurl);
				mLocalCurl->request("http://localhost");

			}
			else if (pmsg->msgid == TS_NOTFOUND)
			{
				logs("== Start notfound curl test.........");
				mCurlNotFound = new EdEasyCurl;
				mCurlNotFound->setOnCurlListener(this);
				mCurlNotFound->setUser((void*) "[curl-notfound]");
				mCurlNotFound->open(mMainCurl);
				mCurlNotFound->request("http://localhost/asdfasdfas");

			}
			else if (pmsg->msgid == TS_TIMEOUT)
			{
				logs("== Start timeout curl test.........");
				mAbnormalCurl.setOnCurlListener(this);
				mAbnormalCurl.setUser((void*) "[curl-notconnected]");
				mAbnormalCurl.open(mMainCurl);
				mAbnormalCurl.request("211.211.211.211", CONNECT_TIMEOUT);
				logs("request abnormal curl=%lx", &mAbnormalCurl);
				logs("this is not going to be connected...");

			}
			else if (pmsg->msgid == TS_REUSE)
			{
				logs("== Start reuse curl test....");
				mReuseCnt = 0;
				mReuseCurl = new EdEasyCurl;
				mReuseCurl->setOnCurlListener(this);
				mReuseCurl->open(mMainCurl);
				mReuseCurl->request("http://localhost");
			}
			else if (pmsg->msgid == TS_LOAD)
			{
				int cnn = LOAD_COUNT;
				mLoadEndCnt = 0;
				logs("== Start load test, try count=%d", cnn);
				int i;
				for (i = 0; i < cnn; i++)
				{
					mLoadCurl[i] = new LoadCurl(this);
					mLoadCurl[i]->setOnCurlListener(this, this);
					mLoadCurl[i]->open(mMainCurl);
					mLoadCurl[i]->curlid = i;
					mLoadCurl[i]->request("http://localhost");
				}
				logs("try %d request...", cnn);
			}
			else if (pmsg->msgid == LOAD_RESULT)
			{
				mLoadEndCnt++;
				if (mLoadEndCnt == LOAD_COUNT)
				{
					logs("all curl end...");
					logs("== End load test");
					nextTest();
				}
			}
			return 0;
		}

		virtual void IOnCurlResult(EdEasyCurl* pcurl, int status)
		{

			logs("curl status = %d, curl=%x", status, pcurl);
			if (pcurl == mLocalCurl)
			{
				if (status != 0)
				{
					logs("### Fail: This curl is expected with normal status code but error status");
					assert(status == 0);
				}
				long len = pcurl->getContentLength();
				if (len != mRecvDataSize)
				{
					logs("### Fail: content length not match");
					assert(0);
				}
				logs("Content length check OK..., len=%ld", mRecvDataSize);

				logs("[curl-normal] : Expected Result. OK.......");
				pcurl->close();
				delete mLocalCurl;
				mLocalCurl = NULL;
				logs("== End normal curl test...\n\n");

				nextTest();
			}
			else if (pcurl == mCurlNotFound)
			{
				logs("%s result: %d", (char* )pcurl->getUser(), pcurl->getResponseCode());
				int code = pcurl->getResponseCode();
				if (code != 404)
				{
					logs("### Fail: 404 response expected, buf code = %d", code);
					assert(0);
				}
				logs("    Not Found Test OK.");
				pcurl->close();
				delete pcurl;
				mCurlNotFound = NULL;
				logs("== End notfound curl test...\n\n");

				nextTest();
			}
			else if (pcurl == &mAbnormalCurl)
			{
				logs("abnormal curl status reported...code=%d", status);
				if (status == 0)
				{
					logs("    ### Fail : For this curl, status must be abnormal...");
					assert(status != 0);
				}
				logs("    %s : %s", (char* )pcurl->getUser(), "Expected Result. OK");
				pcurl->close();
				logs("== End time out curl test...\n\n");

				nextTest();
			}
			else if (pcurl == mReuseCurl)
			{
				if (status != 0)
				{
					logs("### Reuse Test Fail: curl response is not normal. status=%d", status);
					assert(0);
				}

				mReuseCnt++;
				if (mReuseCnt < 5)
				{
					pcurl->reset();
					logs("start new requesting by reusing current curl..., cnt=%d", mReuseCnt);
					pcurl->request("http://localhost");
				}
				else
				{
					logs("== End reuse curl test....\n\n");
					mReuseCurl->close();
					delete mReuseCurl;
					mReuseCurl = NULL;
					nextTest();
				}
			}

			else
			{
				assert(0);
			}
		}

		virtual void IOnCurlHeader(EdEasyCurl* pcurl)
		{

		}

		virtual void IOnCurlBody(EdEasyCurl* pcurl, void* ptr, int size)
		{
			if (pcurl == mLocalCurl)
			{
				mRecvDataSize += size;
				/*
				 char* buf = (char*) malloc(size + 1);
				 assert(buf != NULL);
				 logs("body size = %d", size);
				 memcpy(buf, ptr, size);
				 buf[size] = 0;
				 logs("    body: \n%s", buf);
				 free(buf);
				 */
			}
		}
	};

	logm(">>>> Test: Curl, mode=%d", mode);
	fdcheck_start();
	auto task = new CurlTest;
	task->run();
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< Curl Test OK\n");
}

void testreservefree(int mode)
{
	enum
	{
		TS_SIMPLE_FREE = EDM_USER + 1, TS_SELF_FREE,
	};
	class ReserveFreeTest: public TestTask, public EdEventFd::IEventFd, public EdTimer::ITimerCb
	{
		// simple free test
		EdEventFd* mEv;
		EdTimer* mTimer;
		u64 mEventCnt;

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				addTest(TS_SIMPLE_FREE);
				addTest(TS_SELF_FREE);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				logs("Reserve free test closed...");
			}
			else if (pmsg->msgid == TS_SIMPLE_FREE)
			{
				mEventCnt = 0;

				mEv = new EdEventFd;
				mEv->setOnListener(this);
				mEv->open();
				mEv->raise();

				mTimer = new EdTimer;
				mTimer->setOnListener(this);
				mTimer->set(1000);

			}
			else if (pmsg->msgid == TS_SELF_FREE)
			{
				class ti: public EdTimer::ITimerCb
				{
					virtual void IOnTimerEvent(EdTimer* ptimer)
					{
						logs("self free test timer on");
						ptimer->kill();
						getCurrentTask()->reserveFree(ptimer);
						//delete ptimer;
						logs("free reserved for self free timer");
						((ReserveFreeTest*) getCurrentTask())->nextTest();
						delete this;
					}
				};
				logs("== Start self free object test...");
				EdTimer* mt = new EdTimer;
				mt->setOnListener(new ti);
				mt->set(100);
			}
			return 0;
		}

		virtual void IOnEventFd(EdEventFd *pefd, int cnt)
		{
			if (pefd == mEv)
			{
				mEventCnt++;
				pefd->raise();
			}
		}

		virtual void IOnTimerEvent(EdTimer* ptimer)
		{
			if (ptimer == mTimer)
			{
				logs("timer on, raise cnt=%ld", mEventCnt);
				ptimer->kill();
				mEv->close();
				reserveFree(ptimer);
				reserveFree(mEv);
				logs("reserved free objs...");
				nextTest();
			}
		}

	};

	logm(">>>> Test: ReserveFree, mode=%d", mode);
	fdcheck_start();
	auto task = new ReserveFreeTest;
	task->run();
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< Reserve Free Test OK\n\n");
}

void testMainThreadTask(int mode)
{
	class MainThreadTask: public EdTask
	{
		virtual int OnEventProc(EdMsg *pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				logs("task init event");
				postMsg(EDM_USER);
				postExit();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				logs("task close event");
			}
			else if (pmsg->msgid == EDM_USER)
			{
				logs("user event msg");
			}
			return 0;
		}
	};

	logm("== Start main thread task test...");
	fdcheck_start();
	MainThreadTask task;
	task.runMain(mode);
	fdcheck_end();
	logm("== End main thread task test...\n\n");
}

void testMultiTaskInstance(int mode)
{
#define TASK_INSTANCE_NUM 10
	enum
	{
		UM_TASK_START = EDM_USER + 1,
	};
	class MultiTestTask: public TestTask
	{

	public:
		long hitCount;

		u64 getHitCount()
		{
			return hitCount;
		}
		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				hitCount = 0;
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

			}
			else if (pmsg->msgid == UM_TASK_START)
			{
				class efd: public EdEventFd, public EdTimer::ITimerCb
				{
					u64 mOnCnt;
					EdTimer timer;
					int mId;
					u32 mStartTime;
					MultiTestTask* mTask;
				public:
					efd(MultiTestTask* ptask)
					{
						mOnCnt = 0;
						mId = -1;
						mTask = ptask;
					}
					void start(int id)
					{
						logs("== Start task, id=%d", id);
						mId = id;
						timer.setOnListener(this);
						timer.set(1000);
						open();
						raise();
						mStartTime = EdTime::msecTime();
					}
					void OnEventFd(int cnt)
					{
						mOnCnt++;
						mTask->hitCount += cnt;
						raise();
					}

					void IOnTimerEvent(EdTimer* ptimer)
					{
						u32 t = EdTime::msecTime();
						logs("[%2d] : end-time=%d, on-count=%ld, hit-count=%ld", mId, t - mStartTime, mOnCnt, mTask->hitCount);
						ptimer->kill();
						close();
						logs("end.....");
						getCurrentTask()->postExit();
						delete this;

					}
				};
				efd *pfd = new efd(this);
				pfd->start(pmsg->p1);
			}
			return 0;
		}
	};

	logm(">>>> Test: Multi task instance, mode=%d", mode);
	fdcheck_start();
	MultiTestTask task[TASK_INSTANCE_NUM];
	for (int i = 0; i < TASK_INSTANCE_NUM; i++)
	{
		task[i].run();
		task[i].postMsg(UM_TASK_START, i);
	}

	int totalhit = 0;
	for (int i = 0; i < TASK_INSTANCE_NUM; i++)
	{
		task[i].wait();
		totalhit += task[i].getHitCount();
	}
	logm("Total hit count=%ld", totalhit);
	fdcheck_end();
	logm("<<<< Multi task instance OK\n\n");

}

void testHttpBase(int mode)
{
	enum
	{
		TS_STRING_READER = EDM_USER + 1,
	};
	class BaseHttp: public TestTask
	{
		EdHttpStringReader mReader;

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{

				addTest(TS_STRING_READER);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

			}
			else if (pmsg->msgid == TS_STRING_READER)
			{
				long rdcnt;
				char data[] = "0123456789";
				char buf[100];
				long count = 0;
				mReader.setString(data);
				rdcnt = 0;
				count = 0;

				rdcnt = mReader.IReadBodyData(buf + count, 3);
				assert(rdcnt == 3);
				count += rdcnt;

				rdcnt = mReader.IReadBodyData(buf + count, 3);
				assert(rdcnt == 3);
				count += rdcnt;

				rdcnt = mReader.IReadBodyData(buf + count, 3);
				assert(rdcnt == 3);
				count += rdcnt;

				rdcnt = mReader.IReadBodyData(buf + count, 3);
				assert(rdcnt == 1);
				count += rdcnt;

				rdcnt = mReader.IReadBodyData(buf + count, 3);
				assert(rdcnt == -1);

				if (memcmp(data, buf, strlen(data)))
				{
					logs("### Fail: string reader no data match");
					assert(0);
				}

				nextTest();
			}

			return 0;
		}
	};

	logm(">>>> Test: Base Http , mode=%d", mode);
	fdcheck_start();
	BaseHttp task;
	task.runMain(mode);
	fdcheck_end();
	logm("<<<< Base Http OK\n\n");
}

void testHttpSever(int mode)
{
	static bool serverEnd = false;
	static EdHttpServer* server = NULL;

	enum
	{
		TS_NORMAL = EDM_USER + 1, TS_SERVER_END, TS_MULTIPART,
	};
	class MyHttpTask;
	class MyController;
	class FileCtrl;
	class UpFileCtrl;
	class MultipartCtrl;
	class UrlImagePathCtrl;
	class UrlPngPathCtrl;

	class MyHttpTask: public EdHttpTask
	{
		EdHttpStringWriter *mWriter;
		EdHttpStringReader *mReader;

	public:

		virtual int OnEventProc(EdMsg* pmsg)
		{
			int ret = EdHttpTask::OnEventProc(pmsg);
			if (pmsg->msgid == EDM_INIT)
			{
#if USE_SSL
				setDefaultCertPassword("123456");
				setDefaultCertFile("/tmp/test.crt", "/tmp/test.key");
#endif
				regController<MyController>("/userinfo", NULL);
				regController<FileCtrl>("/getfile", NULL);
				regController<UpFileCtrl>("/upfile", NULL);
				regController<MultipartCtrl>("/multi", NULL);

				regControllerPath<UrlImagePathCtrl>("/image", NULL);
				regControllerPath<UrlPngPathCtrl>("/image/png", NULL);
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				logs("free ssl ...");
				EdSSLContext::freeDefaultEdSSL();
			}
			return ret;
		}

	};

	class MyController: public EdHttpController, public EdTimer::ITimerCb
	{
		EdHttpStringReader *mStrReader;
		EdTimer mTimer;
		MyHttpTask *mMyTask;
	public:
		MyController()
		{
			mStrReader = NULL;
			mMyTask = (MyHttpTask*) EdTask::getCurrentTask();
			logs("mycont const.....");
		}
		virtual void OnHttpRequestHeader()
		{
			logs("after 100msec, send response...");
			mTimer.setOnListener(this);
			mTimer.set(100);
		}

		virtual void OnHttpDataRecvComplete(EdHttpContent* pctt)
		{
			;
		}
		;

		virtual void OnHttpComplete(int result)
		{
			logs("http complete...result=%d", result);
			delete mStrReader;
			mStrReader = NULL;
			serverEnd = true;
		}

		virtual void IOnTimerEvent(EdTimer* ptimer)
		{
			logs("send response,...");
			ptimer->kill();
			mStrReader = new EdHttpStringReader;
			mStrReader->setString("Hello, ednio http service....\n");
			setRespBodyReader(mStrReader, "text/plain");
			setHttpResult("200");
		}

	};

	class FileCtrl: public EdHttpController
	{
		EdHttpFileReader reader;
		void OnInit()
		{
			logs("file ctrl on init...");
		}
		;
		virtual void OnHttpRequestHeader()
		{
			reader.open("/home/netmind/bb");
			setRespBodyReader(&reader, "application/zip");
			setHttpResult("200");
		}
		;

		virtual void OnHttpComplete(int result)
		{
			logs("file ctrl complete, result=%d", result);
			reader.close();
		}

	};

	class UpFileCtrl: public EdHttpUploadCtrl
	{
		void OnHttpCtrlInit()
		{
			logs("upfile request method=%d, url=%s", getReqMethod(), getReqUrl().c_str());
			if (getReqMethod() != HTTP_PUT)
			{
				setHttpResult("400");
				return;
			}
			setPath("/tmp/ednio/upfile.dat");
		}
		void OnHttpRequestHeader()
		{
			EdHttpUploadCtrl::OnHttpRequestHeader();
			logs("  content=%s", getReqHeader("Content-Type"));
		}
		void OnHttpRequestMsg()
		{
			logs("up file request msg...");
			setHttpResult("200");
		}

	};

	class MultipartCtrl: public EdHttpDefMultiPartCtrl
	{
		EdHttpStringReader reader;
		void OnHttpRequestHeader()
		{
			auto task = (EdHttpTask*) EdTask::getCurrentTask();
			logs("  recv buf size=%d", task->getConfig()->recv_buf_size);
			setFileFolder("/tmp/ednio");
		}
		void OnHttpRequestMsg()
		{
			logs("on multipart request msg, ");

			string info = getData("info");
			if (info.size() > 0)
			{
				logs("info = %s", info.c_str());
				reader.setString("info received...\r\n");
				setRespBodyReader(&reader, "text/plain");
				setHttpResult("200");
			}
			else
			{
				logs("### Fail: not found info value ... ");
				setHttpResult("400");
			}
		}
		void OnHttpComplete(int result)
		{
			logs("mp complete, result=%d", result);
		}
	};

	class UrlImagePathCtrl: public EdHttpController
	{
		void OnHttpCtrlInit()
		{
			logs("url image ctrl init...");
		}
		void OnHttpRequestHeader()
		{
			string url = getReqUrl();
			logs("url is: %s", url.c_str());

			sendHttpResp("200");
		}
	};

	class UrlPngPathCtrl: public EdHttpController
	{
		void OnHttpCtrlInit()
		{
			logs("url png path ctrl init...");
		}
		void OnHttpRequestHeader()
		{
			string url = getReqUrl();
			logs("url is: %s", url.c_str());
			string ctrlpat = EdUrlParser::getRelativePath("/image/png", "/image");
			logs("relative path: %s", ctrlpat.c_str());
			sendHttpResp("200");
		}
	};

	class HttpTestTask: public TestTask
	{
	public:
		virtual ~HttpTestTask()
		{
		}
		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				server = new EdHttpServer;

				addTest(TS_NORMAL);
				addTest(TS_SERVER_END);
				nextTest();

			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				server->stopService();
				delete server;
				server = NULL;

				EdSSLContext::freeDefaultEdSSL();
			}
			else if (pmsg->msgid == TS_NORMAL)
			{
				logs("== Start server normal test ...");
				EdHttpSettings settings = EdHttpServer::getDefaultSettings();
				settings.port = 9090;
				settings.ssl_port = 7070;
				logs("server open, port=%d, task-instance=%d", settings.port, settings.task_num);
				server->startService<MyHttpTask>(&settings);

				nextTest();
			}
			else if (pmsg->msgid == TS_SERVER_END)
			{
				setTimer(1, 500);
			}
			else if (pmsg->msgid == EDM_TIMER)
			{
				if (pmsg->p1 == 1)
				{
					if (serverEnd == true)
					{
						postExit();
						killTimer(pmsg->p1);
					}
				}
			}
			return 0;
		}
	};

	logm(">>>> Test: Http Server, mode=%d", mode);
	fdcheck_start();
	HttpTestTask *task = new HttpTestTask;
	task->runMain(mode);
	delete task;
	fdcheck_end();
	logm("<<<< HttpServer OK\n\n");
}

#if USE_SSL
void testssl(int mode)
{
	enum
	{
		TS_SSL = EDM_USER + 1, TS_SMART_SOCK,
	};
	class SSLTestTask: public TestTask, public EdSSLSocket::ISSLSocketCb, public EdSmartSocket::INet
	{
		EdSSLSocket *ssl;
		int sslReadCnt;

		// smart sock test
		EdSmartSocket* smartSock;

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				ssl = NULL;
				smartSock = NULL;

				addTest(TS_SSL);
				//addTest(TS_SMART_SOCK);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

				EdSSLContext::freeDefaultEdSSL();
			}
			else if (pmsg->msgid == TS_SSL)
			{
				logs("== Start basic ssl client test...");
				ssl = new EdSSLSocket;
				sslReadCnt = 0;
				ssl->openSSLClientSock();
				ssl->setOnSSLListener(this);
				ssl->connect("127.0.0.1", 443);
			}
			else if (pmsg->msgid == TS_SMART_SOCK)
			{
				logs("== Start smart sock test...");
				smartSock = new EdSmartSocket;
				smartSock->socketOpen(true);
				smartSock->setOnNetListener(this);
				smartSock->connect("127.0.0.1", 443);
			}
			return 0;
		}

		virtual void IOnSSLSocket(EdSSLSocket *psock, int event)
		{
			if (event == SSL_EVENT_CONNECTED)
			{
				logs("ssl connected...");
				char req[] = "GET / HTTP/1.1\r\n"
						"User-Agent: curl/7.22.0 (x86_64-pc-linux-gnu) libcurl/7.22.0 OpenSSL/1.0.1 zlib/1.2.3.4 libidn/1.23 librtmp/2.3\r\n"
						"Host: 127.0.0.1\r\n"
						"Accept: */*\r\n\r\n";
				ssl->send(req, strlen(req));

			}
			else if (event == SSL_EVENT_DISCONNECTED)
			{
				logs("ssl disconnected..., cur read cnt=%d", sslReadCnt);
				if (sslReadCnt < 177)
				{
					logs("### Fail: ssl read count mismatch, count=%d", sslReadCnt);
					assert(0);
				}
				ssl->close();

				logs("== basic ssl client test OK...\r\n");
				nextTest();
			}
			else if (event == SSL_EVENT_READ)
			{
				char buf[8 * 1024 + 1];
				int rdcnt = ssl->recv(buf, 8 * 1024);
				if (rdcnt > 0)
				{
					sslReadCnt += rdcnt;
					buf[rdcnt] = 0;
					logs(buf);
					if (sslReadCnt == 462)
					{
						ssl->close();
						reserveFree(ssl);
						ssl = NULL;
						nextTest();
					}
				}
			}

		}

		virtual void IOnNet(EdSmartSocket *psock, int event)
		{
			if (event == NETEV_CONNECTED)
			{
				logs("sm on connected..");
				char req[] = "GET / HTTP/1.1\r\n"
						"User-Agent: curl/7.22.0 (x86_64-pc-linux-gnu) libcurl/7.22.0 OpenSSL/1.0.1 zlib/1.2.3.4 libidn/1.23 librtmp/2.3\r\n"
						"Host: 127.0.0.1\r\n"
						"Accept: */*\r\n\r\n";
				psock->sendPacket(req, strlen(req));
			}
			else if (event == NETEV_DISCONNECTED)
			{
				logs("sm on disconnected..");
				psock->socketClose();
				delete psock;
				nextTest();
			}
			else if (event == NETEV_READ)
			{
				logs("sm on read");
				char buf[1000];
				int rcnt = psock->recvPacket(buf, 500);
				if (rcnt > 0)
				{
					buf[rcnt] = 0;
					logs(buf);
				}
			}
		}
	};

	logm(">>>> Test: SSL, mode=%d", mode);
	fdcheck_start();
	auto task = new SSLTestTask;
	task->run(mode);
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< SSL test OK\n");
}
#endif

void testsmartsock(int mode)
{
	enum
	{
		TS_ECHO = EDM_USER + 1, TS_PENDING, TS_SSL,
	};

	class ClientTask: public TestTask, public EdSmartSocket::INet
	{

		EdSmartSocket* mEchoSock;
		char echostr[100];

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				logs("client start...");
				addTest(TS_ECHO);
				addTest(TS_PENDING);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

			}
			else if (pmsg->msgid == TS_ECHO)
			{
				logs("== Start normal test");
				strcpy(echostr, "'echo message'");
				mEchoSock = new EdSmartSocket;
				mEchoSock->setOnNetListener(this);
				mEchoSock->socketOpen();
				mEchoSock->connect("127.0.0.1", 7000);

			}
			else if (pmsg->msgid == TS_PENDING)
			{
				subtestpending();
			}
			else if (pmsg->msgid == TS_SSL)
			{

			}
			return 0;
		}

		void subtestpending()
		{
			static EdSmartSocket* pdSock;

			class echoimpl: public EdSmartSocket::INet
			{
				long writeCnt;
				void IOnNet(EdSmartSocket* psock, int event)
				{
					if (event == NETEV_CONNECTED)
					{
						logs("pending connected...");
						writeCnt = 0;
						char buf[1024];
						int ret;
						for (;;)
						{
							memset(buf, 0, 1024);
							ret = psock->sendPacket(buf, 1024);
							if (ret == SEND_PENDING)
							{
								writeCnt += 1024;
								logs("send pending... cur wrietcnt=%d", writeCnt);
								break;
							}
							else if (ret == SEND_FAIL)
							{
								logs("### Fail: send fail......writeCnt=%d", writeCnt);
								assert(0);
							}
							else if (ret == SEND_OK)
							{
								writeCnt += 1024;
							}
						}

					}
					else if (event == NETEV_DISCONNECTED)
					{
						logs("pending disconnected...");
						delete psock;
						delete this;
					}
					else if (event == NETEV_SENDCOMPLETE)
					{
						logs("pending send complete...write cnt=%d", writeCnt);
						usleep(1000 * 1000);
						psock->close();
						getCurrentTask()->reserveFree(psock);
						delete this;
						((TestTask*) getCurrentTask())->nextTest();
					}
				}
			};
			echoimpl *pif = new echoimpl;
			;
			pdSock = new EdSmartSocket;
			pdSock->setOnNetListener(pif);
			pdSock->socketOpen();
			pdSock->connect("127.0.0.1", 7001);

		}

		virtual void IOnNet(EdSmartSocket* psock, int event)
		{
			if (psock == mEchoSock)
			{
				if (event == NETEV_CONNECTED)
				{
					logs("echo connected...");
					int ret = mEchoSock->sendPacket(echostr, strlen(echostr));
					if (ret != SEND_OK)
					{
						logs("### Fail: send fail...");
						assert(0);
					}
				}
				else if (event == NETEV_DISCONNECTED)
				{
					logs("echo disconnected...");
				}
				else if (event == NETEV_READ)
				{
					int rcnt;
					char buf[100];
					rcnt = psock->recvPacket(buf, 100);
					if (rcnt > 0)
					{
						buf[rcnt] = 0;
						if (!strcmp(echostr, buf))
						{
							logs("echo test ok...\n");
							nextTest();
							psock->socketClose();
							reserveFree(psock);
							mEchoSock = NULL;
						}
						else
						{
							logs("### Fail: echo string mismatch...");
							assert(0);
						}
					}
				}
			}
		}
	};

	class ServerTask: public TestTask, public EdSocket::ISocketCb, public EdSmartSocket::INet
	{
		EdSmartSocket *mChildSock;
		EdSocket* mEchoSvrSock;

		// pending test
		EdSocket* mPendingSvr;
		EdSmartSocket* mPendingSock;

		virtual int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				logs("server start...");
				mChildSock = NULL;

				mEchoSvrSock = new EdSocket;
				mEchoSvrSock->setOnListener(this);
				mEchoSvrSock->listenSock(7000);

				mPendingSvr = new EdSocket;
				mPendingSvr->setOnListener(this);
				mPendingSvr->listenSock(7001);
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				mEchoSvrSock->close();
				delete mEchoSvrSock;

				mPendingSvr->close();
				delete mPendingSvr;

			}
			return 0;
		}

		virtual void IOnSocketEvent(EdSocket *psock, int event)
		{
			if (psock == mEchoSvrSock)
			{
				if (event == SOCK_EVENT_INCOMING_ACCEPT)
				{
					int fd = mEchoSvrSock->accept();
					assert(fd > 0);
					assert(mChildSock == NULL);
					mChildSock = new EdSmartSocket;
					mChildSock->setOnNetListener(this);
					mChildSock->socketOpenChild(fd);
				}
			}
			else if (psock == mPendingSvr)
			{
				int fd = mPendingSvr->accept();
				assert(fd > 0);
				class sif: public EdSmartSocket::INet
				{
					void IOnNet(EdSmartSocket* psock, int event)
					{
						static int bwait = 1;
						static int pendReadCnt = 0;
						if (event == NETEV_DISCONNECTED)
						{
							logs("pending sock disconnected..., pend read cnt=%d", pendReadCnt);

							psock->close();
							delete psock;
							delete this;
						}
						else if (event == NETEV_READ)
						{
							//logs("pending sock on read");
							if (bwait == 1)
							{
								usleep(1000 * 1000);
								bwait = 0;
							}
							char buf[1000];
							int rcnt = psock->recvPacket(buf, 500);
							if (rcnt > 0)
							{
								pendReadCnt += rcnt;
							}

						}
					}
				};
				mPendingSock = new EdSmartSocket;
				mPendingSock->setOnNetListener(new sif);
				mPendingSock->socketOpenChild(fd);
			}
		}

		void IOnNet(EdSmartSocket *psock, int event)
		{

			if (event == NETEV_DISCONNECTED)
			{
				logs("child disconnected...");
				psock->close();
				delete psock;
				mChildSock = NULL;
			}
			else if (event == NETEV_CONNECTED)
			{
				assert(0);
			}
			else if (event == NETEV_READ)
			{
				char buf[100 + 1];
				int rcnt = psock->recvPacket(buf, 100);
				if (rcnt > 0)
				{
					buf[rcnt] = 0;
					logs("server recv: %s", buf);
					int ret = psock->sendPacket(buf, rcnt);
					if (ret != SEND_OK)
					{
						logs("### Fail: echo server send fail...");
						assert(0);
					}
				}
				else
				{
					logs("server read fail...ret=%d", rcnt);
				}
			}
		}

	};

	logm(">>>> Test: smart socket, mode=%d", mode);
	fdcheck_start();
	auto stask = new ServerTask;
	stask->run(mode);

	auto ctask = new ClientTask;
	ctask->run(mode);

	ctask->wait();
	stask->terminate();
	delete ctask;
	delete stask;
	fdcheck_end();
	logm("<<<< smart socket test OK\n");
}

void testreadclose(int mode)
{
	enum
	{
		TS_NORMAL = EDM_USER + 1,
	};
	class MainTask: public TestTask, public EdSocket::ISocketCb
	{
		EdSocket *sock;
		int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				addTest(TS_NORMAL);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{

			}
			else if (pmsg->msgid == TS_NORMAL)
			{
				logs("== Start normal test...");
				sock = new EdSocket;
				sock->setOnListener(this);
				sock->connect("127.0.0.1", 4040);
			}
			return 0;
		}

		void IOnSocketEvent(EdSocket *psock, int event)
		{
			if (event == SOCK_EVENT_READ)
			{
				logs("sevt read...");
				char buf[200];
				int rcnt = psock->recv(buf, 100);
				logs("    rcnt = %d", rcnt);
			}
			else if (event == SOCK_EVENT_DISCONNECTED)
			{
				logs("sevt disc...");
				psock->close();
				delete psock;
				nextTest();
			}
			else if (event == SOCK_EVENT_CONNECTED)
			{
				logs("sevt conn...");
			}
			else if (event == SOCK_EVENT_WRITE)
			{
				logs("sevt write...");
			}
		}
	};

	logm(">>>> Test: Task, mode=%d", mode);
	fdcheck_start();
	auto task = new MainTask;
	task->run(mode);
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< Task test OK\n");
}

void testmultipartapi()
{
	class Mp
	{
	public:
		static void onPartBegin(const char *buffer, size_t start, size_t end, void *userData)
		{
			printf("onPartBegin\n");
		}

		static void onHeaderField(const char *buffer, size_t start, size_t end, void *userData)
		{
			printf("onHeaderField: (%s)\n", string(buffer + start, end - start).c_str());
		}

		static void onHeaderValue(const char *buffer, size_t start, size_t end, void *userData)
		{
			printf("onHeaderValue: (%s)\n", string(buffer + start, end - start).c_str());
		}

		static void onPartData(const char *buffer, size_t start, size_t end, void *userData)
		{
			printf("onPartData: (%s)\n", string(buffer + start, end - start).c_str());
		}

		static void onPartEnd(const char *buffer, size_t start, size_t end, void *userData)
		{
			printf("onPartEnd\n");
		}

		static void onEnd(const char *buffer, size_t start, size_t end, void *userData)
		{
			printf("onEnd\n");
		}
	};
	MultipartParser parser;

	parser.onPartBegin = Mp::onPartBegin;
	parser.onHeaderField = Mp::onHeaderField;
	parser.onHeaderValue = Mp::onHeaderValue;
	parser.onPartData = Mp::onPartData;
	parser.onPartEnd = Mp::onPartEnd;
	parser.onEnd = Mp::onEnd;

	static char testmsg[] = "--abcd\r\n"
			"content-type: text/plain\r\n"
			"content-disposition: form-data; name=\"field1\"; filename=\"field1\"\r\n"
			"foo-bar: abc\r\n"
			"x: y\r\n\r\n"
			"hello world\r\n\r\n"
			"x\r\n\r\n"
			"--abcd--\r\n";
	int cnt = 0;
	int bufsize = strlen(testmsg);
	int feedlen;
	parser.setBoundary("abcd");
	int rdcnt = 0;
	for (;;)
	{
		feedlen = min(bufsize - cnt, 5);
		cnt = parser.feed(testmsg + rdcnt, feedlen);
		if (cnt == 0)
			break;
		else
			rdcnt += cnt;
	}
}

#if 0
void testHttpPipeLine(int mode)
{
	enum
	{
		TS_NORMAL = EDM_USER + 1,

	};
	class UserInfoCtrl: public EdHttpController, public EdTimer::ITimerCb
	{
		EdTimer mTimer;
		void OnHttpRequestMsg()
		{
			logs("server request: userinfo, ");
			logs("send response curl#1 after 2 sec ...");
			mTimer.setOnListener(this);
			mTimer.set(2000);
		}

		void IOnTimerEvent(EdTimer* ptimer)
		{
			ptimer->kill();
			setHttpResult("404");
		}
	};

	class AddrCtrl: public EdHttpController
	{
		void OnHttpRequestMsg()
		{
			logs("server request: addr");
			logs("send response curl#2 immediately ...");
			setHttpResult("200");
		}

	};

	class WorkTask: public EdHttpTask
	{
		void OnInitHttp()
		{
			regController<UserInfoCtrl>("/userinfo", NULL);
			regController<AddrCtrl>("/addr", NULL);
		}
	};

	class TestPileLine: public TestTask, public EdEasyCurl::ICurlResult
	{
		EdHttpServer* server;
		EdMultiCurl* mainCurl;
		EdEasyCurl *curl1, *curl2;

		int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				logs("init Pipelining Test Task...");
				server = new EdHttpServer;
				EdHttpSettings hs = EdHttpServer::getDefaultSettings();
				hs.port = 9090;
				hs.ssl_port = 0;
				server->startService<WorkTask>(&hs);

				addTest(TS_NORMAL);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				logs("Pipeline test task closing...");
				mainCurl->close();
				CHECK_DELETE_OBJ(mainCurl);
				server->stopService();
				CHECK_DELETE_OBJ(server);

			}
			else if (pmsg->msgid == TS_NORMAL)
			{
				logs("start normal pipelining...");
				mainCurl = new EdMultiCurl;
				mainCurl->open();
				//mainCurl->setPipelineing(1);
				curl1 = new EdEasyCurl;
				curl1->setOnCurlListener(this);
				curl1->open(mainCurl);

				curl1->request("http://127.0.0.1:9090/userinfo");
				//curl1->request("http://127.0.0.1:9090/addr");

//				curl2 = new EdEasyCurl;
//				curl2->setOnCurlListener(this);
//				curl2->open(mainCurl);
//				curl2->request("http://127.0.0.1:9090/addr");
			}
			return 0;
		}

		void IOnCurlResult(EdEasyCurl* pcurl, int result)
		{
			if (pcurl == curl1)
			{
				int code = pcurl->getResponseCode();
				logs("** curl#1 result=%d, code=%3d", result, code);
				if (code != 200)
				{
					curl1->reset();
					curl1->request("http://127.0.0.1:9090/addr");
				}
				else
				{
					curl1->close();
					CHECK_DELETE_OBJ(curl1);
					nextTest();
				}
			}
			else if (pcurl == curl2)
			{
				int code = pcurl->getResponseCode();
				logs("** curl#2 result=%d, code=%3d", result, code);
				if (curl1 != NULL)
				{
					logs("### Fail: curl2 was terminated before curl1...");
					assert(0);
				}
				logs("== Normal pipelining Test OK\n");
				curl2->close();
				CHECK_DELETE_OBJ(curl2);
				nextTest();
			}
		}
	};

	logm(">>>> Test: HTTP pipelining, mode=%d", mode);
	fdcheck_start();
	auto task = new TestPileLine;
	task->run(mode);
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< HTTP pipelining OK\n");
}
#endif

void testMariadb(int mode)
{
	enum
	{
		TS_CNN_FAIL = EDM_USER + 1, TS_CNN_OK, TS_CREATE_TABLE, TS_QUERY_EARLY, TS_QUERY_PENDING, TS_NORMAL_QUERY_STORE,
	};

	class UserQuery: public EdMdbQueryStore
	{
	public:
		UserQuery(EdMdbCnn* pcnn) :
				EdMdbQueryStore(pcnn)
		{

		}
		void printResult()
		{
			logs("print result...");
			for (;;)
			{
				MYSQL_ROW row = getRow();
				if (row == NULL)
				{
					logs("no more result...");
					break;
				}
				logs("name=%s,  address=%s", row[0], row[1]);
			}
		}
		void OnQueryEnd(MYSQL_RES *res)
		{
			logs(" query end...res=%0x", res);
			if (res == NULL)
			{
				logs("### Fail: normal query result fail...");
				assert(0);
			}
			printResult();
			close();
//			MYSQL_ROW row;
//			for (;;)
//			{
//				row = mysql_fetch_row(res);
//				if (row == NULL)
//					break;
//				dbgd("name=%s, addr=%s", row[1], row[2]);
//			}
			logs("normal query test ok...\n")
			delete this;
			TestTask *task = ((TestTask*) EdTask::getCurrentTask());
			task->nextTest();
		}
	};

	class DbTask: public TestTask, public EdMdbCnn::IMdbCnn
	{

		EdMdbCnn *Cnn;
		UserQuery *qry;

		int OnEventProc(EdMsg* pmsg)
		{
			if (pmsg->msgid == EDM_INIT)
			{
				Cnn = new EdMdbCnn;
				addTest(TS_CNN_FAIL);
				addTest(TS_CNN_OK);
				//addTest(TS_CREATE_TABLE);
				addTest(TS_QUERY_EARLY);
				addTest(TS_QUERY_PENDING);
				addTest(TS_NORMAL_QUERY_STORE);
				nextTest();
			}
			else if (pmsg->msgid == EDM_CLOSE)
			{
				Cnn->closeDb();
				delete Cnn;
				Cnn = NULL;
				EdMySqlEnd();
			}
			else if (pmsg->msgid == TS_CNN_FAIL)
			{
				int cnnret;
				class _mdbstatus: public EdMdbCnn::IMdbCnn
				{
					void IOnMdbCnnStatus(EdMdbCnn* pcnn, int result)
					{
						if (result == 1)
						{
							logs("### Fail: unexpected connection ok...");
							assert(0);
						}

						DbTask *task = ((DbTask*) getCurrentTask());
						task->Cnn->closeDb();
						task->nextTest();
						delete this;
						logs("== expected db disconnected...,  ok.\n");
					}
				};
				logs("== Early disconnected test...");
				_mdbstatus *mdbimpl = new _mdbstatus;
				Cnn->setOnListener(mdbimpl);
				cnnret = Cnn->connectDb("127.0.0.1", "myclient", "netmind", "1234", 5656);
				//int cnnret = Cnn->connectDb("211.23.23.23", 0, "netmind", "1234", "myclient");
				if (cnnret < 0)
				{
					logs("### Fail: connection error,ret=%d", cnnret);
					assert(0);
				}
			}
			else if (pmsg->msgid == TS_QUERY_EARLY)
			{
				testQueryEarly();
			}
			else if (pmsg->msgid == TS_QUERY_PENDING)
			{
				testQueryPending();
			}
			else if (pmsg->msgid == TS_CNN_OK)
			{
				int cnnret;
				Cnn->setOnListener(this);
				cnnret = Cnn->connectDb("127.0.0.1", "myclient", "netmind", "1234");
				if (cnnret < 0)
				{
					logs("### Fail: connection error,ret=%d", cnnret);
					assert(0);
				}
			}
			else if (pmsg->msgid == TS_CREATE_TABLE)
			{
				create_table_subtest();
			}
			else if (pmsg->msgid == TS_NORMAL_QUERY_STORE)
			{
				logs("== start normal query and store...");
				UserQuery *qr = new UserQuery(Cnn);
				int ret, err;
				ret = qr->query("select name,address from userinfo", &err);
				if (ret == MDB_COMPLETE)
				{
					logs("  early query,store...");
					qr->printResult();
					qr->close();
					delete qr;
					nextTest();
				}
			}

			return 0;
		}

		void testQueryEarly()
		{
			logs("== start early query test...");
			class ACmdQry: public EdMdbQuery
			{
				DbTask *mTask;
			public:
				ACmdQry(EdMdbCnn* pcnn) :
						EdMdbQuery(pcnn)
				{
					mTask = (DbTask*) getCurrentTask();
				}

				void printRow(MYSQL_ROW row)
				{
					logs("name=%s,  address=%s", row[0], row[1]);
				}

				int getRows()
				{
					int ret;
					MYSQL_ROW row;
					for (;;)
					{
						ret = fetchRow(&row);
						if (ret == MDB_COMPLETE)
						{
							if (row != NULL)
							{
								printRow(row);
							}
							else
							{
								logs("fetch no result...");
								break;
							}
						}
						else
						{
							break;
						}
					}
					return ret;
				}

				virtual void OnQueryResult(int err)
				{
					logs("query result, err=%d", err);
					if (err == 0)
					{
						int ret = getRows();
						if (ret == MDB_COMPLETE)
						{
							mTask->nextTest();
							close();
							delete this;
							logs("== earyl query test ok...\n");
						}
					}
				}

				virtual void OnFetchRow(MYSQL_ROW row)
				{
					if (row != NULL)
					{
						printRow(row);
						int ret = getRows();
						if (ret == MDB_COMPLETE)
						{
							mTask->nextTest();
							close();
							delete this;
							logs("== earyl query test ok...\n");
						}
					}
					else
					{
						logs("fetch row end...");
						mTask->nextTest();
						close();
						delete this;
						logs("== earyl query test ok...\n");
					}
				}
			};

			auto cmdq = new ACmdQry(Cnn);
			int err;
			int ret = cmdq->query("select name,address from userinfo", &err);
			if (ret == MDB_COMPLETE)
			{
				if (err == 0)
				{
					ret = cmdq->getRows();
					if (ret == MDB_COMPLETE)
					{
						nextTest();
						cmdq->close();
						delete cmdq;
					}
					else
					{
						logs("fetch pending...");
					}
				}
			}
			else
			{
				logs("query pending...");
			}
		}

		void testQueryPending()
		{
			logs("== start pending query test...");
			class ACmdQry: public EdMdbQuery
			{
				DbTask *mTask;
			public:
				ACmdQry(EdMdbCnn* pcnn) :
						EdMdbQuery(pcnn)
				{
					mTask = (DbTask*) getCurrentTask();
				}

				void printRow(MYSQL_ROW row)
				{
					logs("name=%s,  address=%s", row[0], row[1]);
				}

				virtual void OnQueryResult(int err)
				{
					logs("on query result, err=%d", err);
					if (err == 0)
					{
						int ret = fetchRow();
						if (ret == MDB_COMPLETE)
						{
							logs("### Fail: unexpected fetch complete");
							assert(0);
						}
					}
				}

				virtual void OnFetchRow(MYSQL_ROW row)
				{
					logs("on fetch result, row=%x", row);
					if (row != NULL)
					{
						printRow(row);
						int ret = fetchRow();
						if (ret == MDB_COMPLETE)
						{
							logs("### Fail: unexpected fetch complete");
							assert(0);
						}
					}
					else
					{
						logs("fetch row end...");
						mTask->nextTest();
						close();
						delete this;
						logs("== Pending query test ok...\n");
					}
				}
			};

			auto cmdq = new ACmdQry(Cnn);
			int ret = cmdq->query("select name,address from userinfo");
			if (ret == MDB_COMPLETE)
			{
				logs("### Fail: unexpected query pending...");
				assert(0);
			}
		} // testQueryPending

		void create_table_subtest()
		{
			logs("== Test: create table ...");
			class ACrtTbl: public EdMdbQuery
			{
				DbTask *mTask;
			public:
				ACrtTbl() :
						EdMdbQuery(NULL)
				{
					mTask = (DbTask*) getCurrentTask();
				}
				void OnQueryResult(int err)
				{
					logs("create table result, err=%d", err);
					checkResult(MDB_COMPLETE, err);
				}

				void checkResult(int ret, int err)
				{
					if (ret == MDB_COMPLETE)
					{
						if (err != 0)
						{
							logs("### Fail: create table error,");
							assert(0);
						}
						mTask->nextTest();
						delete this;
					}
					else if (ret == MDB_CONTINUE)
					{
						logs("create table pending...");
					}
				}
			};

			auto qr = new ACrtTbl;
			qr->setConnection(Cnn);
			qr->query("create table test_tbl (col1 int, col2 int)");
			//qr->checkResult(ret, err);
		}

		void IOnMdbCnnStatus(EdMdbCnn* pcnn, int result)
		{
			logs("on mdb cnn, result=%d", result);
			if (pcnn != Cnn || result != 1)
			{
				logs("### Fail: unexpected connection fail...result=%d", result);
				assert(0);
			}
			logs("normal connection test ok...\n");
			nextTest();
		}

	};
	logm(">>>> Test: Db Task, mode=%d", mode);
	fdcheck_start();
	auto task = new DbTask;
	task->run(mode);
	task->wait();
	delete task;
	fdcheck_end();
	logm("<<<< Task Db test OK\n");
}

int main()
{
	EdNioInit();
	return 0;
	init_test();
	for (int i = 0; i < 1; i++)
	{
//		testmsg(i);
		//testSocket(i);
//		testMainThreadTask(i);
//		testMultiTaskInstance(1);
//		testtimer(i);
//		testcurl(i);
//		testreservefree(i);
//		testreadclose(i);
		//testssl(i);
		//testsmartsock(i);
		//testHttpBase(i);
		testHttpSever(i);
		//testmultipartapi();
		//testMariadb(i);
	}
	return 0;
}
