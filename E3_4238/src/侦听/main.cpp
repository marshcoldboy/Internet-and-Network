
#define _CRT_SECURE_NO_WARNINGS
#include <pcap.h>
#include <Packet32.h>
#include <ntddndis.h>
#include <string>
#include <iostream>
#include <sstream>
#pragma comment(lib,"wpcap.lib")
#pragma comment(lib,"Packet.lib")
#pragma comment(lib, "WS2_32")
#define TO_FILE
/* 4 bytes IP address */
typedef struct ip_address
{
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
}ip_address;
typedef struct ip_header
{
	u_char	ver_ihl;		// Version (4 bits) + Internet header length (4 bits)
	u_char	tos;			// Type of service 
	u_short tlen;			// Total length 
	u_short identification; // Identification
	u_short flags_fo;		// Flags (3 bits) + Fragment offset (13 bits)
	u_char	ttl;			// Time to live
	u_char	proto;			// Protocol
	u_short crc;			// Header checksum
	ip_address	saddr;		// Source address
	ip_address	daddr;		// Destination address
	u_int	op_pad;			// Option + Padding
}ip_header;

typedef struct mac_header 
{
	u_char dest_addr[6];
	u_char src_addr[6];
	u_char type[2];
} mac_header;
/* prototype of the packet handler */
void packet_handler(u_char* param, const struct pcap_pkthdr
	* header, const u_char* pkt_data);
//#define FROM_NIC
int main()
{
#ifdef TO_FILE
	freopen("ans.csv", "w", stdout);
#endif
	pcap_if_t* alldevs;
	pcap_if_t* d;
	int inum;
	int i = 0;
	pcap_t* adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
	u_int netmask;
	char packet_filter[] = "ip and udp";
	struct bpf_program fcode;
#ifdef FROM_NIC
	/* Retrieve the device list */
	if (pcap_findalldevs(&alldevs,errbuf) == -1)
	{
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}
	/* Print the list */
	for (d = alldevs; d; d = d->next) {
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
		else
			printf(" (No description available)\n");
	}

	if (i == 0) {
		printf("\nNo interfaces found! Make sure WinPcap isinstalled.\n");
		return -1;
	}
	printf("Enter the interface number (1-%d):", i);
	scanf_s("%d", &inum);

	if (inum < 1 || inum > i) {
		printf("\nInterface number out of range.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}
	/* Jump to the selected adapter */
	for (d = alldevs, i = 0; i < inum - 1; d = d->next, i++);

	/* Open the adapter */
	if ((adhandle = pcap_open_live(d->name, 65536,1, 1000,errbuf)) == NULL) 
	{
		fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n");
		pcap_freealldevs(alldevs);
		return -1;
	}

	if (pcap_datalink(adhandle) != DLT_EN10MB) {
		fprintf(stderr, "\nThis program works only on Ethernet networks.\n");
		pcap_freealldevs(alldevs);
		return -1;
	}
	if (d->addresses != NULL)
		netmask = ((struct sockaddr_in*)(d->addresses->netmask))->sin_addr.S_un.S_addr;
	else
		netmask = 0xffffff;

	//compile the filter
	if (pcap_compile(adhandle, &fcode, packet_filter, 1, netmask)
		< 0) {
		fprintf(stderr, "\nUnable to compile the packet filter. Check the syntax.\n");
		pcap_freealldevs(alldevs);
		return -1;
	}
	//set the filter
	if (pcap_setfilter(adhandle, &fcode) < 0) {
		fprintf(stderr, "\nError setting the filter.\n");
		pcap_freealldevs(alldevs);
		return -1;
	}

	printf("\nlistening on %s...\n", d->description);
	/* At this point, we don't need any more the device list.
	Free it */
	pcap_freealldevs(alldevs);
	/* start the capture */
	pcap_loop(adhandle, 0, packet_handler, NULL);
#else
	/* Open the capture file */
	if ((adhandle = pcap_open_offline("E:\\Ftp.pcap",			// name of the device
		errbuf					// error buffer
	)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the file.\n");
		return -1;
	}
	pcap_loop(adhandle, 0, packet_handler, NULL);

	pcap_close(adhandle);
#endif
	return 0;
}
std::string user, pass;
void packet_handler(u_char* param, const struct pcap_pkthdr
	* header, const u_char* pkt_data)
{
	struct tm* ltime;
	char timestr[80];
	u_int ip_len;
	u_short sport, dport;
	time_t local_tv_sec;

	/*
	 * unused parameter
	 */
	(VOID)(param);

	/* convert the timestamp to readable format */
	local_tv_sec = header->ts.tv_sec;
	ltime = localtime(&local_tv_sec);
	strftime(timestr, sizeof timestr, "%Y-%m-%d %H:%M:%S", ltime);
	mac_header* mh;
	mh = (mac_header*)pkt_data;
	ip_header* ih;
	ih = (ip_header*)(pkt_data +
		14); //length of ethernet header
	int head;
	std::string com;
	for (head = 0; head < 60; head++)
	{
		com.clear();
		for (int i = 0; i < 4; i++)
			com += (char)pkt_data[head + i];
		if (com == "USER" || com == "PASS" || com == "230 " || com == "530 ")
			break;
	}
	if (com == "USER")
	{
		std::ostringstream sout;
		for (int i = head + 5; pkt_data[i] != 13; i++)
		{
			sout << pkt_data[i];
		}
		user = sout.str();
	}
	if (com == "PASS")
	{
		std::ostringstream sout;
		for (int i = head + 5; pkt_data[i] != 13; i++)
		{
			sout << pkt_data[i];
		}
		pass = sout.str();
	}
	if (com == "230 ")
	{
		printf( "%s,", timestr);
		for (int i = 0; i < 6; i++)
		{
			printf("%02X", mh->dest_addr[i]);
			if (i != 5)
				printf("-");
		}
		printf(",");
		printf( "%d.%d.%d.%d,",
			ih->saddr.byte1,
			ih->saddr.byte2,
			ih->saddr.byte3,
			ih->saddr.byte4);
		for (int i = 0; i < 6; i++)
		{
			printf("%02X", mh->src_addr[i]);
			if (i != 5)
				printf("-");
		}
		printf(",");
		printf( "%d.%d.%d.%d,",
			ih->daddr.byte1,
			ih->daddr.byte2,
			ih->daddr.byte3,
			ih->daddr.byte4);
		//printf("%d,", header->len);
		std::cout << user << "," << pass << ",";
		printf("SUCCEED\n");
		user.clear();
		pass.clear();
	}
	if (com == "530 ")
	{
		printf("%s,", timestr);
		for (int i = 0; i < 6; i++)
		{
			printf("%02X", mh->dest_addr[i]);
			if (i != 5)
				printf("-");
		}
		printf(",");
		printf("%d.%d.%d.%d,",
			ih->saddr.byte1,
			ih->saddr.byte2,
			ih->saddr.byte3,
			ih->saddr.byte4);
		for (int i = 0; i < 6; i++)
		{
			printf("%02X", mh->src_addr[i]);
			if (i != 5)
				printf("-");
		}
		printf(",");
		printf("%d.%d.%d.%d,",
			ih->daddr.byte1,
			ih->daddr.byte2,
			ih->daddr.byte3,
			ih->daddr.byte4);
		//printf("%d,", header->len);
		std::cout << user << "," << pass << ",";
		printf("FAILED\n");
		user.clear();
		pass.clear();
	}
	
}