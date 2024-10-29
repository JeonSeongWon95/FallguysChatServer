c++을 이용하여 구현한 WinSocket Server입니다.
TCP/IP, Unity의 Project와 연동하여 동작하는 Chat Server입니다.

[플레이영상](https://youtu.be/Mt5rhxtEc5g?si=_HRwCH-_m2oFiuGm, "Youtube") 에서 보실 수 있습니다.

### Winsocket ChatServer

```cpp
	ServerAddress.sin_port = htons(7777);
	ServerAddress.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
```
IP address는 루프백 주소를 사용하였으며, Port는 7777을 이용하였습니다.

```cpp
	fd_set ReadSockets;
	FD_ZERO(&ReadSockets);
	FD_SET(Server, &ReadSockets);

	fd_set CopySockets;
	FD_ZERO(&CopySockets);

	timeval time;
	time.tv_sec = 0;
	time.tv_usec = 10;
```
파일 디스크립션의 집합체인 ReadSockets를 선언하고, 초기화한 다음 Server(SOCKET 구조체)를 ReadSockets 안에 넣었습니다.
Select 함수를 통해 변경된 Socket을 반환하는 경우 인자로 전달한 파일 디스크립션의 집합체를 수정하므로 원본이 아닌 복제본(CopySockets)도 선언하였습니다.

검사할 시간은 10마이크로(0.00001)초로 설정하였습니다.


```cpp
		CopySockets = ReadSockets;
		int ChangeSocketCount = select(CopySockets.fd_count, &CopySockets, 0, 0, &time);

		if (ChangeSocketCount == 0)
			continue;
		if (ChangeSocketCount < 0)
			break;
```
반복문 안에서는 우선 ReadSockets을 복사본(copysockets)에 복사하고 Select 함수를 통해 변경된 값이 있는지 체크하였습니다.
0 이거나(변경 없음), 0보다 작은 경우(Error)는 반복문이 넘어가거나 종료하도록 예외처리하였습니다.

```cpp
for(int i = 0; i < ReadSockets.fd_count; ++i)
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
			std::cout << "Socket Count : " << ReadSockets.fd_count << std::endl;
		}
		else
		{
			byte Buffer[1024] = {};
			int recvCount = recv(ReadSockets.fd_array[i], (char*)Buffer, sizeof(Buffer), 0);

			if (recvCount <= 0)
			{
				std::cout << "Recv Error" << std::endl;
				SOCKET DisconnectedSocket = ReadSockets.fd_array[i];
				FD_CLR(DisconnectedSocket, &ReadSockets);
				closesocket(DisconnectedSocket);
			}
			else
			{
				std::cout << "Recv Message : " << recvCount << std::endl;
				for(int j = 0; j < static_cast<int>(ReadSockets.fd_count); ++j)
				{
					if (ReadSockets.fd_array[j] != Server)
					{
						int SendCount = send(ReadSockets.fd_array[j], (char*)Buffer, recvCount, 0);
						std::cout << "Send Message : " << SendCount << std::endl;
					}
				}
			}
		}
	}
}
```
변경된 소켓이 있는 경우 for문을 돌면서 변경된 소켓을 찾고, Server 소켓인 경우 -> Accept() 함수를 통해 Client와 연결 및 ReadSockets에 추가하였습니다.
Server 소켓이 아닌 경우 Client 소켓을 통해 패킷이 들어온 것이므로 Buffet를 통해 recv 후 Server 소켓을 제외한 모든 Client에게 패킷을 전달하였습니다.



