#include "WgdConn.h"
#include <stdio.h>
#include <io.h>
#include "BaseSocket.h"
#include "WgdServer.h"

CWGDConn::CWGDConn()
{
	m_busy = false;
}

CWGDConn::~CWGDConn()
{

}

void CWGDConn::OnConnect(net_handle_t handle, CBaseSocket* pBaseSocket)
{
	m_pBaseSocket = pBaseSocket;
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

	m_pBaseSocket->m_pBaseServer->OnStatisticEnd();

	printf("Have Receive Data Cmd: %d\n", pHead->nCmd);

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

int32_t CWGDConn::WriteFile(const std::string & strFilePath, void * data, uint32_t len)
{
#if 0
	FILE* pRecvFile = fopen(strFilePath.c_str(), "wb");
	if (NULL == pRecvFile)
	{
		return -1;
	}

	int32_t nRemain = len;
	uint32_t nHaveWrited = 0;

	uint32_t nTrueWrite = MAX_SIZE_WRITE_FILE_ONCE;

	while (nRemain > 0)
	{
		if (nRemain < nTrueWrite)
			nTrueWrite = nRemain;
		
		uchar_t* pWrite = (uchar_t*)data + nHaveWrited;
		uint32_t nWrite = fwrite(pWrite, 1, nTrueWrite, pRecvFile);
		if (nWrite == 0)
		{
			break;
		}

		nHaveWrited += nWrite;
		nRemain -= nWrite;
	}

	fclose(pRecvFile);

// 	WGDHEAD wgdHead;
// 	strncpy(wgdHead.szFileName, "Send Success!", sizeof(wgdHead.szFileName));
// 	wgdHead.nDataLen = 0;
// 
// 	netlib_send_inner((net_handle_t)handle, &wgdHead, sizeof(WGDHEAD));

	return int32_t(nHaveWrited);
#endif

	return 0;
}

int32_t CWGDConn::ReadFile(const std::string & strFilePath, CSimpleBuffer & buffer)
{
	const char* pPath = strFilePath.c_str();

	FILE* pFile = fopen(pPath, "rb");
	if (NULL == pFile)
	{
		return -1;
	}

	int size = filelength(fileno(pFile));

	char* pBuffer = new char[size];
	memset(pBuffer, 0x0, size);
	if (fread(pBuffer, 1, size, pFile) == size)
	{
		buffer.Write(pBuffer, size);
		delete[] pBuffer;
		fclose(pFile);

		return size;
	}
	
	delete[] pBuffer;
	fclose(pFile);

	return int32_t(-1);
}

void CWGDConn::OnRead()
{
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

	ParseBuffer(m_in_buf);
}

void CWGDConn::OnWrite()
{
	if (!m_busy)
		return;

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

	if (m_out_buf.GetWriteOffset() == 0) 
	{
		m_busy = false;
	}
}

int CWGDConn::Send(void* data, int len)
{
#if 0
	int offset = 0;
	int remain = len;

	while (remain > 0)
	{
		int send_size = remain;
		if (send_size > 8192)
			send_size = 8192;

		int nSend = send((char*)data + offset, send_size, 0);
		if (nSend <= 0)
		{
			if (SOCKET_ERROR == nSend && WSAGetLastError() == WSAEWOULDBLOCK)
			{
				continue;
			}

			break;
		}

		offset += nSend;
		remain -= nSend;
	}
#endif

	return 0;

// 	if (m_busy)
// 	{
// 		m_out_buf.Write(data, len);
// 		return len;
// 	}
// 
// 	int offset = 0;
// 	int remain = len;
// 	while (remain > 0) {
// 		int send_size = remain;
// 		if (send_size > NETLIB_MAX_SOCKET_BUF_SIZE) {
// 			send_size = NETLIB_MAX_SOCKET_BUF_SIZE;
// 		}
// 
// 		int ret = netlib_send_inner(m_handle, (char*)data + offset, send_size);
// 		if (ret <= 0) {
// 			ret = 0;
// 			break;
// 		}
// 
// 		offset += ret;
// 		remain -= ret;
// 	}
// 
// 	if (remain > 0)
// 	{
// 		m_out_buf.Write((char*)data + offset, remain);
// 		m_busy = true;
// 	}

	//return 0;
}