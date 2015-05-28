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

#define MAXLINE  511 //�ִ밪 ����
#define BLOCK 255 //BLOCK ������ ����

struct sockaddr_in servaddr;
socklen_t addrlen = sizeof(servaddr); //���� �ּ��� size�� ����

//�޽��� ���� �κ� ó��
void sendMessage(int s, char* buf) {
    if((sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, addrlen)) < 0) {
        perror("sendto fail");
        exit(0);
    }
}

int udp_packet_send()
{
    int s; //socket
    int nbyte;
    char buf[MAXLINE+1], buf2[MAXLINE+1];
    FILE *stream; //���� �����

    //socket ���� 0���� ������ Error
    if((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket fail");
        exit(0);
    }

    //���� �ּ� ����
    memset(&servaddr, 0, addrlen); //bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; //���ͳ� Addr Family
    servaddr.sin_addr.s_addr = inet_addr("115.145.178.127"); //argv[1]���� �ּҸ� ������
    servaddr.sin_port = htons(atoi("3927")); //argv[2]���� port�� ������

        if((stream = fopen("a.txt", "r")) == NULL) { //argv[3]�� ������ open
                printf("Error");
                exit(1);
        }

    //stream ���� �б�
        while(!feof(stream)) {
                buf[0] = '\0'; //buffer�� �ʱ�ȭ
                fgets(buf, BLOCK, stream); //buffer�� BLOCK ��ŭ �о����

                printf("Send : %s\n", buf); //���� �޽��� ���

                sendMessage(s, buf);
        }
        fclose(stream);

    sendMessage(s, "end of file"); //���� ����� ���� �Ϸ� ó��

    //recvfrom ó�� �κ�
        if((stream = fopen("a.txt", "r")) == NULL) {
                printf("Error not File");
                exit(1);
        }

        while(!feof(stream))
        {
                buf2[0] = '\0'; //2��° buffer �ʱ�ȭ
                fgets(buf2, BLOCK, stream); //buf2�� ������ �о� ����
                puts("get Server : waiting request.");
        sendMessage(s, buf);
                if((nbyte = recvfrom(s, buf, MAXLINE, 0,
                                (struct sockaddr *)&servaddr, &addrlen)) < 0) {
                        perror("recvfrom fail");
                        exit(1);
                }
                buf[nbyte] = 0;

                if(strncmp(buf, buf2, BLOCK)) { //���� ���� buf�� buf2�� �� ��
                        printf("Not Match buf : %s\nbuf2 : %s", buf, buf2);
                        fclose(stream);
                        exit(0);
                } else {
                        printf("Match buf : %s\nbuf2 : %s", buf, buf2);
                }

                puts("sendto complete");
        }

        fclose(stream); //stream close
    close(s); //socket close
    return 0;
}

gboolean udp_test_thread(GIOChannel *source, GIOCondition condition, gpointer data)
{
	int rv;
	char a[10];

	printf("Event received from stdin\n");

	rv = read(0, a, 10);

#if 0
	if (rv <= 0 || a[0] == '0') {
		rv = wifi_deinitialize();

		if (rv != WIFI_ERROR_NONE)
			printf("Fail to deinitialize.\n");

		exit(1);
	}
#endif	

	if (a[0] == '\n' || a[0] == '\r') {
		printf("\n\n Network Connection API Test App\n\n");
		printf("Options..\n");
		printf("1 	- Wi-Fi init and set callbacks\n");
		printf("2 	- Wi-Fi deinit(unset callbacks automatically)\n");
		printf("3	- Activate Wi-Fi device\n");
		printf("4 	- Deactivate Wi-Fi device\n");
		printf("5 	- Is Wi-Fi activated?\n");
		printf("6	- Get connection state\n");
		printf("7 	- Get MAC address\n");
		printf("8 	- Get Wi-Fi interface name\n");
		printf("9 	- Scan request\n");
		printf("a 	- Get Connected AP\n");
		printf("b 	- Get AP list\n");
		printf("c 	- Connect\n");
		printf("d 	- Disconnect\n");
		printf("e 	- Connect by wps pbc\n");
		printf("f 	- Forget an AP\n");
		printf("g 	- Set & connect EAP\n");
		printf("h 	- Set IP method type\n");
		printf("i 	- Set Proxy method type\n");
		printf("j 	- Get AP info\n");
		printf("k 	- Scan hidden AP\n");
		printf("0 	- Exit \n");

		printf("ENTER  - Show options menu.......\n");
	}

	switch (a[0]) {
	case '1':
		rv = udp_packet_send();
		break;
#if 0		
	case '2':
		rv = test_wifi_deinit();
		break;
	case '3':
		rv = test_wifi_activate();
		break;
	case '4':
		rv = test_wifi_deactivate();
		break;
	case '5':
		rv = test_is_activated();
		break;
	case '6':
		rv = test_get_connection_state();
		break;
	case '7':
		rv = test_get_mac_address();
		break;
	case '8':
		rv = test_get_interface_name();
		break;
	case '9':
		rv = test_scan_request();
		break;
	case 'a':
		rv = test_get_connected_ap();
		break;
	case 'b':
		rv = test_foreach_found_aps();
		break;
	case 'c':
		rv = test_connect_ap();
		break;
	case 'd':
		rv = test_disconnect_ap();
		break;
	case 'e':
		rv = test_connect_wps();
		break;
	case 'f':
		rv = test_forget_ap();
		break;
	case 'g':
		rv = test_connect_eap_ap();
		break;
	case 'h':
		rv = test_set_ip_method();
		break;
	case 'i':
		rv = test_set_proxy_method();
		break;
	case 'j':
		rv = test_get_ap_info();
		break;
	case 'k':
		rv = test_scan_hidden_ap();
		break;
#endif		
	default:
		break;
	}

	if (rv == 1)
		printf("Operation succeeded!\n");
	else
		printf("Operation filed!\n");

	return TRUE;
}
