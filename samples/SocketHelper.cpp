#include <windows.h>
#include <thread>
#include "SocketHelper.h"
#include "TCPSocket.h"
#include "UDPSocket.h"

CTcpSocketHandler::CTcpSocketHandler()
{
	this->SetAutoReConnect(false);
	this->SetRecvFlag(false);
	m_clientSocket = INVALID_SOCKET;
}

CTcpSocketHandler::~CTcpSocketHandler()
{

}

int CTcpSocketHandler::Init(const char *cServerAddr, int nServerPort, bool bAutoReConnect/* = false*/)
{
	this->SetAutoReConnect(bAutoReConnect);
	this->SetServerAddr(cServerAddr);
	this->SetServerPort(nServerPort);
		
	ClientInit();

	SetRecvFlag(true);

	//启动接收线程
	StartRecvDataThread();

	return 0;
}

void CTcpSocketHandler::ClientInit()
{
	ClientSockInitialize();
	ClientSetSockOptions();
	ClientSetSockAddrsIn();
	ClientExecuteConnect();
}

void CTcpSocketHandler::ClientSockInitialize()
{
	m_clientReConnectSocket = Tcp_SockInitialize();
}

void CTcpSocketHandler::ClientSetSockOptions()
{
	//默认不设置超时
	Tcp_SetSockOptions(m_clientSocket, 0, 0, TRUE);
}

void CTcpSocketHandler::ClientSetSockAddrsIn()
{
	Tcp_SetSockAddrsIn(this->m_strServerAddr.c_str(), this->m_nServerPort, this->m_serverSockAddrIn);
}
void CTcpSocketHandler::ClientExecuteConnect()
{
	m_clientSocket = Tcp_Connect(m_clientReConnectSocket, this->m_serverSockAddrIn);
}

int CTcpSocketHandler::Exit()
{
	SetRecvFlag(false);	
	SetAutoReConnect(false);
	while (this->GetReConnecting())
	{
		Sleep(1);
	}
			
	Tcp_Exit(m_clientSocket);
	m_clientSocket = INVALID_SOCKET;

	return 0;
}

void CTcpSocketHandler::RecvDataFromApp(void * p)
{
	SOCKET clientSocket = INVALID_SOCKET;
	char cResult[16384] = { 0 };
	int nResult = 0;
	CTcpSocketHandler *pWSH = (CTcpSocketHandler *)p;
	if (pWSH)
	{
		//自动重连尚未开始
		pWSH->SetReConnecting(false);
		while (pWSH->GetRecvFlag())
		{
			clientSocket = pWSH->GetClientSocket();
			if (clientSocket != INVALID_SOCKET)
			{
				memset(cResult, 0, sizeof(cResult));
				nResult = Tcp_Recv(clientSocket, cResult, sizeof(cResult));
				if (!pWSH->GetRecvFlag())
				{
					break;
				}
				if (nResult > 0)
				{
#ifdef DEBUG
					printf("Bytes received: %d-%ws\n", nResult, UTF8ToUnicode(cResult).c_str());
#endif // DEBUG
					printf("Bytes received: %d-%s\n", nResult, cResult);
				}
				else //如果nResult <= 0
				{
					//设置当前Socket为无效值
					pWSH->SetClientSocket(INVALID_SOCKET);

					//设置正在进行重连
					pWSH->SetReConnecting(true);
				}
			}
			else
			{
				//设置正在进行重连
				pWSH->SetReConnecting(true);
			}
			
			//判断是否设置了自动重连
			if (pWSH->GetAutoReConnect())
			{
				//初始化
				pWSH->ClientSockInitialize();
				//设置选项
				pWSH->ClientSetSockOptions();
				//完成自动重连事务
				while (pWSH->GetReConnecting() && pWSH->GetAutoReConnect() && pWSH->GetRecvFlag() && pWSH->GetClientSocket() == INVALID_SOCKET)
				{
					pWSH->ClientExecuteConnect();
				}
				
				std::string strCommand = "";
				pWSH->RequestCommand(strCommand.c_str(), strCommand.length());

				//自动重连已经结束
				pWSH->SetReConnecting(false);
			}
			else
			{
				break;
			}
		}
	}
}

int CTcpSocketHandler::StartRecvDataThread()
{
	std::thread(&CTcpSocketHandler::RecvDataFromApp, this).detach();

	return 0;
}

int CTcpSocketHandler::RequestCommand(const char * pCommand, int nCommandLength)
{
	return Tcp_Send(m_clientSocket, pCommand, nCommandLength);
}

//////////////////////////////////////////////////////////////////////////////////////////
SOCKET CUdpSocketHandler::udp_start(int nPort)
{
	SOCKET s = INVALID_SOCKET;

	Udp_InitSocket(s);
	Udp_BindSocket(s, nPort);

	return s;
}

DWORD WINAPI CUdpSocketHandler::udp_recvdata_thread(void *p)
{
	SOCKET s = (SOCKET)p;
	SOCKETPACKET sp = { 0 };

	while (true)
	{
		//等待并接收数据
		sp.ssize = sizeof(SOCKETPACKET);
		Udp_RecvData(s, &sp.si, (unsigned char *)&sp, sp.ssize);
		if (sp.size)
		{
			Udp_RecvData(s, &sp.si, (unsigned char *)&sp.data, sp.size);
		}
	}

	Udp_ExitSocket(s);

	return 0;
}

DWORD WINAPI CUdpSocketHandler::udp_senddata_thread(void *p)
{
	SOCKETPACKET * psp = (SOCKETPACKET *)p;
	psp->ssize = sizeof(SOCKETPACKET);
	Udp_SendData(psp->s, &psp->si, (unsigned char *)psp, psp->ssize);
	if (psp->size)
	{
		Udp_RecvData(psp->s, &psp->si, (unsigned char *)psp->data, psp->size);
	}

	return 0;
}