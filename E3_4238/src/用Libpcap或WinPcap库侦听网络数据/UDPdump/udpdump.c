/*
 * Copyright (c) 1999 - 2005 NetGroup, Politecnico di Torino (Italy)
 * Copyright (c) 2005 - 2006 CACE Technologies, Davis (California)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Politecnico di Torino, CACE Technologies 
 * nor the names of its contributors may be used to endorse or promote 
 * products derived from this software without specific prior written 
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifdef _MSC_VER
/*
 * we do not want the warnings about the old deprecated and unsecure CRT functions
 * since these examples can be compiled under *nix as well
 */
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "pcap.h"

/* 4 bytes IP address */
typedef struct ip_address
{
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
}ip_address;

/* IPv4 header */
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

/* UDP header*/
typedef struct udp_header
{
	u_short sport;			// Source port
	u_short dport;			// Destination port
	u_short len;			// Datagram length
	u_short crc;			// Checksum
}udp_header;

/* prototype of the packet handler */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

typedef struct mac_header {
	u_char dest_addr[6];
	u_char src_addr[6];
	u_char type[2];
} mac_header;

FILE* file = NULL;
unsigned long long int start;
unsigned long long int preview;
unsigned long long int current;
unsigned long long int get_pre;

u_char src[100000][10] = { 0 };
u_char dest[100000][10] = { 0 };
int src_packet_length[100000] = { 0 };
int dest_packet_length[100000] = { 0 };
int src_length = 0;
int dest_length = 0;
int main()
{
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int inum;
	int i=0;
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
	u_int netmask;
	char packet_filter[] = "ip and udp";
	struct bpf_program fcode;
	
	/* Retrieve the device list */
	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}
	
	/* Print the list */
	for(d=alldevs; d; d=d->next)
	{
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
		else
			printf(" (No description available)\n");
	}

	if(i==0)
	{
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
		return -1;
	}
	
	printf("Enter the interface number (1-%d):",i);
	scanf("%d", &inum);
	
	/* Check if the user specified a valid adapter */
	if(inum < 1 || inum > i)
	{
		printf("\nAdapter number out of range.\n");
		
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/* Jump to the selected adapter */
	for(d=alldevs, i=0; i< inum-1 ;d=d->next, i++);
	
	/* Open the adapter */
	if ((adhandle= pcap_open_live(d->name,	// name of the device
							 65536,			// portion of the packet to capture. 
											// 65536 grants that the whole packet will be captured on all the MACs.
							 1,				// promiscuous mode (nonzero means promiscuous)
							 1000,			// read timeout
							 errbuf			// error buffer
							 )) == NULL)
	{
		fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}
	
	/* Check the link layer. We support only Ethernet for simplicity. */
	if(pcap_datalink(adhandle) != DLT_EN10MB)
	{
		fprintf(stderr,"\nThis program works only on Ethernet networks.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}
	
	if(d->addresses != NULL)
		/* Retrieve the mask of the first address of the interface */
		netmask=((struct sockaddr_in *)(d->addresses->netmask))->sin_addr.S_un.S_addr;
	else
		/* If the interface is without addresses we suppose to be in a C class network */
		netmask=0xffffff; 


	//compile the filter
	if (pcap_compile(adhandle, &fcode, packet_filter, 1, netmask) <0 )
	{
		fprintf(stderr,"\nUnable to compile the packet filter. Check the syntax.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}
	
	//set the filter
	if (pcap_setfilter(adhandle, &fcode)<0)
	{
		fprintf(stderr,"\nError setting the filter.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}
	
	printf("\nlistening on %s...\n", d->description);
	get_pre = GetTickCount64();
	start = GetTickCount64();
	preview = GetTickCount64();
	
	/* At this point, we don't need any more the device list. Free it */
	pcap_freealldevs(alldevs);
	
	/* start the capture */
	pcap_loop(adhandle, 0, packet_handler, NULL);
	
	return 0;
}

/* Callback function invoked by libpcap for every incoming packet */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	struct tm *ltime;
	char timestr[80];
	ip_header *ih;
	udp_header *uh;
	mac_header* mh;
	u_int ip_len;
	u_short sport,dport;
	time_t local_tv_sec;

	/*
	 * unused parameter
	 */
	(VOID)(param);

	/* convert the timestamp to readable format */
	local_tv_sec = header->ts.tv_sec;
	ltime=localtime(&local_tv_sec);

	strftime( timestr, sizeof timestr, "%Y-%m-%d %H:%M:%S", ltime);
	mh = (mac_header*)pkt_data;
	if ((file = fopen("Myoutput.csv", "a")) == NULL)
	{
		file = fopen("Myoutput.csv", "w");
	}
	if (file == NULL)
	{
		printf("file is null\n");
		return;
	}
	/* print timestamp and length of the packet */
	//printf("%s.%.6d len:%d ", timestr, header->ts.tv_usec, header->len);
	fprintf(file,"%s,", timestr);
	for (int i = 0; i < 6; i++)
	{
		fprintf(file,"%02X", mh->dest_addr[i]);
		if (i != 5)
			fprintf(file,"-");
	}
	fprintf(file,",");
	/* retireve the position of the ip header */
	ih = (ip_header *) (pkt_data +
		14); //length of ethernet header

	/* retireve the position of the udp header */
	ip_len = (ih->ver_ihl & 0xf) * 4;
	uh = (udp_header *) ((u_char*)ih + ip_len);

	/* convert from network byte order to host byte order */
	sport = ntohs( uh->sport );
	dport = ntohs( uh->dport );
	/* print ip addresses and udp ports */
	fprintf(file,"%d.%d.%d.%d,",
		ih->saddr.byte1,
		ih->saddr.byte2,
		ih->saddr.byte3,
		ih->saddr.byte4);
	
	for (int i = 0; i < 6; i++)
	{
		fprintf(file,"%02X", mh->src_addr[i]);
		if (i != 5)
			fprintf(file,"-");
	}
	fprintf(file,",");
	fprintf(file,"%d.%d.%d.%d,",
		ih->daddr.byte1,
		ih->daddr.byte2,
		ih->daddr.byte3,
		ih->daddr.byte4);
	//printf("\ttype: %04X", ntohs((u_short)mh->type));
	fprintf(file,"%d\n",header->len);
	int flag = 0;
	for (int i = 0; i < src_length; i++)
	{
		if (src[i][0] == ih->saddr.byte1
			&& src[i][1] == ih->saddr.byte2
			&& src[i][2] == ih->saddr.byte3
			&& src[i][3] == ih->saddr.byte4
			&& src[i][4] == mh->src_addr[0]
			&& src[i][5] == mh->src_addr[1]
			&& src[i][6] == mh->src_addr[2]
			&& src[i][7] == mh->src_addr[3]
			&& src[i][8] == mh->src_addr[4]
			&& src[i][9] == mh->src_addr[5])
		{
			src_packet_length[i] += header->len;
			flag = 1;
			break;
		}
	}
	if (flag == 0)
	{
		src[src_length][0] = ih->saddr.byte1;
		src[src_length][1] = ih->saddr.byte2;
		src[src_length][2] = ih->saddr.byte3;
		src[src_length][3] = ih->saddr.byte4;
		src[src_length][4] = mh->src_addr[0];
		src[src_length][5] = mh->src_addr[1];
		src[src_length][6] = mh->src_addr[2];
		src[src_length][7] = mh->src_addr[3];
		src[src_length][8] = mh->src_addr[4];
		src[src_length][9] = mh->src_addr[5];
		src_packet_length[src_length] = header->len;
		src_length++;
	}

	flag = 0;
	for (int i = 0; i < dest_length; i++)
	{
		if (dest[i][0] == ih->daddr.byte1
			&& dest[i][1] == ih->daddr.byte2
			&& dest[i][2] == ih->daddr.byte3
			&& dest[i][3] == ih->daddr.byte4
			&& dest[i][4] == mh->dest_addr[0]
			&& dest[i][5] == mh->dest_addr[1]
			&& dest[i][6] == mh->dest_addr[2]
			&& dest[i][7] == mh->dest_addr[3]
			&& dest[i][8] == mh->dest_addr[4]
			&& dest[i][9] == mh->dest_addr[5])
		{
			dest_packet_length[i] += header->len;
			flag = 1;
			break;
		}
	}
	if (flag == 0)
	{
		dest[dest_length][0] = ih->daddr.byte1;
		dest[dest_length][1] = ih->daddr.byte2;
		dest[dest_length][2] = ih->daddr.byte3;
		dest[dest_length][3] = ih->daddr.byte4;
		dest[dest_length][4] = mh->dest_addr[0];
		dest[dest_length][5] = mh->dest_addr[1];
		dest[dest_length][6] = mh->dest_addr[2];
		dest[dest_length][7] = mh->dest_addr[3];
		dest[dest_length][8] = mh->dest_addr[4];
		dest[dest_length][9] = mh->dest_addr[5];
		dest_packet_length[dest_length] = header->len;
		dest_length++;
	}
	if (GetTickCount64() - preview >= 60000)
	{
		fprintf(file, "1min（源ip与MAC地址）:\n");
		for (int i = 0; i < src_length; i++)
		{
			fprintf(file, "第%d个,ip地址为%d.%d.%d.%d,MAC地址为%02X-%02X-%02X-%02X-%02X-%02X,数据包总数为%d\n",
				(i + 1),
				src[i][0],
				src[i][1],
				src[i][2],
				src[i][3],
				src[i][4],
				src[i][5],
				src[i][6],
				src[i][7],
				src[i][8],
				src[i][9],
				src_packet_length[i]);
		}
		fprintf(file, "1min（目的ip与MAC地址）:\n");
		for (int i = 0; i < dest_length; i++)
		{
			fprintf(file, "第%d个,ip地址为%d.%d.%d.%d,MAC地址为%02X:%02X:%02X:%02X:%02X:%02X,数据包总数为%d\n",
				(i + 1),
				dest[i][0],
				dest[i][1],
				dest[i][2],
				dest[i][3],
				dest[i][4],
				dest[i][5],
				dest[i][6],
				dest[i][7],
				dest[i][8],
				dest[i][9],
				dest_packet_length[i]);
		}
		memset(src, 0, sizeof(src));
		memset(dest, 0, sizeof(dest));
		memset(src_packet_length, 0, sizeof(src_packet_length));
		memset(dest_packet_length, 0, sizeof(dest_packet_length));
		src_length = 0;
		dest_length = 0;
		preview = GetTickCount64();
	}
	if (GetTickCount64() - start >= 125000)
	{
		printf("done");
		exit(1);
	}
	fclose(file);

	if (GetTickCount64() - get_pre >= 1000)
	{
		printf("%d s ago，写入成功\n", (GetTickCount64() - start) / 1000);
		get_pre = GetTickCount64();
	}
	/*int length = sizeof(mac_header) + sizeof(ip_header);
	for (int i = 0; i < length; i++) {
		printf("%02X ", pkt_data[i]);
		if ((i & 0xF) == 0xF)
			printf("\n");
	}
	printf("\n");*/
	
	//printf("mac_header:\n");
	//printf("\tdest_addr: ");
	
	/* retireve the position of the ip header */
	ih = (ip_header*)(pkt_data + sizeof(mac_header)); //length of ethernet header
	/*printf("ip_header\n");
	printf("\t%-10s: %02X\n", "ver_ihl", ih->ver_ihl);
	printf("\t%-10s: %02X\n", "tos", ih->tos);
	printf("\t%-10s: %04X\n", "tlen", ntohs(ih->tlen));
	printf("\t%-10s: %04X\n", "identification", ntohs(ih->identification));
	printf("\t%-10s: %04X\n", "flags_fo", ntohs(ih->flags_fo));
	printf("\t%-10s: %02X\n", "ttl", ih->ttl);
	printf("\t%-10s: %02X\n", "proto", ih->proto);
	printf("\t%-10s: %04X\n", "crc", ntohs(ih->crc));
	printf("\t%-10s: %08X\n", "op_pad", ntohs(ih->op_pad));
	printf("\t%-10s: ", "saddr:");*/
}
