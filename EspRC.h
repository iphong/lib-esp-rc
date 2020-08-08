//
// Author: Phong Vu
//
#ifndef __ESP_RC_H__
#define __ESP_RC_H__

#include <espnow.h>

#define ESP_RC_MAX_LISTENER 100

typedef void (*esp_rc_cb_t)();

typedef void (*esp_rc_str_cb_t)(String);
struct esp_rc_listener_t {
	String subscribe;
	esp_rc_cb_t callback;
	esp_rc_str_cb_t str_callback;
};

class EspRCClass;

extern EspRCClass EspRC;

class EspRCClass {
protected:
	esp_rc_listener_t listeners[ESP_RC_MAX_LISTENER];
	uint8_t listener_num = 0;
	uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

public:
	void begin(uint8_t channel = 1) {
		if (esp_now_init() == OK) {
			esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
			if (esp_now_is_peer_exist(EspRC.broadcast))
				esp_now_del_peer(EspRC.broadcast);
			esp_now_add_peer(EspRC.broadcast, ESP_NOW_ROLE_COMBO, channel, 0, 0);
			esp_now_register_recv_cb([](uint8_t *mac, uint8_t *payload, uint8_t size) {
				String msg = "";
				for (auto i = 0; i < size; i++)
					msg.concat((const char) payload[i]);
				EspRC.parse(msg);
			});
		}
	}

	void parse(String msg) {
		for (auto i = 0; i < listener_num; i++) {
			esp_rc_listener_t listener = EspRC.listeners[i];
			if (msg.startsWith(listener.subscribe)) {
				if (listener.callback) {
					listener.callback();
				}
				if (listener.str_callback) {
					String value = msg.substring(listener.subscribe.length(), msg.length());
					value.trim();
					listener.str_callback(value);
				}
			}
		}
	}

	void send(String payload) {
		esp_now_send(EspRC.broadcast, (uint8_t *) payload.c_str(), (uint8_t) payload.length());
	}

	void send(String payload, String value, char delimiter = ' ') {
		send(payload + delimiter + String(value));
	}

	void send(String payload, double value, char delimiter = ' ') {
		send(payload + delimiter + String(value));
	}

	void on(String subscribe, esp_rc_cb_t callback) {
		EspRC.listeners[listener_num++] = {subscribe, callback, NULL};
	}

	void on(String subscribe, esp_rc_str_cb_t callback) {
		EspRC.listeners[listener_num++] = {subscribe, NULL, callback};
	}
} EspRC;

#endif //__ESP_RC_H__

