
#define CONNECTION_REQ  0x74160001
#define CONNECTION_RSP  0x74160001 + 0x1000

#define REPORT_DATA_REQ 0x74160002
#define REPORT_DATA_RSP 0x74160002 + 0x1000

#define PUSH_DATA_REQ   0x74160003
#define PUSH_DATA_RSP   0x74160003 + 0x1000


#define MAX_FILE_NAME   128

struct stMsg{
	unsigned int ulMsgId;
	unsigned int ulValue[4];        // Reserved
	char        cName[MAX_FILE_NAME];
};

int msg_send_func(unsigned int ulMsgId, char *strMsg, unsigned int ul1stValue, unsigned int ul2ndValue);
gboolean udp_test_thread(GIOChannel *source, GIOCondition condition, gpointer data);
void *udp_thread_start(void*);
