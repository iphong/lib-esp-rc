//
// Author: Phong Vu
//
#ifndef __esp_rc_H__
#define __esp_rc_H__

#include <espnow.h>

namespace MeshRC {
	
	typedef std::function<void(u8*, u8)> esp_rc_callback_t;

	struct esp_rc_event_t {
		String prefix;
		esp_rc_callback_t callback;
	} events[250];

	u8 broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	u8 events_num = 0;
	u32 received;
	u32 ignored;
	u8 *sender = NULL;
	u32 sendTime;
	u16 duration;
	u8 *master = NULL;
	u8 buffer[250];
	bool sending;

	void setMaster(u8 *addr) {
		master = addr;
	}
	void send(u8 *data, u8 size) {
		sending = true;
		sendTime = micros();
		esp_now_send(broadcast, data, size);
	}
	void send(String data) {
		send((u8*)data.c_str(), data.length());
	}
	void send(String type, u8 *data, u8 size) {
		memcpy(&buffer[0], (u8*)type.c_str(), type.length());
		memcpy(&buffer[type.length()], data, size);
		send(buffer, type.length() + size);
	}
	void on(String prefix, esp_rc_callback_t callback) {
		events[events_num++] = (esp_rc_event_t){ prefix, callback };
	}
	void all(esp_rc_callback_t callback) {
		return on("", callback);
	}
	void wait() {
		while (MeshRC::sending) yield(); // Wait while sending
	}
	void delayMs(u32 time) {
		u32 delayUntil = millis() + time;
		while (millis() < delayUntil) yield();
	}
	bool equals(u8 *a, u8 *b, u8 size, u8 offset = 0) {
		for (auto i = offset; i < offset + size; i++)
			if (a[i] != b[i])
				return false;
		return true;
	}
	void begin() {
		if (esp_now_init() == OK) {
			if (esp_now_is_peer_exist(broadcast))
				esp_now_del_peer(broadcast);
				esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
				esp_now_add_peer(broadcast, ESP_NOW_ROLE_COMBO, 0, 0, 0);
				esp_now_register_send_cb([](u8 *addr, u8 err) {
					sending = false;
					duration = micros() - sendTime;
				});
			esp_now_register_recv_cb([](u8 *addr, u8 *data, u8 size) {
				if (master == NULL || equals(addr, master, 6)) {
					received++;
					sender = addr;
					for (auto i = 0; i < events_num; i++) {
						u8 offset = events[i].prefix.length();
						if (equals(data, (u8*)events[i].prefix.c_str(), offset)) {
							events[i].callback(&data[offset], size - offset);
						}
					}
				} else {
					ignored++;
				}
			});
		}
	}
}

#endif  //__esp_rc_H__
