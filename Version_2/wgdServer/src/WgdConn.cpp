#include "WgdConn.h"
#include <stdio.h>
#include <io.h>
#include "BaseSocket.h"
#include "WgdServer.h"
#include "CommonDef.h"
#include "CommonNet.h"

CWGDConn::CWGDConn(CWgdServer* pNotify):m_pServer(pNotify)
{
	m_pBaseSocket = NULL;
}

CWGDConn::~CWGDConn()
{

}

void CWGDConn::OnClose() 
{
	net_handle_t fd = m_pBaseSocket->GetSocket();

	m_pBaseSocket->Close();

	m_pServer->RemoveWgdConn(fd);
}

int CWGDConn::OnRead()
{
	u_long avail = 0;
	int nRet = ioctlsocket(m_pBaseSocket->GetSocket(), FIONREAD, &avail);
	if ( (nRet == SOCKET_ERROR) || (avail == 0) )
	{
		OnClose();
		return -1;
	}

	char szRcvBuf[8192] = { 0 };

	int READ_BUF_SIZE = 8192;

	for (;;)
	{
		uint32_t free_buf_len = m_in_buf.GetAllocSize() - m_in_buf.GetWriteOffset();
		if (free_buf_len < READ_BUF_SIZE)
			m_in_buf.Extend(READ_BUF_SIZE);

		int nRet = m_pBaseSocket->Recv(m_in_buf.GetBuffer() + m_in_buf.GetWriteOffset(), READ_BUF_SIZE);
		if (SOCKET_ERROR == nRet)
		{
			if ((WSAGetLastError() == WSAEINTR))
				continue;
			else
				break;
		}

		m_in_buf.IncWriteOffset(nRet);
	}

	//这里一定要循环把所有的消息都处理掉 因为一次接受的数据可能包含了多个消息结构体
	while (ParseBuffer(m_in_buf) != -1)
	{

	}

	return 0;
}

int32_t CWGDConn::ParseBuffer(CSimpleBuffer& simpleBuffer)
{
	uint32_t uintHave = simpleBuffer.GetWriteOffset();
	int32_t nHeadSize = sizeof(WGDHEAD);

	if (uintHave < nHeadSize)
	{
		return -1;
	}

	uchar_t* pBuffer = simpleBuffer.GetBuffer();
	WGDHEAD* pHead = (WGDHEAD*)pBuffer;
	int32_t nTotalSize = nHeadSize + pHead->nDataLen;
	if (uintHave < nTotalSize)
	{
		return -1;
	}

	return DealBuffer(simpleBuffer);
}

int32_t CWGDConn::DealBuffer(CSimpleBuffer & simpleBuffer)
{
	uchar_t* pBuffer = simpleBuffer.GetBuffer();
	WGDHEAD* pHead = (WGDHEAD*)pBuffer;
	int32_t nTotalSize = sizeof(WGDHEAD) + pHead->nDataLen;

	//m_pBaseSocket->m_pBaseServer->OnStatisticEnd();

	m_pServer->OnReceivedNotify(m_pBaseSocket->GetSocket(), pBuffer, nTotalSize);

	simpleBuffer.Read(NULL, nTotalSize);

	return 0;

#if 0
	uchar_t* pBuffer = simpleBuffer.GetBuffer();
	WGDHEAD* pHead = (WGDHEAD*)pBuffer;

	char* name = pHead->szFileName;

	switch (pHead->type)
	{
	case eNet_Upload:
		{
			std::string strFilePath = RECEIVE_DIR + std::string(pHead->szFileName);
			int nHeadLen = pHead->nDataLen;
			m_in_buf.Read(NULL, sizeof(WGDHEAD));
			int32_t nWrite = WriteFile(strFilePath, m_in_buf.GetBuffer(), nHeadLen);
			if (nWrite > 0 && nWrite == nHeadLen)
			{
				m_in_buf.Read(NULL, nWrite);
			}

			return 0;
		}
		break;
	case eNet_Download:
		{
			std::string strFilePath = RECEIVE_DIR + std::string(name);
			m_in_buf.Read(NULL, sizeof(WGDHEAD));

			CSimpleBuffer tmpBuffer;

			WGDHEAD head;
			head.type = eNet_Download;
			//strncpy(head.szFileName, pHead->szFileName, sizeof(head.szFileName));

			tmpBuffer.Write((void*)&head, sizeof(WGDHEAD));

			int32_t nRead = ReadFile(strFilePath, tmpBuffer);

			WGDHEAD* pHead = (WGDHEAD*)tmpBuffer.GetBuffer();
			pHead->nDataLen = nRead;

			Send(tmpBuffer.GetBuffer(), tmpBuffer.GetWriteOffset());
		}
		break;
	case eNet_GetDirectory:
		{

		}
		break;
	default:
		break;
	}

	return int32_t(0);
#endif
}

void CWGDConn::OnWrite()
{
	int error = 0;
	socklen_t len = sizeof(error);
	getsockopt(m_pBaseSocket->GetSocket(), SOL_SOCKET, SO_ERROR, (char*)&error, &len);
	if (error) 
	{
		OnClose();
		return ;
	}

	while (m_out_buf.GetWriteOffset() > 0) 
	{
		int send_size = m_out_buf.GetWriteOffset();
		if (send_size > NETLIB_MAX_SOCKET_BUF_SIZE) 
		{
			send_size = NETLIB_MAX_SOCKET_BUF_SIZE;
		}

		int ret = m_pBaseSocket->Send(m_out_buf.GetBuffer(), send_size);
		if (ret <= 0)
		{
			ret = 0;
			break;
		}

		m_out_buf.Read(NULL, ret);
	}
}

int CWGDConn::Send(void* data, int len)
{
	int offset = 0;
	int remain = len;

	while (remain > 0)
	{
		int send_size = remain;
		if (send_size > 8192)
			send_size = 8192;

		int nSend = m_pBaseSocket->Send((char*)data + offset, send_size);
		if (nSend <= 0)
		{
			if ((WSAGetLastError() == WSAEINPROGRESS) || (WSAGetLastError() == WSAEWOULDBLOCK))
			{
				continue;
			}

			break;
		}

		offset += nSend;
		remain -= nSend;
	}

	return 0;
}