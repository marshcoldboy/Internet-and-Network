#include <stdio.h>
#include <winsock2.h>
#include <stdbool.h>
#include <WS2tcpip.h>
#include<time.h>
#pragma comment(lib,"ws2_32.lib")//���������ӿ��ļ���ʵ��ͨ�ų���Ĺ���
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
	//��ʼ��wsa
	WORD sockVision = MAKEWORD(2, 2);
	WSADATA wsadata;
	//��������
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
		printf("wsa��ʼ��ʧ��\n");
		return 0;
	}
	//�����׽���
	slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		printf("socket�����ߴ���ʧ��\n");
		return 0;
	}
	//��IP�Ͷ˿�
	sin.sin_family = AF_INET;
	sin.sin_port = htons(2021);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("��IP�Ͷ˿�ʧ��\n");
		return 0;
	}

	//����
	if (listen(slisten, 5) == SOCKET_ERROR)
	{
		printf("����ʧ��\n");
		return 0;
	}
	//ѭ����������
	while (1)
	{
		printf("�ȴ�����.......\n");
		sClient = accept(slisten, (SOCKADDR*)&remoteAddr, &nAddrlen);
		if (sClient == INVALID_SOCKET)
		{
			printf("���ܿͻ���ʧ��\n");
			continue;
		}
		char StringBUF[16];
		memset(StringBUF, 0, 16 * sizeof(char));
		printf("���տͻ��˳ɹ�: %s\n", inet_ntop(AF_INET, (void*)&remoteAddr.sin_addr, StringBUF, 16));//�����ַת���ɡ�.��������ַ�����ʽ
		//��������
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
				response = " ATTENTION,��Ӧ��ʱ������������\n";
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
							printf("�Ͽ�����\n");
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
		printf("�����ѭ������\n");
	}
	closesocket(sClient);
	printf("�����ѭ������\n");
	WSACleanup();
	return 0;
}
