#include <stdio.h>
#include <winsock2.h>
#include <stdbool.h>
#include <WS2tcpip.h>
#include<time.h>
#pragma comment(lib,"ws2_32.lib")//加载了连接库文件，实现通信程序的管理
int num_ticket_out = 0;
int ticket_array[11];
char* do_hello(char* revdata)
{

}
char* do_goodbye(char* revdata)
{
	int x;
	for (x = 0; x < 10 && ticket_array[x] == 1; x++);
	if (x > 10)
		return "Sorry";
	ticket_array[x] = 0;
	num_ticket_out--;
	return "GBYE";
}
int IsFull()
{
	num_ticket_out++;
	if (num_ticket_out > 10)
		return 1;
	ticket_array[num_ticket_out] = 1;
	return 0;
}

void Initialize_tickets()
{
	for (int i = 0; i < strlen(ticket_array); i++)
		ticket_array[i] = 0;

}
int main(int argc, char argv[])
{
	char* response = "initialize";
	Initialize_tickets();
	//初始化wsa
	WORD sockVision = MAKEWORD(2, 2);
	WSADATA wsadata;
	//其他变量
	SOCKET slisten;
	SOCKET sClient;
	struct sockaddr_in remoteAddr;
	struct sockaddr_in sin;
	int ret = -1;
	int nAddrlen = sizeof(remoteAddr);
	char revdata[255];
	char senddata[255];
	if (WSAStartup(sockVision, &wsadata) != 0)
	{
		printf("wsa初始化失败\n");
		return 0;
	}
	//创建套接字
	slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		printf("socket监听者创建失败\n");
		return 0;
	}
	//绑定IP和端口
	sin.sin_family = AF_INET;
	sin.sin_port = htons(2021);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("绑定IP和端口失败\n");
		return 0;
	}

	//监听
	if (listen(slisten, 5) == SOCKET_ERROR)
	{
		printf("监听失败\n");
		return 0;
	}
	//循环接受数据
	while (1)
	{
		printf("等待连接.......\n");
		sClient = accept(slisten, (SOCKADDR*)&remoteAddr, &nAddrlen);
		if (sClient == INVALID_SOCKET)
		{
			printf("接受客户端失败\n");
			continue;
		}
		char StringBUF[16];
		memset(StringBUF, 0, 16 * sizeof(char));
		printf("接收客户端成功: %s\n", inet_ntop(AF_INET, (void*)&remoteAddr.sin_addr, StringBUF, 16));//网络地址转换成“.”点隔的字符串格式
		//接收数据
		while (1)
		{
			time_t first, second;
			time(&first);
			ret = recv(sClient, revdata, 255, 0);
			time(&second);
			double minus;
			minus = difftime(second, first);
			printf("%f\n", minus);

			if (minus > 75);
			{
				//bool flag = minus > 75;
				//printf("%d\n",flag);
				response = " ATTENTION,响应超时，请重新连接\n";
				send(sClient, response, strlen(response), 0);
				break;
			}
			if (ret > 0)
			{
				if (strncmp(revdata, "1234567890", 10) == 0)
				{

					if (!IsFull())
					{
						response = "Welcome to server!";
						printf("%s\n", response);

					}
					else
					{

						response = "Sorry,the number has reached";
						printf("%s\n", response);
						//fprintf(stderr, response, sClient);
						send(sClient, response, strlen(response), 0);
						//printf("")
						break;
					}
				}
				else

				{
					if (strncmp(revdata, "HELO", 4) == 0)
					{
						response = "HELO";
						printf("%s\n", response);
					}
					else
					{
						if (strncmp(revdata, "GBYE", 4) == 0)

						{
							response = do_goodbye(revdata);
							//fprintf(stderr, response, sClient);
							send(sClient, response, strlen(response), 0);
							printf("断开连接\n");
							break;
						}

						else
						{
							response = "FAIL invaild request";
							printf("%s\n", response);
							//fprintf(stderr, response, sClient);
							send(sClient, response, strlen(response), 0);
							break;
						}
					}
				}
				//fprintf(stderr, response, sClient);}
			}
			send(sClient, response, strlen(response), 0);
		}
		closesocket(sClient);
		printf("最里层循环结束\n");
	}
	closesocket(sClient);
	printf("最外层循环结束\n");
	WSACleanup();
	return 0;
}
