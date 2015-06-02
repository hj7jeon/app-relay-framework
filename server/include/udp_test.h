
#define MAX_FILE_NAME   128

struct stMsg{
	unsigned int ulMsgId;
	unsigned int ulValue[4];        // Reserved
	char        cName[MAX_FILE_NAME];
};

int msg_send_func(unsigned int ulMsgId, char *strMsg, unsigned int ul1stValue, unsigned int ul2ndValue);
gboolean udp_test_thread(GIOChannel *source, GIOCondition condition, gpointer data);
void *udp_thread_start(void*);
