#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <wifi.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <tizen_error.h>

#define UDP_PORT    0x8000
#define MAXLINE    1024
#define MAX_FILE_NAME   128

#define CONNECTION_REQ  0x74160001
#define CONNECTION_RSP  0x74160001 + 0x1000

#define REPORT_DATA_REQ 0x74160002
#define REPORT_DATA_RSP 0x74160002 + 0x1000

#define PUSH_DATA_REQ   0x74160003
#define PUSH_DATA_RSP   0x74160003 + 0x1000

struct stMsg{
	unsigned int ulMsgId;
	unsigned int ulValue[4];        // Reserved
	char        cName[MAX_FILE_NAME];
};

int socket_fd;

int msg_send_func(unsigned int ulMsgId, char *strMsg, unsigned int ul1stValue, unsigned int ul2ndValue)
{
	struct stMsg stSendBuf;
	struct sockaddr_in destaddr;
	int addrlen = sizeof(destaddr); 

	memset(&stSendBuf, 0x0, sizeof(stSendBuf));

	stSendBuf.ulMsgId = htonl(ulMsgId);
	strcpy(stSendBuf.cName, strMsg);
	stSendBuf.ulValue[0] = htonl(ul1stValue);
	stSendBuf.ulValue[1] = htonl(ul2ndValue);

	// Sender
    destaddr.sin_family = AF_INET;
    destaddr.sin_addr.s_addr = inet_addr("115.145.178.127"); 
    destaddr.sin_port = htons(UDP_PORT);

	if((sendto(socket_fd, &stSendBuf, sizeof(struct stMsg), 0, (struct sockaddr *)&destaddr, addrlen)) < 0) 
	{
		perror("sendto fail");
		exit(0);
	}
}

gboolean udp_test_thread(GIOChannel *source, GIOCondition condition, gpointer data)
{
	int rv,icnt;
	char a[10];

	printf("Event received from stdin\n");

	rv = read(0, a, 10);
	if (a[0] == '\n' || a[0] == '\r') {
		printf("==================================\n\r");
    	printf("1) Send Connection Req\n\r");
    	printf("2) Send Report Data  Req\n\r");
		printf("q) Send Report Data  Req\n\r");
    	printf("==================================\n\r");
		printf("ENTER  - Show options menu.......\n");
	}

	switch (a[0]) 
	{
		case '1':
			printf("Send Connenction Req Message \n\r");
			msg_send_func(CONNECTION_REQ, "", 0, 0);
			break;
			
		case '2':
			printf("Send Report Data Req Message \n\r");
			msg_send_func(REPORT_DATA_REQ, "/abc/hyojin.mp3", 1234, 87);
			break;

		case 'q':
			return TRUE;
			break;
			
		default:
			break;
	}

	return TRUE;
}

void *udp_thread_start(void*)
{
	char RecvBuf[MAXLINE];
	struct stMsg *stUdpMsg;
	int nbyte;
	struct sockaddr_in cliaddr;
	int icnt, iMode;
	struct sockaddr_in srcaddr;
	int addrlen = sizeof(srcaddr);

	if((socket_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket fail");
        exit(0);
    }

	// Receiver 
	srcaddr.sin_family = AF_INET;
	srcaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srcaddr.sin_port = htons(UDP_PORT);

	if(bind(socket_fd, (struct sockaddr *)&srcaddr, addrlen) < 0) {
	    perror("bind fail");
	    exit(0);
	}

	while(1)
	{
		printf("\n\rNow waiting...\n\r");
	
		memset(RecvBuf, 0, sizeof(RecvBuf));
	
		nbyte = recvfrom(socket_fd, RecvBuf, MAXLINE , 0, (struct sockaddr *)&cliaddr, (socklen_t *)&addrlen);
		if(nbyte< 0) 
	    {
	        perror("recvfrom fail");
	        exit(1);
	    }

		stUdpMsg = (struct stMsg *)RecvBuf;

		printf("\n\r");
		printf("Message ID : 0x%08X \n\r", ntohl(stUdpMsg->ulMsgId));
		printf("File Name  : [%s] \n\r", stUdpMsg->cName);
		printf("1st  Value : %d \n\r", ntohl(stUdpMsg->ulValue[0]));
		printf("2nd  Value : %d \n\r", ntohl(stUdpMsg->ulValue[1]));
		printf("3rd  Value : %d \n\r", ntohl(stUdpMsg->ulValue[2]));
		printf("4th  Value : %d \n\r", ntohl(stUdpMsg->ulValue[3]));
	}

	close(socket_fd);
}
	

