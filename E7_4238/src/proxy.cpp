#include <stdio.h> 
#include <Windows.h> 
#include <process.h> 
#include <string.h> 
#include <tchar.h>

#pragma comment(lib,"Ws2_32.lib") 

#define MAXSIZE 65507 //�������ݱ��ĵ���󳤶� 
#define HTTP_PORT 80 //http �������˿� 

#define CACHE_MAXSIZE 100
#define DATELENGTH 40
//Http ��Ҫͷ������ 
struct HttpHeader {
	char method[7]; // POST ���� GET��ע����ЩΪ CONNECT����ʵ���ݲ�����
	char url[1024];  //  ����� url 
	char host[1024]; //  Ŀ������ 
	char cookie[1024 * 10]; //cookie 
	HttpHeader() {
		ZeroMemory(this, sizeof(HttpHeader));
	}
};
//����Ǹ߷µ�http��ͷ�����ݣ�������cache���ҵ����󣬵��ǽ�Լ��cookie�Ŀռ�
struct cache_HttpHeader {
	char method[4]; // POST ���� GET��ע����ЩΪ CONNECT����ʵ���ݲ�����
	char url[1024];  //  ����� url 
	char host[1024]; //  Ŀ������ 
	cache_HttpHeader() {
		ZeroMemory(this, sizeof(cache_HttpHeader));
	}
};
//ʵ�ִ���������Ļ��漼��
struct __CACHE {
	cache_HttpHeader htphed;
	char buffer[MAXSIZE];
	char date[DATELENGTH];//�洢�ĸ���ʱ��
	__CACHE() {
		ZeroMemory(this->buffer, MAXSIZE);
		ZeroMemory(this->buffer, sizeof(date));
	}

};
int __CACHE_number = 0;//�����һ��Ӧ�÷Ż����λ��
__CACHE cache[CACHE_MAXSIZE];//��`����

BOOL InitSocket();
BOOL ParseHttpHead(char* buffer, HttpHeader* httpHeader);
BOOL ConnectToServer(SOCKET* serverSocket, char* host);
unsigned int __stdcall ProxyThread(LPVOID lpParameter);
int Cache_find(__CACHE* cache, HttpHeader htp);

//������ز��� 
SOCKET ProxyServer;
sockaddr_in ProxyServerAddr;
const int ProxyPort = 10240;
//��ֹ���ʵ�������������վ
char* host[10] = { "today.hit.edu.cn", "jwts.hit.edu.cn", "resource.hit.edu.cn", "www.hit.edu.cn" };
const int host_number = 4;

//��վ�յ�
char* host_to_another = "djangobook.py3k.cn";//�յ���pt.hit.edu.cn,url=http://pt.hit.edu.cn/  ,urlԴ=http://djangobook.py3k.cn/2.0/
char* another[2] = { "pt.hit.edu.cn","http://pt.hit.edu.cn/" };//��ת���ĵ�ַ
//char* host_to_another = "www.pku.edu.cn";
//char* another[2] = { "today.hit.edu.cn", "http://today.hit.edu.cn/" };
//char Cache[][];
//�����µ����Ӷ�ʹ�����߳̽��д������̵߳�Ƶ���Ĵ����������ر��˷���Դ
//����ʹ���̳߳ؼ�����߷�����Ч�� 
//const int ProxyThreadMaxNum = 20; 
//HANDLE ProxyThreadHandle[ProxyThreadMaxNum] = {0}; 
//DWORD ProxyThreadDW[ProxyThreadMaxNum] = {0}; 
struct ProxyParam {
	SOCKET clientSocket;
	SOCKET serverSocket;
};

int _tmain(int argc, _TCHAR* argv[])
{

	printf("�����������������\n");
	printf("��ʼ��...\n");
	if (!InitSocket()) {

		printf("socket ��ʼ��ʧ��\n");
		return -1;
	}
	printf("����������������У������˿�  %d\n", ProxyPort);
	SOCKET acceptSocket = INVALID_SOCKET;
	ProxyParam* lpProxyParam;
	HANDLE hThread;
	DWORD dwThreadID;
	//������������ϼ��� 
	sockaddr_in verAddr;
	int hahaha = sizeof(SOCKADDR);
	while (true) {
		acceptSocket = accept(ProxyServer, (SOCKADDR*)&verAddr, &(hahaha));
		lpProxyParam = new ProxyParam;
		if (lpProxyParam == NULL) {
			continue;
		}
		/*if (!strcmp("127.0.0.1", inet_ntoa(verAddr.sin_addr))) {
			printf("�����Ƶ��û����ʣ�\n");
			continue;
		}*/
		lpProxyParam->clientSocket = acceptSocket;
		hThread = (HANDLE)_beginthreadex(NULL, 0,
			&ProxyThread, (LPVOID)lpProxyParam, 0, 0);
		CloseHandle(hThread);
		Sleep(200);
	}
	closesocket(ProxyServer);
	WSACleanup();
	return 0;
}


