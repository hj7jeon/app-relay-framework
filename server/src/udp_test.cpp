
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

#include <dlog.h>
#include <udp_test.h>

#include <vconf.h>
#include <aul.h>
#include <app.h>

#define UDP_PORT    0x8000
#define MAXLINE    1024

#define MUSIC_PLAYER_PKG_NAME "org.tizen.music-player"
#define SOUND_PLAYER_PKG_NAME "org.tizen.sound-player"

int socket_fd;

static void playMusic()
{
	service_h service;
	service_create(&service);
	service_set_operation(service, SERVICE_OPERATION_VIEW);
	service_set_package(service, SOUND_PLAYER_PKG_NAME);
	service_set_uri(service, "file:///opt/usr/media/Downloads/2.mp3");
	if (service_send_launch_request(service, NULL, NULL) == SERVICE_ERROR_NONE) {
		printf("Success\n");
	} else {
		printf("Fail\n");
	}
	service_destroy(service);
}

int msg_send_func(unsigned int ulMsgId, char *strMsg, unsigned int ul1stValue, unsigned int ul2ndValue)
{
	struct stMsg stSendBuf;
	struct sockaddr_in destaddr;
	int addrlen = sizeof(destaddr); 
	FILE *fp;
	char ip_addr[16];

	memset(ip_addr, 0, sizeof(ip_addr));

	fp = fopen("/temp/app_relay_sever_ip.txt", "r");
	fread(ip_addr, 1, 15, fp);
	fclose(fp);

	printf("\n\r Dest Ip Addres is %s \n\r",ip_addr);

	memset(&stSendBuf, 0x0, sizeof(stSendBuf));

	stSendBuf.ulMsgId = htonl(ulMsgId);
	strcpy(stSendBuf.cName, strMsg);
	stSendBuf.ulValue[0] = htonl(ul1stValue);
	stSendBuf.ulValue[1] = htonl(ul2ndValue);

	// Sender
    destaddr.sin_family = AF_INET;
    destaddr.sin_addr.s_addr = inet_addr((const char*)ip_addr); 
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

bool bConnectionDone = FALSE;
unsigned int ulSendCount = 0;

void global_socket_init()
{
	struct sockaddr_in srcaddr;
	int addrlen = sizeof(srcaddr);
	int iMode;

	close(socket_fd);
	
	if((socket_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket fail");
        exit(0);
    }

	// Receiver 
	srcaddr.sin_family = AF_INET;
	srcaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srcaddr.sin_port = htons(UDP_PORT);

	#if 1
    iMode = 1;
    if (ioctl(socket_fd, FIONBIO, &iMode))
    {
        perror("ioctl failed");
        exit(0);
    }
    #endif

	if(bind(socket_fd, (struct sockaddr *)&srcaddr, addrlen) < 0) {
	    perror("bind fail");
	    exit(0);
	}
}


void *udp_thread_start(void*)
{
	char RecvBuf[MAXLINE];
	struct stMsg *stUdpMsg;
	int nbyte;
	struct sockaddr_in cliaddr;
	int addrlen = sizeof(cliaddr);
	int icnt;
	double progress;
	unsigned a,b;

	global_socket_init();

	while(1)
	{
		ALOGI("\n\rNow waiting...\n\r");
	
		memset(RecvBuf, 0, sizeof(RecvBuf));
		nbyte = recvfrom(socket_fd, RecvBuf, MAXLINE , 0, (struct sockaddr *)&cliaddr, (socklen_t *)&addrlen);
		if(nbyte > 0)
		{
			stUdpMsg = (struct stMsg *)RecvBuf;

			printf("\n\r");
			printf("Message ID : 0x%08X \n\r", ntohl(stUdpMsg->ulMsgId));
			printf("1st  Value : %d \n\r", stUdpMsg->ulValue[0]);
			printf("2nd  Value : %d \n\r", stUdpMsg->ulValue[1]);
			printf("3rd  Value : %d \n\r", stUdpMsg->ulValue[2]);
			printf("4th  Value : %d \n\r", stUdpMsg->ulValue[3]);
			printf("File Name  : [%s] \n\r", stUdpMsg->cName);

			switch(ntohl(stUdpMsg->ulMsgId))
			{
				case CONNECTION_RSP:
					printf("Get Connection Rsp Message\n\r");
					printf("Connection complete!!\n\r");
					bConnectionDone = TRUE;
					break;

				case REPORT_DATA_RSP:
					printf("Get Report Rsp Message\n\r");
					break;

				case PUSH_DATA_REQ:
					printf("Get Push Data Req Message\n\r");
					msg_send_func(PUSH_DATA_RSP, "", 0, 0);

					// To Do : process value , Launch Music Player, send Posisition Data
					a = stUdpMsg->ulValue[0];
					b = stUdpMsg->ulValue[1];
					progress = (double)a + ((double)b / 1000.);
					printf("progress is [%8.3f] \n\r", progress);

					vconf_set_dbl("memory/private/org.tizen.music-player/pos", progress);
					/*
					aul_open_app(MUSIC_PLAYER_PKG_NAME);
					printf("start music play\n");
					aul_open_file("/opt/usr/media/Downloads/2.mp3");
					printf("music play\n");
					*/

					playMusic();

					// vconf_set_bool("memory/private/org.tizen.music-player/player_state", 1);

					break;

				default :
					break;
			}
		}

		if(bConnectionDone == FALSE)
		{
			printf("Send Connenction Req Message \n\r");
			msg_send_func(CONNECTION_REQ, "", 0, 0);
			ulSendCount++;
		}

		if(ulSendCount > 5)
		{
			ulSendCount = 0;
			global_socket_init();
		}

		sleep(1);
	}

	close(socket_fd);
}
	

