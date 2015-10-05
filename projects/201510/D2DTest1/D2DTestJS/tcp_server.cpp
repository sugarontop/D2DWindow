#include "stdafx.h"
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <strstream>
#include "DateTime.h"

#define PORT_NUMBER 9888

static DWORD CALLBACK MyWebServerEx(LPVOID p);
static int  kitcut_recv(SOCKET s, std::stringstream& sm, int& error_code);
static HANDLE Tcpthead;

std::function<void(LPCWSTR)> f1a;

/*
 sockaddr_in はv4だけ、古いので使わない

*/
 
void SRV_StartServer(std::function<void(LPCWSTR)> f1)
{
	f1a = f1;
	WSADATA data;
	int r = WSAStartup(MAKEWORD(2, 2), &data);

	DWORD dw = 0;
	Tcpthead = ::CreateThread(NULL, 0, MyWebServerEx, 0, 0, &dw);
}

void SRV_StopServer()
{
	if (Tcpthead )
		::TerminateThread(Tcpthead,0);
	WSACleanup();
}

void AppError( LPCWSTR msg, DWORD err_no )
{
	
}

DWORD CALLBACK MyWebServerEx(LPVOID p)
{
	//Server& sever = *(Server*) p;

	addrinfo hints;
	addrinfo* res=nullptr;
	char* service = "9888";

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6; //AF_UNSPEC; // V4,V6 dual
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;   
	
	int err = getaddrinfo("localhost", service, &hints, &res);

	SOCKET srvsock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (srvsock < 0)
	{
		AppError(L"socket error", GetLastError());
		return -1;
	}

	
	int result = bind(srvsock, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);

	if (result < 0)
	{
		AppError(L"bind error", GetLastError());
		closesocket(srvsock);
		return -1;
	}

	result = listen(srvsock, 1);

	if (result < 0)
	{
		AppError(L"listen error", GetLastError());
		closesocket(srvsock);
		return -1;
	}

	
	SOCKET client_sock;

	struct sockaddr_storage sa; 
	socklen_t salen = sizeof(sa); 


	while (0 <= (client_sock = accept(srvsock, (sockaddr*) &sa, &salen)))
	{
		int rc;
		int err;
		std::stringstream sm;
		if ((rc = kitcut_recv(client_sock, sm, err)) > 0)
		{			
			V4::FString ss; 
			
			std::string x = sm.rdbuf()->str();
			ss.FromUtf8( x.c_str() );


			f1a( ss.c_str());


			
			V4::DateTime dt;
			auto date = dt.GNow().Format( V4::DateTime::TYP::ISO8601);

			ss = V4::FString::Format( L"success %s\n", (LPCWSTR)date );

			int len = 0;
			char* send_data = ss.ToUtf8(&len);
					
			send(client_sock, send_data, len, 0 );
						
			delete [] send_data;

			closesocket(client_sock);
		}

	}


	shutdown(srvsock, SD_BOTH);
	closesocket(srvsock);
	Tcpthead = 0;
	return 0;
}
int  kitcut_recv(SOCKET s, std::stringstream& sm, int& error_code)
{
	char buf[256];
	int r, length = 0;

	u_long val = 1;
	ioctlsocket(s, FIONBIO, &val);
		
	error_code = 0;
	while (true)
	{
		r = recv(s, buf, sizeof(buf), 0);
		if (r > 0)
		{
			sm.write(buf, r);
			length += r;

			//ATLTRACE( "kitcut_recv add=%d len=%d\n", r, length );
		}
		else if (r == SOCKET_ERROR)
		{
			error_code = WSAGetLastError();
			if (WSAEWOULDBLOCK == error_code)
			{
				// recv buffer is empty.
				error_code = 0;

				//ATLTRACE( "kitcut_recv end len=%d\n", length );
			}
			else
				length = SOCKET_ERROR;

			break;
		}
		else if (r == 0)
		{
			// client shutdown!
			error_code = -1;
			length = 0;
			break;
		}
	}

	return length;
}