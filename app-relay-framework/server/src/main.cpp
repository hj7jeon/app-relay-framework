
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vconf.h>

#include "common.h"
#include <dlog.h>
#include <glib.h>
#include <bluetooth.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_Evas.h>

#undef LOG_TAG
#define LOG_TAG "REMOTE_KEY_FW"

static GMainLoop* gMainLoop = NULL;
static bt_adapter_visibility_mode_e gVisibilityMode = BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE;
static int gSocketFd = -1;
static bt_adapter_state_e gBtState = BT_ADAPTER_DISABLED;
static const char uuid[] = "00001101-0000-1000-8000-00805F9B34FB";

// Lifecycle of this framework
int rkf_initialize_bluetooth(void);
int rkf_finalize_bluetooth_socket(void);
int rkf_finalize_bluetooth(void);
int rkf_listen_connection(void);

// Callbacks
void rkf_received_data_cb(bt_socket_received_data_s *, void *);
void rkf_socket_connection_state_changed_cb(int, bt_socket_connection_state_e, bt_socket_connection_s *, void *);
void rkf_state_changed_cb(int, bt_adapter_state_e, void *);
gboolean timeout_func_cb(gpointer);



#define VCONF_PLAYER_SHUFFLE	"db/private/org.tizen.music-player/shuffle"
#define VCONF_PLAYER_PROGRESS	"memory/private/org.tizen.music-player/progress_pos"


void _vconf_noti_callback(keynode_t *node, void* data)
{
	printf("%s:+++\n", __func__);


		struct appdata *ad = (struct appdata *)data;
		char *keyname = vconf_keynode_get_name(node);
	
		printf("key changed: %s\n", keyname);
#if 1
		/*
		if (strcmp(keyname, VCONF_PLAYER_SHUFFLE) == 0)
		{
			bool shuffle = vconf_keynode_get_bool(node);
			printf("shuffle=%d\n", shuffle);
		}
		else if (strcmp(keyname, VCONF_PLAYER_PROGRESS) == 0)
		{
			double progress = vconf_get_dbl(node);
			printf("prgress changed: %ld", progress);
		}
		else if (strcmp(keyname, MP_VCONFKEY_PLAYING_PID) == 0)
		{
			int playing_pid = vconf_keynode_get_int(node);
			if (playing_pid != getpid())
			{
				DEBUG_TRACE("other player activated : [pid:%d]", playing_pid);
				if (ad->player_state == PLAY_STATE_PLAYING) {
					ad->paused_by_other_player = TRUE;
					mp_play_control_play_pause(ad, false);
				}
	
				//mp_minicontroller_destroy(ad);
			}
		}
		*/		
#else
	switch(vconf_keynode_get_type(node))
   {
	  case VCONF_TYPE_INT:
   printf("key = %s, value = %d(int)\n",
	   vconf_keynode_get_name(key), vconf_keynode_get_int(key));
   break;
	  case VCONF_TYPE_BOOL:
   printf("key = %s, value = %d(bool)\n",
	   vconf_keynode_get_name(key), vconf_keynode_get_bool(key));
   break;
	  case VCONF_TYPE_DOUBLE:
   printf("key = %s, value = %f(double)\n",
	   vconf_keynode_get_name(key), vconf_keynode_get_dbl(key));
   break;
	  case VCONF_TYPE_STRING:
   printf("key = %s, value = %s(string)\n",
	   vconf_keynode_get_name(key), vconf_keynode_get_str(key));
   break;
	  default:
   fprintf(stderr, "Unknown Type(%d)\n", vconf_keynode_get_type(key));
   break;
   }
   return;

#endif		

}

Eina_Bool mp_app_mouse_event_cb(void *data, int type, void *event)
{
	printf("TEST\n");
	if (type == ECORE_EVENT_MOUSE_BUTTON_DOWN) {
		printf("ECORE_EVENT_MOUSE_BUTTON_DOWN\n");
	}
	else if (type == ECORE_EVENT_MOUSE_BUTTON_UP) {
		printf("ECORE_EVENT_MOUSE_BUTTON_UP\n");
	}

	return 0;
}

bool initEcore()
{
	printf("initEcore()\n");

	int ret, type;
	Eina_Bool did = EINA_FALSE;
	Ecore_Event_Handler *mouse_down = NULL;
	Ecore_Event_Handler *handler = NULL;
	Ecore_Event *event;

	ret = ecore_init();
	if (ret != 1)
		printf("ecore_init fail\n");

	ecore_event_init();
	type = ecore_event_type_new();
	if (type < 1) 
		printf("type fail\n");

	handler = ecore_event_handler_add(type, mp_app_mouse_event_cb, &did);
	if (!handler) 
		printf("Regi fail 1\n");

	event = ecore_event_add(type, NULL, NULL, NULL);
	if (!event)
		printf("add fail\n");


	mouse_down = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, mp_app_mouse_event_cb, NULL);
	if (!mouse_down)
		printf("Regi fail 2\n");

	printf("%d %d\n", type, ECORE_EVENT_MOUSE_BUTTON_DOWN);

	printf("main_loop_bengin()\n");
	ecore_main_loop_begin();

	ret = ecore_shutdown();
	printf("unreached main_loop_bengin()\n");
}

