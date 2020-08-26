// //
// // Author: Phong Vu
// //
// #ifndef __ESP_RC_H__
// #define __ESP_RC_H__

// #include <esp8266wifi.h>
// #include <espnow.h>
// #include <ticker.h>

// u8 broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// typedef std::function<void()> esp_rc_cb_t;
// typedef std::function<void(u8*, u8)> esp_rc_callback_t;

// struct esp_rc_listener_t {
// 	String prefix;
// 	esp_rc_cb_t callback;
// };

// struct esp_rc_event_t {
// 	String prefix;
// 	esp_rc_callback_t callback;
// };
// extern struct esp_rc_class esp_rc;

// struct esp_rc_class {
// 	esp_rc_event_t events[100];
// 	u8 events_num = 0;
// 	void begin() {
// 		if (esp_now_init() == OK) {
// 			esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
// 			if (esp_now_is_peer_exist(broadcast))
// 				esp_now_del_peer(broadcast);
// 			esp_now_add_peer(broadcast, ESP_NOW_ROLE_COMBO, 0, 0, 0);
// 			esp_now_register_send_cb([](u8 *sender, u8 err) {
// 				Serial.println("sent");
// 			});
// 			esp_now_register_recv_cb([](u8 *sender, u8 *data, u8 size) {
// 				auto i = 0;
// 				while (i < esp_rc.events_num) {
// 					esp_rc_event_t e = esp_rc.events[i];
// 					u8 offset = e.prefix.length();
// 					if (esp_rc.equals(data, (u8*)e.prefix.c_str(), offset)) {
// 						e.callback(&data[offset], size - offset);
// 					}
// 					i++;
// 				}
// 			});
// 		}
// 	}
// 	void send(String data) {
// 		esp_now_send(broadcast, (u8*)data.c_str(), data.length());
// 	}
// 	void on(String prefix, esp_rc_callback_t callback) {
// 		events[events_num++] = esp_rc_event_t { prefix, callback };
// 	}
// 	bool equals(u8 *a, u8 *b, u8 size, u8 offset = 0) {
// 		for (auto i = offset; i < offset + size; i++) {
// 			if (a[i] != b[i]) {
// 				return false;
// 			}
// 		}
// 		return true;
// 	}
// } esp_rc;




// class EspRCClass;

// extern EspRCClass EspRC;

// class EspRCClass {
// public:
// Ticker loop;
// 	esp_rc_listener_t listeners[255];
// 	u8 _listeners_num;
// 	u8 *_sender;
// 	u8 *_data;
// 	u8 _size;
// 	u32 _sentTime;
// 	bool _isSending;

// 	HardwareSerial *_serial;

// 	void bridge(HardwareSerial *port) {
// 		_serial = port;
// 		_serial->begin(921600);
// 	}
// 	u8 _buffer[255];
// 	void update() {
// 		if (_serial->available()) {
// 			u16 size = _serial->readBytesUntil('\n', _buffer, 255);
// 			esp_now_send(broadcast, _buffer, size);
// 			onMessage(NULL, _buffer, size);
// 			_isSending = true;
// 			_sentTime = micros();
// 		}
// 	}
// 	u8 getSize() {
// 		return _size;
// 	}
// 	u8 *getBytes() {
// 		return _data;
// 	}
// 	String getValue() {
// 		String d = "";
// 		for (auto i = 0; i < _size; i++) d.concat((const char)_data[i]);
// 		return d;
// 	}
// 	void send(String data, u8 *receiver = NULL) {
// 		if (_isSending) return;
// 		esp_now_send(receiver, (u8 *)data.c_str(), (u8)data.length());
// 		_isSending = true;
// 		_sentTime = micros();
// 	}
// 	void send(String data, String value) {
// 		send(data + value);
// 	}
// 	void send(String data, double value) {
// 		send(data + String(value));
// 	}
// 	void on(String prefix, esp_rc_cb_t callback) {
// 		listeners[_listeners_num++] = {prefix, callback};
// 	}
// 	void end() {
// 		esp_now_del_peer(broadcast);
// 		esp_now_unregister_recv_cb();
// 		esp_now_deinit();
// 	}
// 	void begin(u8 channel = 0) {
// 		if (WiFi.getMode() == WIFI_OFF) {
// 			WiFi.mode(WIFI_STA);
// 		}
// 		if (esp_now_init() == OK) {
// 			esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
// 			if (esp_now_is_peer_exist(broadcast))
// 				esp_now_del_peer(broadcast);
// 			esp_now_add_peer(broadcast, ESP_NOW_ROLE_COMBO, channel, 0, 0);
// 			esp_now_register_send_cb(onSend);
// 			esp_now_register_recv_cb(onMessage);
// 		}
// 	}

// 	static bool equals(u8 *a, u8 *b, u8 size, u8 offset = 0) {
// 		for (auto i = offset; i < offset + size; i++) {
// 			if (a[i] != b[i]) {
// 				return false;
// 			}
// 		}
// 		return true;
// 	}

//    private:
// 	esp_now_send_cb_t onSend = [](u8 *receiver, u8 err) {
// 		EspRC._isSending = false;
// 	};
// 	esp_now_recv_cb_t onMessage = [](u8 *sender, u8 *data, u8 size) {
// 		if (EspRC._serial) {
// 			for (auto i = 0; i < size; i++)
// 				EspRC._serial->write(data[i]);
// 			EspRC._serial->print('\n');
// 		}
// 		for (auto i = 0; i < EspRC._listeners_num; i++) {
// 			esp_rc_listener_t *listener = &EspRC.listeners[i];
// 			u8 *prefix = (u8 *)listener->prefix.c_str();
// 			u8 len = listener->prefix.length();
// 			if (equals(prefix, data, len)) {
// 				if (listener->callback) {
// 					EspRC._data = data + len;
// 					EspRC._size = size - len;
// 					listener->callback();
// 				}
// 			}
// 		}
// 	};
// } EspRC;

// #endif  //__ESP_RC_H__
