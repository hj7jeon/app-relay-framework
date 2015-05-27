
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vconf.h>

#include "common.h"
#include <dlog.h>
#include <glib.h>
#include <bluetooth.h>

#include <wifi.h>

#undef LOG_TAG
#define LOG_TAG "APP_RELAY_FW"

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

int rkf_initialize_bluetooth(const char *device_name) {

#if 1		//Minjin
	int b_val = 0;
	vconf_get_bool("db/private/org.tizen.music-player/shuffle", &b_val);

	if(b_val) 
		vconf_set_bool("db/private/org.tizen.music-player/shuffle", FALSE);
	else 
		vconf_set_bool("db/private/org.tizen.music-player/shuffle", TRUE);
	
	printf("b_val=%d\n", !b_val);
#endif	

#if 0	// By Jeon Hyojin
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
#endif	

	return 0;
}

int rkf_finalize_bluetooth_socket(void) {
	int ret;
	sleep(5); // Wait for completing delivery

#if 0	// Hyojin Jeon	
	ret = bt_socket_destroy_rfcomm(gSocketFd);
	if(ret != BT_ERROR_NONE)
	{
		ALOGD("Unknown exception is occured in bt_socket_destory_rfcomm(): %x", ret);
		return -1;
	}

	bt_deinitialize();
#endif	
	return 0;
}

int rkf_finalize_bluetooth(void) {

	#if 0 
	bt_deinitialize();
	#endif
	
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

	gMainLoop = g_main_loop_new(NULL, FALSE);
	printf("App Relay Sever started\n");

#if 0
	if(argc < 2) {
		char errMsg[] = "No bluetooth device name, so its name is set as default.";
		printf("%s\n", errMsg);
		ALOGW("%s\n", errMsg);
		device_name = default_device_name;
	} else {
		device_name = argv[1];
	}
#endif	

	rv = test_is_activated();

#if 0
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

	// If succeed to accept a connection, start a main loop.
	g_main_loop_run(gMainLoop);

	ALOGI("Server is terminated successfully\n");

error_end_with_socket:
	// Finalized bluetooth
	rkf_finalize_bluetooth_socket();

error_end_without_socket:
	rkf_finalize_bluetooth();

#endif	
	return ret;
}

//! End of a file
