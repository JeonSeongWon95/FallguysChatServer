#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")


int main()
{
	WSAData wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	SOCKET Server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in ServerAddress;
	memset(&ServerAddress, 0, sizeof(ServerAddress));
	ServerAddress.sin_port = htons(7777);
	ServerAddress.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	ServerAddress.sin_family = AF_INET;

	bind(Server, (sockaddr*)&ServerAddress, sizeof(ServerAddress));
	listen(Server, 5);

	fd_set ReadSockets;
	FD_ZERO(&ReadSockets);
	FD_SET(Server, &ReadSockets);

	fd_set CopySockets;
	FD_ZERO(&CopySockets);

	timeval time;
	time.tv_sec = 0;
	time.tv_usec = 10;

	while(true)
	{
		CopySockets = ReadSockets;
		int ChangeSocketCount = select(CopySockets.fd_count, &CopySockets, 0, 0, &time);

		if (ChangeSocketCount == 0)
			continue;
		if (ChangeSocketCount < 0)
			break;

		for(int i = 0; i < ChangeSocketCount; ++i)
		{
			if(FD_ISSET(ReadSockets.fd_array[i], &CopySockets))
			{
				if(ReadSockets.fd_array[i] == Server)
				{
					sockaddr_in ClientAddress;
					memset(&ClientAddress, 0, sizeof(ClientAddress));
					int ClientAddressSize = sizeof(ClientAddress);
					SOCKET ClientServer = accept(Server, (sockaddr*)&ClientAddress, &ClientAddressSize);
					FD_SET(ClientServer, &ReadSockets);

					std::cout << "Client Connect!" << std::endl;
				}
				else
				{
					byte Buffer[1024] = {};
					int recvCount = recv(ReadSockets.fd_array[i], (char*)Buffer, sizeof(Buffer), 0);

					if (recvCount <= 0)
					{
						SOCKET DisconnectedSocket = ReadSockets.fd_array[i];
						FD_CLR(DisconnectedSocket, &ReadSockets);
						closesocket(DisconnectedSocket);
					}
					else
					{
						for(int j = 0; j < static_cast<int>(ReadSockets.fd_count); ++j)
						{
							if (ReadSockets.fd_array[j] == Server)
								continue;

							int SendCount = send(ReadSockets.fd_array[j], (char*)Buffer, recvCount, 0);
						}

					}

				}
			}
			

		}

	}


	WSACleanup();
}