//************************************ 
// Method:        InitSocket 
// FullName:    InitSocket 
// Access:        public   
// Returns:      BOOL 
// Qualifier:  ��ʼ���׽��� 
//************************************ 
BOOL InitSocket() {

	//�����׽��ֿ⣨���룩 
	WORD wVersionRequested;
	WSADATA wsaData;
	//�׽��ּ���ʱ������ʾ 
	int err;
	//�汾 2.2 
	wVersionRequested = MAKEWORD(2, 2);
	//���� dll �ļ� Scoket ��   
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		//�Ҳ��� winsock.dll 
		printf("���� winsock ʧ�ܣ��������Ϊ: %d\n", WSAGetLastError());
		return FALSE;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("�����ҵ���ȷ�� winsock �汾\n");
		WSACleanup();
		return FALSE;
	}
	ProxyServer = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == ProxyServer) {
		printf("�����׽���ʧ�ܣ��������Ϊ��%d\n", WSAGetLastError());
		return FALSE;
	}
	ProxyServerAddr.sin_family = AF_INET;
	ProxyServerAddr.sin_port = htons(ProxyPort);
	ProxyServerAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(ProxyServer, (SOCKADDR*)&ProxyServerAddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		printf("���׽���ʧ��\n");
		return FALSE;
	}
	if (listen(ProxyServer, SOMAXCONN) == SOCKET_ERROR) {
		printf("�����˿�%d ʧ��", ProxyPort);
		return FALSE;
	}
	return TRUE;
}

