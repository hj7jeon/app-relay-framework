#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vconf.h>

#include "common.h"
#include <dlog.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_Evas.h>

#include <wifi.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <tizen_error.h>

#include "udp_test.h"

#include <pthread.h>
#include <FBase.h> 
#include <FApp.h>

#include <aul.h>
#include <app_manager.h>

#undef LOG_TAG
#define LOG_TAG "APP_RELAY_FW"

#define VCONF_PLAYER_SHUFFLE	"db/private/org.tizen.music-player/shuffle"
#define VCONF_PLAYER_PROGRESS	"memory/private/org.tizen.music-player/progress_pos"

#define VCONFKEY_APP_RELAY	"db/private/org.tizen.menu-screen/app_relay"

#define MUSIC_PLAYER_PKG_NAME "org.tizen.music-player"

static GMainLoop* gMainLoop = NULL;
static int gSocketFd = -1;
static const char uuid[] = "00001101-0000-1000-8000-00805F9B34FB";

void _vconf_noti_callback(keynode_t *node, void* data)
{
	printf("%s:+++\n", __func__);

		struct appdata *ad = (struct appdata *)data;
		char *keyname = vconf_keynode_get_name(node);
		int a=0,b=0;
	
		printf("key changed: %s\n", keyname);
#if 1
		if (strcmp(keyname, VCONFKEY_APP_RELAY) == 0) 
		{
			//Tizen::Base::String uri = L"tel:12345678900";
			//Tizen::Base::String str1 = L"tizen.phone";
			//Tizen::Base::String str2 = L"http://tizen.org/appcontrol/operation/dial";
			//Tizen::App::AppControl* pAc = Tizen::App::AppManager::FindAppControlN(str1, str2);

			//if (pAc) {
			//	printf("Dial OK\n");
			//	pAc->Start(&uri, null, null, null);
			//	delete pAc;
			//}

			// app_manager_open_app(MUSIC_PLAYER_PKG_NAME);
			aul_open_app(MUSIC_PLAYER_PKG_NAME);
			//Tizen::App::AppId id = L"tizen.calculator";
			//Tizen::App::AppManager* pAppManager = Tizen::App::AppManager::GetInstance();

			//pAppManager->LaunchApplication(id, Tizen::App::AppManager::LAUNCH_OPTION_DEFAULT);

			printf("Launch MP3 player\n");

#if 1
			//TODO: get vconf information form Music-Player
			double progress;
			vconf_get_dbl("memory/private/org.tizen.music-player/pos", &progress);
			
			a = (unsigned int)progress;
			b = (unsigned int)((progress - (double)a) * 1000.);
#endif
			//TODO: send message to server with vconf info
			msg_send_func(REPORT_DATA_REQ, "", a, b);
			
		}
		/*
	  else if (strcmp(keyname, VCONF_PLAYER_SHUFFLE) == 0)
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
#if 0
	//TODO: vconf function test
	int b_val = 0;
	vconf_get_bool("db/private/org.tizen.music-player/shuffle", &b_val);

	if(b_val) 
		vconf_set_bool("db/private/org.tizen.music-player/shuffle", FALSE);
	else 
		vconf_set_bool("db/private/org.tizen.music-player/shuffle", TRUE);

	printf("b_val=%d\n", !b_val);

	//if (vconf_notify_key_changed(VCONF_PLAYER_SHUFFLE, _vconf_noti_callback, NULL) < 0)
	//{
	//	printf("Error when register callback\n");
	//	res = FALSE;
	//}
#endif	

	if (vconf_notify_key_changed(VCONFKEY_APP_RELAY, _vconf_noti_callback, NULL) < 0)
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

int gReceiveCount = 0;

static const char *__test_convert_error_to_string(wifi_error_e err_type)
{
	switch (err_type) {
	case WIFI_ERROR_NONE:
		return "NONE";
	case WIFI_ERROR_INVALID_PARAMETER:
		return "INVALID_PARAMETER";
	case WIFI_ERROR_OUT_OF_MEMORY:
		return "OUT_OF_MEMORY";
	case WIFI_ERROR_INVALID_OPERATION:
		return "INVALID_OPERATION";
	case WIFI_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED:
		return "ADDRESS_FAMILY_NOT_SUPPORTED";
	case WIFI_ERROR_OPERATION_FAILED:
		return "OPERATION_FAILED";
	case WIFI_ERROR_NO_CONNECTION:
		return "NO_CONNECTION";
	case WIFI_ERROR_NOW_IN_PROGRESS:
		return "NOW_IN_PROGRESS";
	case WIFI_ERROR_ALREADY_EXISTS:
		return "ALREADY_EXISTS";
	case WIFI_ERROR_OPERATION_ABORTED:
		return "OPERATION_ABORTED";
	case WIFI_ERROR_DHCP_FAILED:
		return "DHCP_FAILED";
	case WIFI_ERROR_INVALID_KEY:
		return "INVALID_KEY";
	case WIFI_ERROR_NO_REPLY:
		return "NO_REPLY";
	case WIFI_ERROR_SECURITY_RESTRICTED:
		return "SECURITY_RESTRICTED";
	}

	return "UNKNOWN";
}

int test_is_activated(void)
{
	int rv = 0;
	bool state = false;

	rv = wifi_is_activated(&state);

	if (rv != WIFI_ERROR_NONE) {
		printf("Fail to get Wi-Fi device state [%s]\n", __test_convert_error_to_string((wifi_error_e)rv));
		return -1;
	}

	printf("Success to get Wi-Fi device state : %s\n", (state) ? "TRUE" : "FALSE");

	return 1;
}

int main(int argc, char *argv[])
{
	int error, ret = 0;
//	const char default_device_name[] = "Tizen-RK";
//	const char *device_name = NULL;
	int rv;
	pthread_attr_t attr;
    pthread_t thread_t;

	pthread_attr_t attr_2;
    pthread_t thread_t_2;

	gMainLoop = g_main_loop_new(NULL, FALSE);
	printf("App Relay Sever started\n");

//	rv = test_is_activated();		// need review after

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&thread_t, &attr, &udp_thread_start, NULL);

#if 0
	GIOChannel *channel = g_io_channel_unix_new(0);
	g_io_add_watch(channel, (GIOCondition)(G_IO_IN|G_IO_ERR|G_IO_HUP|G_IO_NVAL), udp_test_thread, NULL);
#endif	

	// Initialize vconf environments
	initVconf();

	// Init ecore
	initEcore();

	// If succeed to accept a connection, start a main loop.
	g_main_loop_run(gMainLoop);

	// Deinitializing vconf
	deinitVconf();

	ALOGI("Server is terminated successfully\n");

	return ret;
}

//! End of a file
