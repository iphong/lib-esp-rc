//
// Author: Phong Vu
//
#ifndef __ESP_RC_H__
#define __ESP_RC_H__

#include <esp8266wifi.h>
#include <espnow.h>

#define ESP_RC_MAX_LISTENER 100

typedef void (*esp_rc_cb_t)();
typedef void (*esp_rc_str_cb_t)(String);

struct esp_rc_listener_t {
	String subscribe;
	uint8_t *sender;
	esp_rc_cb_t callback;
	esp_rc_str_cb_t str_callback;
};

class EspRCClass;

extern EspRCClass EspRC;

class EspRCClass {
protected:
	esp_rc_listener_t listeners[ESP_RC_MAX_LISTENER];
	uint8_t _channel;
	uint8_t max_listeners = 0;
	uint8_t broadcast[6] = {
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};
	uint8_t private_key[16] = {
		0xFF, 0xFF, 0xFF, 0xFF, 
		0xAA, 0xFF, 0x55, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 
		0xFF, 0xFF, 0x22, 0xFF
	};
	uint8_t *_sender;
	String _msgBuffer;

public:
	uint8_t getChannel() {
		return _channel;
	}
	uint8_t * getSender() {
		return _sender;
	}
	uint8_t setChannel(uint8_t channel) {
		return _channel = channel;
	}
	void begin(uint8_t channel = 1) {
		_channel = channel;
		if (WiFi.getMode() == WIFI_OFF) {
			WiFi.mode(WIFI_STA);
		}
		if (esp_now_init() == OK) {
			esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
			if (esp_now_is_peer_exist(EspRC.broadcast))
				esp_now_del_peer(EspRC.broadcast);
			esp_now_add_peer(EspRC.broadcast, ESP_NOW_ROLE_COMBO, 0, 0, 0);
			esp_now_register_recv_cb([](uint8_t *mac, uint8_t *payload, uint8_t size) {
				// First byte is channel ID range from 0-255
				// Only handle message sent to the correct channel
				if (payload[0] == EspRC.getChannel()) {
					EspRC._msgBuffer = "";
					EspRC._sender = mac;
					for (auto i = 1; i < size; i++)
						EspRC._msgBuffer.concat((const char) payload[i]);
					EspRC.handle(EspRC._msgBuffer);
				}
			});
		}
	}
	void end() {
		esp_now_del_peer(broadcast);
		esp_now_unregister_recv_cb();
		esp_now_deinit();
	}
	void handle(String msg) {
		for (auto i = 0; i < max_listeners; i++) {
			esp_rc_listener_t listener = EspRC.listeners[i];
			if (msg.startsWith(listener.subscribe)) {
				if (listener.callback) {
					listener.callback();
				}
				if (listener.str_callback) {
					listener.str_callback((String)msg.substring(listener.subscribe.length(), msg.length()));
				}
			}
		}
	}
	void send(String payload) {
		payload = (char)EspRC.getChannel() + payload;
		esp_now_send(EspRC.broadcast, (uint8_t *) payload.c_str(), (uint8_t) payload.length());
	}

	void send(String payload, String value) {
		send(payload + String(value));
	}

	void send(String payload, double value) {
		send(payload + String(value));
	}

	void on(String subscribe, esp_rc_cb_t callback, uint8_t * sender = NULL) {
		EspRC.listeners[max_listeners++] = {subscribe, sender, callback, NULL};
	}

	void on(String subscribe, esp_rc_str_cb_t callback, uint8_t * sender = NULL) {
		EspRC.listeners[max_listeners++] = {subscribe, sender, NULL, callback};
	}
} EspRC;

#endif //__ESP_RC_H__