//************************************ 
// Method:        ProxyThread 
// FullName:    ProxyThread 
// Access:        public   
// Returns:      unsigned int __stdcall 
// Qualifier:  �߳�ִ�к��� 
// Parameter: LPVOID lpParameter 
//************************************ 
unsigned int __stdcall ProxyThread(LPVOID lpParameter) {
	char Buffer[MAXSIZE];
	char* CacheBuffer;
	ZeroMemory(Buffer, MAXSIZE);
	SOCKADDR_IN clientAddr;
	int length = sizeof(SOCKADDR_IN);
	int recvSize;
	int ret;
	recvSize = recv(((ProxyParam
		*)lpParameter)->clientSocket, Buffer, MAXSIZE, 0);
	if (recvSize <= 0) {
		goto error;
	}

	HttpHeader* httpHeader = new HttpHeader();
	CacheBuffer = new char[recvSize + 1];
	ZeroMemory(CacheBuffer, recvSize + 1);
	memcpy(CacheBuffer, Buffer, recvSize);
	bool change = ParseHttpHead(CacheBuffer, httpHeader);
	delete CacheBuffer;

	/*int j = 0;
	//�Զ�Ӧ����վ���ܷ���--������վ��������ok
	for (j = 0; j < host_number; j++)//4��ʾ������վ�ĸ�������Ȼ������10������ʵ����ֻ��4����������4
	{
		int i = 0;
		bool find = true;
		for (i = 0; i < strlen(host[j]); i++) {
			if (host[j][i] != httpHeader->host[i]) {
				find = false;
				break;
			}
		}
		if (find)
			goto error;
	}

	*/
	if (!ConnectToServer(&((ProxyParam
		*)lpParameter)->serverSocket, httpHeader->host)) {
		goto error;
	}
	printf("������������  %s  �ɹ�\n", httpHeader->host);
	//��ת������һ����վ�����change==tre
	if (change)
	{
		char* CacheBuffer;
		CacheBuffer = new char[MAXSIZE];
		ZeroMemory(CacheBuffer, MAXSIZE);
		//memcpy(CacheBuffer, Buffer, recvSize);
		int ii = 0, lengthth = 0;
		for (ii = 0; ii < strlen(httpHeader->method); ii++) {
			CacheBuffer[lengthth++] = httpHeader->method[ii];
		}
		CacheBuffer[lengthth++] = ' ';
		for (ii = 0; ii < strlen(another[1]); ii++) {
			CacheBuffer[lengthth++] = another[1][ii];
		}
		CacheBuffer[lengthth++] = ' ';
		char* hh = "HTTP/1.1";
		for (ii = 0; ii < strlen(hh); ii++) {
			CacheBuffer[lengthth++] = hh[ii];
		}
		CacheBuffer[lengthth++] = '\r';
		CacheBuffer[lengthth++] = '\n';
		char* hhh = "HOST: ";
		for (ii = 0; ii < strlen(hhh); ii++) {
			CacheBuffer[lengthth++] = hhh[ii];
		}
		for (ii = 0; ii < strlen(httpHeader->host); ii++) {
			CacheBuffer[lengthth++] = httpHeader->host[ii];
		}
		CacheBuffer[lengthth++] = '\r';
		CacheBuffer[lengthth++] = '\n';
		char* ptr;
		const char* delim = "\r\n";
		char* p = strtok_s(Buffer, delim, &ptr);
		int length1 = strlen(p);
		length1 += 2;
		p = strtok_s(NULL, delim, &ptr);
		length1 += strlen(p);
		length1 += 2;
		for (ii = 1; ii < recvSize - length1 + 1; ii++) {
			CacheBuffer[lengthth++] = ptr[ii];
		}
		memcpy(Buffer, CacheBuffer, max(strlen(CacheBuffer), recvSize));
		delete CacheBuffer;
	}
	//printf("BufferΪ----------------------------------\n%s", Buffer);
	//�ڻ������ҵ��˸ö���
	int find = Cache_find(cache, *httpHeader);
	if (find >= 0)
	{
		char* CacheBuffer;
		char Buffer2[MAXSIZE];
		int i = 0, length = 0, length2 = 0;
		ZeroMemory(Buffer2, MAXSIZE);
		CacheBuffer = new char[recvSize + 1];
		ZeroMemory(CacheBuffer, recvSize + 1);
		memcpy(CacheBuffer, Buffer, recvSize);

		const char* delim = "\r\n";
		char* ptr;
		char* p = strtok_s(CacheBuffer, delim, &ptr);
		length += strlen(p);
		length += 2;
		p = strtok_s(NULL, delim, &ptr);
		length += strlen(p);
		length += 2;
		length2 = length;

		char* ife = "If-Modified-Since: ";
		for (i = 0; i < length; i++) {
			Buffer2[i] = Buffer[i];
		}
		for (i = 0; i < strlen(ife); i++)
		{
			Buffer2[length + i] = ife[i];
		}
		length = length + strlen(ife);
		for (i = 0; i < strlen(cache[find].date); i++)
		{
			Buffer2[length + i] = cache[find].date[i];
		}
		length += strlen(cache[find].date);
		Buffer2[length++] = '\r';
		Buffer2[length++] = '\n';
		for (i = length2; i < recvSize; i++) {
			Buffer2[length++] = Buffer[i];
		}
		delete CacheBuffer;

		//printf("BUFFER2222222222222222222222222222222222222222222:\n%s\n", Buffer2);
		//���ͻ��˷��͵� HTTP ���ݱ��Ĵ����ת����Ŀ������� 
		ret = send(((ProxyParam*)lpParameter)->serverSocket, Buffer2, strlen(Buffer2)
			+ 1, 0);
		//�ȴ�Ŀ��������������� 
		recvSize = recv(((ProxyParam
			*)lpParameter)->serverSocket, Buffer2, MAXSIZE, 0);

		if (recvSize <= 0) {
			goto error;
		}
		//printf("\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\%s\n",Buffer2);
		const char* blank = " ";
		const char* Modd = "304";
		if (!memcmp(&Buffer2[9], Modd, strlen(Modd)))
		{
			ret = send(((ProxyParam
				*)lpParameter)->clientSocket, cache[find].buffer, strlen(cache[find].buffer) + 1, 0);
			goto error;
		}
	}
	//���ͻ��˷��͵� HTTP ���ݱ���ֱ��ת����Ŀ������� 
	ret = send(((ProxyParam*)lpParameter)->serverSocket, Buffer, strlen(Buffer)
		+ 1, 0);
	//�ȴ�Ŀ��������������� 
	recvSize = recv(((ProxyParam
		*)lpParameter)->serverSocket, Buffer, MAXSIZE, 0);
	/*if (recvSize <= 0) {
		goto error;
	}*/

	//�õ�ʱ��
	char* chacheBuff = new char[MAXSIZE];
	ZeroMemory(chacheBuff, MAXSIZE);
	memcpy(chacheBuff, Buffer, MAXSIZE);
	const char* delim = "\r\n";
	char* ptr;
	char dada[DATELENGTH];
	ZeroMemory(dada, sizeof(dada));
	char* p = strtok_s(chacheBuff, delim, &ptr);
	bool cun = false;
	while (p) {
		if (p[0] == 'L') {
			if (strlen(p) > 15) {
				char header[15];
				ZeroMemory(header, sizeof(header));
				memcpy(header, p, 14);
				if (!(strcmp(header, "Last-Modified:")))
				{
					memcpy(dada, &p[15], strlen(p) - 15);
					cun = true;
					break;
				}
			}
		}
		p = strtok_s(NULL, delim, &ptr);
	}
	if (cun) {
		//���µĶ������뵽������ȥ
		if (find >= 0)
		{
			memcpy(&(cache[find].buffer), Buffer, strlen(Buffer));
			memcpy(&(cache[find].date), dada, strlen(dada));

			//printf("----------------%s\n",dada);
		}
		else
		{
			memcpy(&(cache[__CACHE_number % CACHE_MAXSIZE].htphed.host), httpHeader->host, strlen(httpHeader->host));
			memcpy(&(cache[__CACHE_number % CACHE_MAXSIZE].htphed.method), httpHeader->method, strlen(httpHeader->method));
			memcpy(&(cache[__CACHE_number % CACHE_MAXSIZE].htphed.url), httpHeader->url, strlen(httpHeader->url));
			memcpy(&(cache[__CACHE_number % CACHE_MAXSIZE].buffer), Buffer, strlen(Buffer));
			memcpy(&(cache[__CACHE_number % CACHE_MAXSIZE].date), dada, strlen(dada));
			__CACHE_number++;
		}
	}
	//printf("�յ�����Ӧ����: %s \n", Buffer);

	//��Ŀ����������ص�����ֱ��ת�����ͻ��� 
	ret = send(((ProxyParam
		*)lpParameter)->clientSocket, Buffer, sizeof(Buffer), 0);
	//������ 
error:
	printf("�ر��׽���\n");
	Sleep(200);
	closesocket(((ProxyParam*)lpParameter)->clientSocket);
	closesocket(((ProxyParam*)lpParameter)->serverSocket);
	delete    lpParameter;
	_endthreadex(0);
	return 0;
}

