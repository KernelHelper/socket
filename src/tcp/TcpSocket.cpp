﻿// TcpSocket.cpp : Defines the entry point for the console application.
//
#include "TcpSocket.h"

SOCKET Tcp_Init(const _TCHAR * ptServerAddr, int nServerPort, DWORD dwSendTimeOut/* = DEFAULT_SEND_TIMEOUT*/,
	DWORD dwRecvTimeOut/* = DEFAULT_RECV_TIMEOUT*/, BOOL bLinger/* = FALSE*/)
{
    int iResult = 0;
	WSADATA wsaData = {0};
	SOCKET connectSocket = INVALID_SOCKET;
	//struct addrinfo *result = NULL,
	//	*ptr = NULL,
	//	hints;
	sockaddr_in serverAddr = { 0 };
	USES_CONVERSION;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		//LOG_DEBUG_PRINT(_T("WSAStartup failed with error: %d\n"), iResult);
		return INVALID_SOCKET;
	}

	/*ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	// Resolve the server address and port
	iResult = getaddrinfo(strServerAddr, cPort, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return INVALID_SOCKET;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return INVALID_SOCKET;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);
	*/
	//创建套接字
	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
	{
		//LOG_DEBUG_PRINT(_T("socket error !"));
		return INVALID_SOCKET;
	}

	if (dwSendTimeOut > 0)
	{
		iResult = setsockopt(connectSocket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&dwSendTimeOut, sizeof(dwSendTimeOut));
	}

	if (dwRecvTimeOut > 0)
	{
		iResult = setsockopt(connectSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&dwRecvTimeOut, sizeof(dwRecvTimeOut));
	}

	if (bLinger)
	{
		struct linger linger = { 1, 0 };//避免TIME_WAIT出现
		iResult = setsockopt(connectSocket, SOL_SOCKET, SO_LINGER, (const char *)&linger, sizeof(linger));
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(nServerPort);
	serverAddr.sin_addr.S_un.S_addr = inet_addr(T2A(ptServerAddr));
	if (connect(connectSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		//LOG_DEBUG_PRINT(_T("connect error !"));
		Tcp_Exit(connectSocket);
		return INVALID_SOCKET;
	}
	if (connectSocket == INVALID_SOCKET) {
		//LOG_DEBUG_PRINT(_T("Unable to connect to server!\n"));
		Tcp_Exit(connectSocket);
		return INVALID_SOCKET;
	}

#ifdef DEBUG
	printf("Connected to %s!\n", ptServerAddr);
#endif // DEBUG

	return connectSocket;
}

SOCKET Tcp_SockInitialize()
{
	WSADATA wsaData;
	SOCKET connectSocket = INVALID_SOCKET;

	int iResult = 0;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
    {
		//LOG_DEBUG_PRINT(_T("WSAStartup failed with error: %d\n"), iResult);
		return INVALID_SOCKET;
	}

	//创建套接字
	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
	{
		//LOG_DEBUG_PRINT(_T("socket error !"));
		return INVALID_SOCKET;
	}

	return connectSocket;
}

int Tcp_SetSockOptions(SOCKET connectSocket, DWORD dwSendTimeOut/* = DEFAULT_SEND_TIMEOUT*/,
					DWORD dwRecvTimeOut/* = DEFAULT_RECV_TIMEOUT*/, BOOL bLinger/* = FALSE*/)
{
	int iResult = 0;

	if (dwSendTimeOut > 0)
	{
		iResult = setsockopt(connectSocket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&dwSendTimeOut, sizeof(dwSendTimeOut));
	}

	if (dwRecvTimeOut > 0)
	{
		iResult = setsockopt(connectSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&dwRecvTimeOut, sizeof(dwRecvTimeOut));
	}

	if (bLinger)
	{
		struct linger linger = { 1, 0 };//避免TIME_WAIT出现
		iResult = setsockopt(connectSocket, SOL_SOCKET, SO_LINGER, (const char *)&linger, sizeof(linger));
	}

	return connectSocket;
}

int Tcp_SetSockAddrsIn(const char * ptServerAddr, int nServerPort, struct sockaddr_in & serverAddrIn)
{
	USES_CONVERSION;
	serverAddrIn.sin_family = AF_INET;
	serverAddrIn.sin_port = htons(nServerPort);
	serverAddrIn.sin_addr.S_un.S_addr = inet_addr(T2A(ptServerAddr));
	return 0;
}

SOCKET Tcp_Connect(SOCKET connectSocket, struct sockaddr_in & serverAddrIn)
{
	SOCKET connectedSocket = connectSocket;
	if (connect(connectedSocket, (sockaddr *)&serverAddrIn, sizeof(serverAddrIn)) == SOCKET_ERROR)
	{
		//LOG_DEBUG_PRINT(_T("connect error !"));
		return INVALID_SOCKET;
	}
	if (connectedSocket == INVALID_SOCKET) {
		//LOG_DEBUG_PRINT(_T("Unable to connect to server!\n"));
		return INVALID_SOCKET;
	}

#ifdef DEBUG
	printf("Connected!\n");
#endif // DEBUG

	return connectedSocket;
}

int Tcp_Read(SOCKET s, void * data, int size)
{
    int n_left = 0;
    int n_read = 0;
    char * ptr = 0;

    n_left = size;
    ptr = (char *)data;

    while(n_left > 0)
    {
        n_read = read(s, ptr, n_left);
        if(n_read < 0)
        {
            if(errno != EINTR)
            {
                return (-1);
            }
            else
            {
                n_read = 0;
            }
        }
        else if(n_read == 0)
        {
            break;
        }
        n_left -= n_read;
        ptr += n_read;
    }
    return (size - n_left);
}

int Tcp_Write(SOCKET s, const void * data, int size)
{
    int n_left = 0;
    int n_written = 0;
    char * ptr = 0;

    n_left = size;
    ptr = (char *)data;

    while(n_left > 0)
    {
         // 开始写
         n_written = write(s, ptr, n_left);
         if(n_written <= 0) // 出错了
         {
            if(errno == EINTR) // 中断错误 我们继续写
            {
                n_written = 0;
            }
            else             // 其他错误 没有办法,只好撤退了
            {
                return(-1);
            }
         }
         n_left -= n_written;
         ptr += n_written;   // 从剩下的地方继续写
    }

    return (0);
}

int Tcp_Send(SOCKET s, const char * data, int size)
{
	int iResult = 0;

	// Send an initial buffer
	iResult = send(s, data, size, 0);
	if (iResult == SOCKET_ERROR)
    {
		//LOG_DEBUG_PRINT(_T("send failed with error: %d\n"), WSAGetLastError());
	}
	else
    {
        //LOG_DEBUG_PRINT(_T("Bytes sent: %d\n"), iResult);
    }

#ifdef DEBUG
	printf("Bytes received: %d\n", iResult);
#endif // DEBUG

	return iResult;
}


int Tcp_Recv(SOCKET s, char * data, int size)
{
	int iResult = 0;

    iResult = recv(s, data, size, 0);
    if (iResult > 0)
    {
        //LOG_DEBUG_PRINT(_T("Bytes received: %d\n"), iResult);
    }
    else if (iResult == 0)
    {
        //LOG_DEBUG_PRINT(_T("Connection closed with error: %d\n"), WSAGetLastError());
    }
    else
    {
        //LOG_DEBUG_PRINT(_T("recv failed with error: %d\n"), WSAGetLastError());
    }

	return iResult;
}

void Tcp_Exit(SOCKET s)
{
	int iResult = 0;

	if (s != INVALID_SOCKET)
	{
		// shutdown the connection since no more data will be sent
		iResult = shutdown(s, SD_BOTH);

		// cleanup
		closesocket(s);
	}

	WSACleanup();
}
