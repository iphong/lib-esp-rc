#include "Arduino.h"
#include "Stream.h"

class RcProtocol;

class RcTransport {
protected:
    RcProtocol *_protocol;
public:
    RcTransport(RcProtocol *p): _protocol(p) {

    }
    virtual void begin();
    virtual void send();
    virtual void receive();
};

class RcProtocol : public Print
{
protected:
    uint8_t syncWord = 0x81;

    uint8_t _buffer[250];
    size_t _length = 0;
    bool synced = false;

    RcTransport *_transport;

public:
    RcProtocol() {
        memset(_buffer, 0xff, 250);
    }
    void begin() {
        _transport = new RcTransport(this);
    }
    size_t write(uint8_t c) {
        _buffer[_length++] = c;
        return 1;
    }
    void decode() {
        if (!_length) return;
        Serial.println("decode");
        Serial.printf(" %u %u \n", _length, _buffer[0]);
        if (!synced && syncWord != _buffer[0]) return;
        Serial.println("synced");
        if (_length >= 2) {
            uint8_t size = _buffer[1];
            Serial.printf("has size %u \n", size);
            if (_length >= size + 3) {
                uint8_t crc = _buffer[size + 3];
                uint8_t sum = checksum(&_buffer[2], size - 1);
                Serial.printf("has size %02X %02X \n", crc, sum);
                if (sum == crc) {
                    _length = 0;

                }
            }
        }
    }
    void encode(uint8_t *data, uint8_t size) {
        uint8_t sum = 0xff;
        uint8_t payload[size + 3];
        payload[0] = syncWord;
        payload[1] = size + 1;
        for (auto i=0; i<size; i++) {
            sum += data[i];
            payload[i+2] = data[i];
        }
        payload[size + 2] = sum;

        for (auto i=0; i<size+3; i++) {
            Serial.printf("%02X\t", payload[i]);
            write(payload[i]);
        }
        Serial.println();

        decode();
    }
    uint8_t checksum(uint8_t *data, uint8_t size) {
        uint8_t sum = 0xff;
        for (int i=0; i<size; i++) {
            sum += data[i];
        }
        return sum;
    }
};