//************************************ 
// Method:        ParseHttpHead 
// FullName:    ParseHttpHead 
// Access:        public   
// Returns:      void 
// Qualifier:  ���� TCP �����е� HTTP ͷ�� 
// Parameter: char * buffer 
// Parameter: HttpHeader * httpHeader 
//************************************ 
BOOL ParseHttpHead(char* buffer, HttpHeader* httpHeader) {
	char* p;
	char* ptr;
	bool change = false;
	const char* delim = "\r\n";
	/*
	strtok()�������ַ����ָ��һ����Ƭ�Ρ�����sָ�����ָ���ַ���������delim��Ϊ�ָ��ַ����а����������ַ�����strtok()�ڲ���s���ַ����з��ֲ���delim�а����ķָ��ַ�ʱ,��Ὣ���ַ���Ϊ\0 �ַ����ڵ�һ�ε���ʱ��strtok()����������s�ַ���������ĵ����򽫲���s���ó�NULL��ÿ�ε��óɹ��򷵻�ָ�򱻷ָ��Ƭ�ε�ָ�롣
	strtok�������ƻ����ֽ��ַ���������������ǰ�͵��ú��s�Ѿ���һ���ˡ�
	*/
	p = strtok_s(buffer, delim, &ptr);//��ȡ��һ�� 
	printf("%s\n", p);
	if (p[0] == 'G') {//GET ��ʽ 
		memcpy(httpHeader->method, "GET", 3);

		memcpy(httpHeader->url, &p[4], strlen(p) - 13);
	}
	else if (p[0] == 'P') {//POST ��ʽ 
		memcpy(httpHeader->method, "POST", 4);
		memcpy(httpHeader->url, &p[5], strlen(p) - 14);
	}
	printf("%s\n", httpHeader->url);
	p = strtok_s(NULL, delim, &ptr);//��ȡ�ڶ���

	while (p) {
		//printf("-------%s\n", p);
		switch (p[0]) {
		case 'H'://Host 
			if (!memcmp(&p[6], host_to_another, strlen(p) - 6))
			{
				memcpy(httpHeader->host, another[0], strlen(another[0]));
				//strcpy(httpHeader->url, "");
				memcpy(httpHeader->url, another[1], strlen(another[1]));
				change = true;
			}
			else {
				memcpy(httpHeader->host, &p[6], strlen(p) - 6);
			}
			break;
		case 'C'://Cookie 
			if (strlen(p) > 8) {
				char header[8];
				ZeroMemory(header, sizeof(header));
				memcpy(header, p, 6);
				if (!strcmp(header, "Cookie")) {
					memcpy(httpHeader->cookie, &p[8], strlen(p) - 8);
				}
			}
			break;
		default:
			break;
		}
		p = strtok_s(NULL, delim, &ptr);
	}
	return change;
}

