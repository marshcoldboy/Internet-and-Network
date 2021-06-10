#include <stdio.h>
#include <winsock2.h>
#include <stdbool.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")//加载了连接库文件，实现通信程序的管理

BOOL first_time = TRUE;
#define MSGLEN 128
#define HOSTLEN 512
#define oops(p){perror(p);exit(1);}
int main()
{
	WORD sockVision = MAKEWORD(2, 2);
	WSADATA wsadata;
	SOCKET sclient;
	struct sockaddr_in serAddr;
	char senddata[255];
	char revdata[255];
	int ret = -1;
	if (WSAStartup(sockVision, &wsadata) != 0)
	{
		printf("WSA初始化失败\n");
		return 0;

	}
	sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		printf("客户端创建socket失败\n");
		return 0;
	}
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(2021);//网络字节序转换
	//serAddr.sin_addr.S_un.S_addr = inet_addr("10.30.92.10");
	inet_pton(AF_INET, "192.168.43.140", &serAddr.sin_addr.s_addr);
	if (connect(sclient, (SOCKADDR*)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		//printf("%d", connect(sclient, (SOCKADDR*)&serAddr, sizeof(serAddr)));
		printf("socket客户端连接失败\n");
		return 0;
	}
	while (1)
	{
		while (1)
		{
			if (first_time)
			{
				printf("输入许可证序号\n");//
				gets_s(senddata);
				first_time = false;

			}
			//未输入的时候隔10s发送一个socket，有输入的时候按输入的走。
			else
			{
				printf("输入内容\n");
				gets_s(senddata);
			}
			send(sclient, senddata, strlen(senddata), 0);
			ret = recv(sclient, revdata, strlen(revdata), 0);
			if (ret > 0)
			{
				if (strncmp(revdata, "Sorry", 5) == 0)
				{
					printf("服务器已满\n");
					break;
				}
				else
				{
					if (strncmp(revdata, "GBYE", 4) == 0)
					{
						printf("成功退出\n");
						break;
					}
					else
						if (strncmp(revdata, "ATTENTION", 9) == 0)
						{
							printf(revdata);
							break;
						}
				}
				revdata[ret] = 0x00;
				printf("\n");
				printf("服务器：");
				printf(revdata);
				printf("\n");
			}

		}
	}
	closesocket(sclient);
	WSACleanup();
	system("pause");
	return 0;
}