bool initVconf()
{
	bool res = TRUE;

	printf("%s:+++\n", __func__);
#if 1
	//TODO: vconf function test
	int b_val = 0;
	vconf_get_bool("db/private/org.tizen.music-player/shuffle", &b_val);

	if(b_val) 
		vconf_set_bool("db/private/org.tizen.music-player/shuffle", FALSE);
	else 
		vconf_set_bool("db/private/org.tizen.music-player/shuffle", TRUE);

	printf("b_val=%d\n", !b_val);
#endif

	if (vconf_notify_key_changed(VCONF_PLAYER_SHUFFLE, _vconf_noti_callback, NULL) < 0)
	{
		printf("Error when register callback\n");
		res = FALSE;
	}
	
	printf("%s:---:res=%d\n", __func__, res);
	return res; 
}

bool deinitVconf()
{
	bool res = TRUE;
	
	printf("%s:+++\n", __func__);
	
    vconf_ignore_key_changed(VCONF_PLAYER_SHUFFLE, _vconf_noti_callback);
	
	printf("%s:---:res=%d\n", __func__, res);
	
	return res;
}


int rkf_initialize_bluetooth(const char *device_name) {

	// Initialize bluetooth and get adapter state
	int ret;
	ret = bt_initialize();
	if(ret != BT_ERROR_NONE) {
		ALOGD("Unknown exception is occured in bt_initialize(): %x", ret);
		return -1;
	}

	ret = bt_adapter_get_state(&gBtState);
	if(ret != BT_ERROR_NONE) {
		ALOGD("Unknown exception is occured in bt_adapter_get_state(): %x", ret);
		return -2;
	}

	// Enable bluetooth device manually
	if(gBtState == BT_ADAPTER_DISABLED)
	{
		ALOGE("[%s] bluetooth is not enabled.", __FUNCTION__);
		return -3;
	}
	else
	{
		ALOGI("[%s] BT was already enabled.", __FUNCTION__);
	}

	// Set adapter's name
	if(gBtState == BT_ADAPTER_ENABLED) {
		char *name = NULL;
		ret = bt_adapter_get_name(&name);
		if(name == NULL) {
			ALOGD("NULL name exception is occured in bt_adapter_get_name(): %x", ret);
			return -5;
		}

		if(strncmp(name, device_name, strlen(name)) != 0) {
			if(bt_adapter_set_name(device_name) != BT_ERROR_NONE)
			{   
				if (NULL != name)
					free(name);
				ALOGD("Unknown exception is occured in bt_adapter_set_name : %x", ret);
				return -6;
			}   
		}
		free(name);
	} else {
		ALOGD("Bluetooth is not enabled");
		return -7;
	}

	//  Set visibility as BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE
	if(bt_adapter_get_visibility(&gVisibilityMode, NULL) != BT_ERROR_NONE)
	{
		LOGE("[%s] bt_adapter_get_visibility() failed.", __FUNCTION__);
		return -11; 
	}

	if(gVisibilityMode != BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE)
	{
		if(bt_adapter_set_visibility(BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE, 0) != BT_ERROR_NONE)
		{   
			LOGE("[%s] bt_adapter_set_visibility() failed.", __FUNCTION__);
			return -12; 
		}   
		gVisibilityMode = BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE;
	}
	else
	{
		LOGI("[%s] Visibility mode was already set as BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE.", __FUNCTION__);
	}

	// Connecting socket as a server
	ret = bt_socket_create_rfcomm(uuid, &gSocketFd);
	if(ret != BT_ERROR_NONE) {
		ALOGD("Unknown exception is occured in bt_socket_create_rfcomm(): %x", ret);
		return -8;
	}

	ret = bt_socket_set_connection_state_changed_cb(rkf_socket_connection_state_changed_cb, NULL);
	if(ret != BT_ERROR_NONE) {
		ALOGD("Unknown exception is occured in bt_socket_set_connection_state_changed_cb(): %x", ret);
		return -9;
	}

	ret = bt_socket_set_data_received_cb(rkf_received_data_cb, NULL);
	if(ret != BT_ERROR_NONE) {
		ALOGD("Unknown exception is occured in bt_socket_set_data_received_cb(): %x", ret);
		return -10;
	}

	return 0;
}

int rkf_finalize_bluetooth_socket(void) {
	int ret;
	sleep(5); // Wait for completing delivery
	ret = bt_socket_destroy_rfcomm(gSocketFd);
	if(ret != BT_ERROR_NONE)
	{
		ALOGD("Unknown exception is occured in bt_socket_destory_rfcomm(): %x", ret);
		return -1;
	}

	bt_deinitialize();
	return 0;
}

