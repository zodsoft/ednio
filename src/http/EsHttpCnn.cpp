/*
 * EsHttpCnn.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: netmind
 */

#define DBGTAG "htcnn"
#define DBG_LEVEL DBG_VERBOSE
#include <stack>
#include <unordered_map>
#include "../edslog.h"
#include "EsHttpCnn.h"
#include "EsHttpTask.h"
#include "http_parser.h"
#include "../EsFile.h"
#include "EsHttpMsg.h"
#include "EdHttp.h"
#include "EsHttpBodyStream.h"

namespace edft
{
EsHttpCnn::EsHttpCnn()
{
	dbgd("http cnn const......");

	_rn = rand();
	_hidx = 0;

	mCurHdrName = mCurHdrVal = NULL;

	memset(&mParserSettings, 0, sizeof(mParserSettings));
	mParserSettings.on_message_begin = msg_begin;
	mParserSettings.on_message_complete = msg_end;
	mParserSettings.on_url = on_url;
	mParserSettings.on_status = on_status;
	mParserSettings.on_header_field = head_field_cb;
	mParserSettings.on_header_value = head_val_cb;
	mParserSettings.on_headers_complete = on_headers_complete;
	mParserSettings.on_body = body_cb;

	memset(&mParser, 0, sizeof(mParser));

	mBufSize = 8 * 1024;
	mReadBuf = (char*) malloc(mBufSize);

	mCurCtrl = NULL;
}

EsHttpCnn::~EsHttpCnn()
{
	free(mReadBuf);
}

#if 0
void EsHttpCnn::OnRead()
{
	dbgd("on read");
	char buf[8*1024];
	int rdcnt = recv(buf, sizeof(buf));
	if(rdcnt>0)
	{
		buf[rdcnt] = 0;
		dbgd("   read str=%s", buf);
	}
}

void EsHttpCnn::OnDisconnected()
{
	dbgd("on disco...");
}
#endif

void EsHttpCnn::initCnn(int fd, u32 handle, EsHttpTask *ptask, int socket_mode)
{
	mSock.setOnNetListener(this);
	mSock.socketOpenChild(fd, socket_mode);

	mTask = ptask;
	mHandle = handle;

	http_parser_init(&mParser, HTTP_REQUEST);
	mParser.data = this;
}

void EsHttpCnn::procRead()
{
#if 1
	int rdcnt = mSock.recvPacket(mReadBuf, mBufSize);
	dbgv("proc read cnt=%d", rdcnt);
	if(rdcnt>0)
	{
		http_parser_execute(&mParser, &mParserSettings, mReadBuf, rdcnt);
	}
#else
	int rdcnt = recv(mReadBuf, mBufSize);
	dbgv("proc read cnt=%d", rdcnt);
	if (rdcnt > 0)
	{
		//mReadBuf[rdcnt] = 0;
		//dbgd("proc read str=%s", mReadBuf);
//		EsFile file;
//		file.openFile("p.dat", EsFile::OPEN_RWC);
//		file.writeFile(mReadBuf, rdcnt);
//		file.closeFile();
		http_parser_execute(&mParser, &mParserSettings, mReadBuf, rdcnt);
	}
#endif
}

void EsHttpCnn::procDisconnected()
{
#if 1
	mSock.socketClose();
#else
	close();
	deregisterEvent();
#endif
}

int EsHttpCnn::head_field_cb(http_parser* parser, const char *at, size_t length)
{
	EsHttpCnn* pcnn = (EsHttpCnn*) parser->data;
	return pcnn->headerNameCb(parser, at, length);
//

}

int EsHttpCnn::head_val_cb(http_parser* parser, const char *at, size_t length)
{
	EsHttpCnn *pcnn = (EsHttpCnn*) parser->data;
	return pcnn->headerValCb(parser, at, length);

}

int EsHttpCnn::body_cb(http_parser* parser, const char *at, size_t length)
{
	EsHttpCnn *pcnn = (EsHttpCnn*) parser->data;
	return pcnn->bodyDataCb(parser, at, length);
}

int EsHttpCnn::msg_begin(http_parser* parser)
{
	EsHttpCnn *pcnn = (EsHttpCnn*) parser->data;
	return pcnn->dgMsgBeginCb(parser);

}

int EsHttpCnn::msg_end(http_parser* parser)
{
	EsHttpCnn *pcnn = (EsHttpCnn*) parser->data;
	return pcnn->dgMsgEndCb(parser);
}

int EsHttpCnn::on_url(http_parser* parser, const char* at, size_t length)
{
	EsHttpCnn *pcnn = (EsHttpCnn*) parser->data;
	return pcnn->dgUrlCb(parser, at, length);
}

int EsHttpCnn::on_headers_complete(http_parser* parser)
{
	EsHttpCnn *pcnn = (EsHttpCnn*) parser->data;
	return pcnn->dgHeaderComp(parser);

}

int EsHttpCnn::headerNameCb(http_parser*, const char* at, size_t length)
{
	string tmp(at, length);
	dbgv("name cb, %s", tmp.c_str());
	if (mPs == PS_FIRST_LINE)
	{
		procReqLine();
	}

	if (mIsHdrVal)
	{
		dbgd("header set, name=%s, val=%s", mCurHdrName->c_str(), mCurHdrVal->c_str());
		procHeader();
		mIsHdrVal = false;
	}
	if (mCurHdrName == NULL)
		mCurHdrName = new string();
	mCurHdrName->append(at, length);
	return 0;
}

int EsHttpCnn::headerValCb(http_parser*, const char* at, size_t length)
{
	string tmp(at, length);
	dbgv("val cb, %s", tmp.c_str());

	if (mCurHdrVal == NULL)
		mCurHdrVal = new string();

	mCurHdrVal->append(at, length);
	mIsHdrVal = true;
	return 0;

}

int EsHttpCnn::dgHeaderComp(http_parser* parser)
{
	if (mIsHdrVal)
	{
		dbgd("header comp, name=%s, val=%s", mCurHdrName->c_str(), mCurHdrVal->c_str());
		procHeader();
		mIsHdrVal = false;
	}

	if(mCurCtrl != NULL)
		mCurCtrl->OnRequest();
/*
	IUriControllerCb *cb = mTask->getController(mCurUrl);
	if (cb)
	{
		mCurTrans->mController = cb;
		cb->IOnNewHttpTrans(mHandle, mCurTrans->mHandle, mCurTrans);
		if (mCurTrans->mIsResponsed)
		{
			mCurTrans->encodeResp();
			//transmitReserved();
		}
	}
	*/
	return 0;
}

int EsHttpCnn::bodyDataCb(http_parser*, const char* at, size_t length)
{
	dbgd("body data len=%d", length);

	return 0;
}

int EsHttpCnn::dgMsgBeginCb(http_parser* parser)
{

	mPs = PS_FIRST_LINE;

	mIsHdrVal = false;
	mCurHdrName = NULL;
	mCurHdrVal = NULL;

	//mCurTrans = new EsHttpTrans(mTrhseed, this);
	//mTransMap.push_back(mCurTrans);
	//_hidx++;


	return 0;
}

int EsHttpCnn::dgMsgEndCb(http_parser*)
{

	return 0;
}

int EsHttpCnn::dgUrlCb(http_parser* parser, const char* at, size_t length)
{
	if (mCurUrl == NULL)
		mCurUrl = new string(at, length);
	else
		mCurUrl->append(at, length);

	return 0;
}

int EsHttpCnn::on_status(http_parser* parser, const char* at, size_t length)
{
	return 0;
}

int EsHttpCnn::statusCb(http_parser* parser, const char* at, size_t length)
{
	return 0;
}

void EsHttpCnn::procHeader()
{
	if(mCurCtrl != NULL)
		mCurCtrl->addReqHeader(mCurHdrName, mCurHdrVal);


	delete mCurHdrName;
	delete mCurHdrVal;

	mCurHdrName = mCurHdrVal = NULL;
}

void EsHttpCnn::procReqLine()
{
	dbgd("url = %s", mCurUrl->c_str());
	mPs = PS_HEADER;
	//EdHttpController* pctl = mTask->OnNewRequest(http_method_str((http_method)mParser.method), mCurUrl->c_str());
	mCurCtrl = mTask->getRegController(mCurUrl->c_str());
	if(mCurCtrl != NULL) {
		mCurCtrl->initCtrl(this);
		mCtrlList.push_back(mCurCtrl);
	}
}

#if 0
bool EsHttpCnn::transmitResponse(EsHttpTrans* ptrans)
{
	int len;
	EsHttpMsg *resp = &ptrans->mRespMsg;
	EsHttpBodyStream *body = ptrans->mBodyStream;
	char tmp[100];

	// status line
	string firstline = string("HTTP/1.1 ") + ptrans->mStatusCode + " " + es_get_http_desp(ptrans->mStatusCode) + "\r\n";
	resp->setStatusLine(&firstline);

	// Date header
	es_get_httpDate(tmp);
	resp->addHdr(HTTPHDR_DATE, tmp);
	resp->addHdr(HTTPHDR_SERVER, "EDNIO/0.2.0");
	if (body != NULL)
	{
		char tmp[100];
		sprintf(tmp, "%d", body->getContentLen());
		resp->addHdr(HTTPHDR_CONTENT_LEN, tmp);
		resp->addHdr(HTTPHDR_CONTENT_TYPE, body->getContentType());
	}

	string outbuf;
	resp->encodeRespMsg(&outbuf);

	if (body)
	{
		body->open();
		char* ptxt = (char*) body->getBuffer();
		outbuf.append(ptxt, body->getContentLen());
		body->close();
	}
	send(outbuf.c_str(), outbuf.size());

}
#endif


void EsHttpCnn::scheduleTransmit()
{
	dbgd("scheduling transmit..., ready ctrl cnt=%d", mCtrlList.size());
	if (mCurSendCtrl != NULL)
		return;

	if (mCtrlList.size() == 0)
	{
		dbge("### unexpected state, there is no ctrl");
		return;
	}

	mCurSendCtrl = mCtrlList.front();

	stack<list<EdHttpController*>::iterator> dellist;

	int sr;
	auto itr = mCtrlList.begin();
	for (; itr != mCtrlList.end(); itr++)
	{
		mCurSendCtrl = (*itr);
		sr = sendCtrlStream(mCurSendCtrl, 16*1024);
		if(sr == SEND_OK) {
			dellist.push(itr);
			mCurSendCtrl->OnContentSendComplete();
			mTask->freeController(mCurSendCtrl);
			mCurSendCtrl = NULL;
		}
		else {
			break;
		}
	}


	dbgd("to remove cnt=%d", dellist.size());
	for (; !dellist.empty();)
	{
		auto itr = dellist.top();
		dellist.pop();
		mCtrlList.erase(itr);
	}

	dbgd("ctrl list cnt=%d", mCtrlList.size());
}



void EsHttpCnn::IOnNet(EdSmartSocket* psock, int event)
{
	if(event == NETEV_READ) {
		procRead();
	} else if(event == NETEV_SENDCOMPLETE) {
		mCtrlList.pop_front();
		mTask->freeController(mCurSendCtrl);
		mCurSendCtrl = NULL;
		scheduleTransmit();
	} else if (event == NETEV_DISCONNECTED) {
		procDisconnected();
	}
}


int EsHttpCnn::sendCtrlStream(EdHttpController* pctl, int maxlen)
{
	int retVal=SEND_FAIL;
	packet_buf_t bf;
	if(mSock.isWritable() == false)
		return SEND_FAIL;

	for(;;) {
		bf = pctl->getSendPacket();

		if(bf.len >0) {
			retVal = mSock.sendPacket(bf.buf, bf.len);

			free(bf.buf);
			if(retVal != SEND_OK)
				break;
		}
		else
		{
			retVal = SEND_OK;
			break;
		}
	}

	return retVal;
}

} // namespace edft