//************************************ 
// Method:        ConnectToServer 
// FullName:    ConnectToServer 
// Access:        public   
// Returns:      BOOL 
// Qualifier:  ������������Ŀ��������׽��֣������� 
// Parameter: SOCKET * serverSocket 
// Parameter: char * host 
//************************************ 
BOOL ConnectToServer(SOCKET* serverSocket, char* host) {
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(HTTP_PORT);
	HOSTENT* hostent = gethostbyname(host);//��������Ĵ���ֵ����������������������"www.google.cn"�ȵȡ�����ֵ����һ��hostent�Ľṹ�������������ʧ�ܣ�������NULL��
	if (!hostent) {
		return FALSE;
	}
	/*
	����hostent�ṹ������ָ��
	struct hostent
	{
	char    *h_name;
	char    **h_aliases;
	int     h_addrtype;
	int     h_length;
	char    **h_addr_list;
	#define h_addr h_addr_list[0]
	};
	hostent->h_name
	��ʾ���������Ĺ淶��������www.google.com�Ĺ淶����ʵ��www.l.google.com��
	hostent->h_aliases
	��ʾ���������ı���.www.google.com����google���Լ��ı������е�ʱ���е����������кü�����������Щ����ʵ����Ϊ�������û������Ϊ�Լ�����վ��ȡ�����֡�
	hostent->h_addrtype
	��ʾ��������ip��ַ�����ͣ�������ipv4(AF_INET)������pv6(AF_INET6)
	hostent->h_length
	��ʾ��������ip��ַ�ĳ���
	hostent->h_addr_lisst
	��ʾ����������ip��ַ��ע�⣬������������ֽ���洢�ġ�ǧ��Ҫֱ����printf��%s���������������������������ۡ����Ե�������Ҫ��ӡ�����IP�Ļ�����Ҫ����inet_ntop()��
	const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt) ��
	����������ǽ�����Ϊaf�������ַ�ṹsrc��ת������������ַ�����ʽ������ڳ���Ϊcnt���ַ����С�����ָ��dst��һ��ָ�롣����������ô��󣬷���ֵ��NULL��
	*/
	in_addr Inaddr = *((in_addr*)*hostent->h_addr_list);//������ip��ַ
	serverAddr.sin_addr.s_addr = inet_addr(inet_ntoa(Inaddr));
	*serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (*serverSocket == INVALID_SOCKET) {
		return FALSE;
	}
	if (connect(*serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr))
		== SOCKET_ERROR) {
		closesocket(*serverSocket);
		return FALSE;
	}
	return TRUE;
}
//�ж����������Ƿ���ͬ--������cookie
BOOL Isequal(cache_HttpHeader htp1, HttpHeader htp2)
{
	if (strcmp(htp1.method, htp2.method)) return false;
	if (strcmp(htp1.url, htp2.url)) return false;
	if (strcmp(htp1.host, htp2.host)) return false;
	return true;
}

//�ڻ������ҵ���Ӧ�Ķ���
int Cache_find(__CACHE* cache, HttpHeader htp)
{
	int i = 0;
	for (i = 0; i < CACHE_MAXSIZE; i++)
	{
		if (Isequal(cache[i].htphed, htp)) return i;
	}
	return -1;
}