int rkf_finalize_bluetooth(void) {
	bt_deinitialize();
	return 0;
}

int rkf_listen_connection(void) {
	// Success to get a socket
	int ret = bt_socket_listen_and_accept_rfcomm(gSocketFd, 5);
	switch(ret) {
		case BT_ERROR_NONE:
			{
				// Success to listen and accept a connection from client
				ALOGD("listen successful");
				return 0;
			}
			break;
		case BT_ERROR_INVALID_PARAMETER:
			{
				// Invalid parameter exception
				ALOGD("Invalid parameter exception is occured in bt_socket_listen_and_accept_rfcomm()");
				return -1;
			}
			break;
		default:
			{
				// Unknown exception
				ALOGD("Unknown exception is occured in bt_socket_listen_and_accept_rfcomm(): %x", ret);
				return -2;
			}
	}
}

int gReceiveCount = 0;

// bt_socket_data_received_cb
void rkf_received_data_cb(bt_socket_received_data_s *data, void *user_data) {
	static char buffer[1024];
	char menu_string[]="menu";
	char home_string[]="home";
	char back_string[]="back";

	strncpy(buffer, data->data, 1024);
	buffer[data->data_size] = '\0';
	ALOGD("RemoteKeyFW: received a data!(%d) %s", ++gReceiveCount, buffer);

	// ACTION!
	if(strncmp(buffer, menu_string, strlen(menu_string)) == 0) {
		system("/bin/echo 1 > /sys/bus/platform/devices/homekey/coordinates");
	} else if(strncmp(buffer, home_string, strlen(home_string)) == 0) {
		system("/bin/echo 11 > /sys/bus/platform/devices/homekey/coordinates");
	} else if(strncmp(buffer, back_string, strlen(back_string)) == 0) {
		system("/bin/echo 111 > /sys/bus/platform/devices/homekey/coordinates");
	}

}

// bt_socket_connection_state_changed_cb
void rkf_socket_connection_state_changed_cb(int result, bt_socket_connection_state_e connection_state_event, bt_socket_connection_s *connection, void *user_data) {
	if(result == BT_ERROR_NONE) {
		ALOGD("RemoteKeyFW: connection state changed (BT_ERROR_NONE)");
	} else {
		ALOGD("RemoteKeyFW: connection state changed (not BT_ERROR_NONE)");
	}

	if(connection_state_event == BT_SOCKET_CONNECTED) {
		ALOGD("RemoteKeyFW: connected");
	} else if(connection_state_event == BT_SOCKET_DISCONNECTED) {
		ALOGD("RemoteKeyFW: disconnected");
		g_main_loop_quit(gMainLoop);
	}
}

void rkf_state_changed_cb(int result, bt_adapter_state_e adapter_state, void *user_data) {
	if(adapter_state == BT_ADAPTER_ENABLED) {
		if(result == BT_ERROR_NONE) {
			ALOGD("RemoteKeyFW: bluetooth was enabled successfully.");
			gBtState = BT_ADAPTER_ENABLED;
		} else {
			ALOGD("RemoteKeyFW: failed to enable BT.: %x", result);
			gBtState = BT_ADAPTER_DISABLED;
		}
	}
	if(gMainLoop) {
		ALOGD("It will terminate gMainLoop.", result);
		g_main_loop_quit(gMainLoop);
	}
}

gboolean timeout_func_cb(gpointer data)
{
	ALOGE("timeout_func_cb");
	if(gMainLoop)
	{
		g_main_loop_quit((GMainLoop*)data);
	}
	return FALSE;
}

int main(int argc, char *argv[])
{
	int error, ret = 0;
	const char default_device_name[] = "Tizen-RK";
	const char *device_name = NULL;
	gMainLoop = g_main_loop_new(NULL, FALSE);
	ALOGD("Sever started\n");

	if(argc < 2) {
		char errMsg[] = "No bluetooth device name, so its name is set as default.";
		printf("%s\n", errMsg);
		ALOGW("%s\n", errMsg);
		device_name = default_device_name;
	} else {
		device_name = argv[1];
	}

	// Initialize vconf environments
	initVconf();
	
	// Init ecore
	initEcore();
#if 0	// <--- ignore temporarily
	// Initialize bluetooth
	error = rkf_initialize_bluetooth(device_name);
	if(error != 0) {
		ret = -2;
		goto error_end_without_socket;
	}
	ALOGD("succeed in rkf_initialize_bluetooth()\n");

	// Listen connection
	error = rkf_listen_connection();
	if(error != 0) {
		ret = -3;
		goto error_end_with_socket;
	}
#endif	//#if 0

	// If succeed to accept a connection, start a main loop.
	g_main_loop_run(gMainLoop);

	// Deinitializing vconf
	deinitVconf();

	ALOGI("Server is terminated successfully\n");

error_end_with_socket:
	// Finalized bluetooth
	rkf_finalize_bluetooth_socket();

error_end_without_socket:
	rkf_finalize_bluetooth();
	return ret;
}

//! End of a file
