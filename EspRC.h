//
// Author: Phong Vu
//
#ifndef __ESP_RC_H__
#define __ESP_RC_H__

#include <esp8266wifi.h>
#include <espnow.h>

#define ESP_RC_MAX_LISTENER 100

typedef std::function<void()> esp_rc_cb_t;

struct esp_rc_listener_t {
	String prefix;
	esp_rc_cb_t callback;
};

class EspRCClass;

extern EspRCClass EspRC;


void printhex(u8 *d, u8 len) {
    for (auto i=0; i<len; i++)
        Serial.printf("%02X ", d[i]);
    Serial.print('\n');
}

class EspRCClass {
protected:
	esp_rc_listener_t listeners[ESP_RC_MAX_LISTENER];
	u8 _channel;
	u8 max_listeners = 0;
	u8 broadcast[6] ={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	u8 *_sender;
	u8 *_data;
	u8  _size;
	u32 _sentTime;
	bool _isSending = false;

public:
	static bool equals(u8 *a, u8* b, u8 size, u8 offset = 0) {
		for (auto i=offset; i<offset+size; i++) {
			if (a[i] != b[i]) {
				return false;
			}
		}
		return true;
	}
	u8 getChannel() {
		return _channel;
	}
	u8 * getSender() {
		return _sender;
	}
	String getValue() {
		String d = "";
		for (auto i=0; i<_size; i++) d.concat((const char)_data[i]);
		return d;
	}
	void send(String data, u8* receiver = EspRC.broadcast) {
		if (_isSending) return;
		data = String((const char)EspRC.getChannel() + data);
		esp_now_send(receiver, (u8*)data.c_str(), (u8)data.length());
		_isSending = true;
		_sentTime = micros();
	}
	void send(String data, String value) {
		send(data + value);
	}
	void send(String data, double value) {
		send(data + String(value));
	}
	void on(String prefix, esp_rc_cb_t callback) {
		listeners[max_listeners++] ={
			prefix,
			callback
		};
	}
	void end() {
		esp_now_del_peer(broadcast);
		esp_now_unregister_recv_cb();
		esp_now_deinit();
	}
	void begin(u8 channel = 1) {
		_channel = channel;
		if (WiFi.getMode() == WIFI_OFF) {
			WiFi.mode(WIFI_STA);
		}
		if (esp_now_init() == OK) {
			esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
			if (esp_now_is_peer_exist(EspRC.broadcast))
				esp_now_del_peer(EspRC.broadcast);
			esp_now_add_peer(EspRC.broadcast, ESP_NOW_ROLE_COMBO, 0, 0, 0);

			esp_now_register_send_cb([](u8 *receiver, u8 err) {
				EspRC._isSending = false;
				Serial.println(micros() - EspRC._sentTime);
			});
			esp_now_register_recv_cb([](u8 *sender, u8 *data, u8 size) {
				if (data[0] == EspRC.getChannel()) {
					for (auto i = 0; i < EspRC.max_listeners; i++) {
						esp_rc_listener_t listener = EspRC.listeners[i];
						u8 *prefix = (u8*)listener.prefix.c_str();
						u8 len = listener.prefix.length();
						data++;
						size--;
						if (EspRC.equals(prefix, data, len)) {
							if (listener.callback) {
								EspRC._data = data + len;
								EspRC._size = size - len;
								listener.callback();
							}
						}
					}
				}
				});
		}
	}

} EspRC;

#endif //__ESP_RC_H__

