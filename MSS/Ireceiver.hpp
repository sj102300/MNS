#pragma once
#include "Packet.hpp"
#include <string>

// Port: 10001(�߻� ���), 10002(�߻� ����), 10003(���� ����), 10004(����), 10005(��� ����)

class IReceiver {
public:

    virtual ~IReceiver() = default;

    virtual bool init(const std::string& address, int port) = 0;

    virtual void start() = 0;

    virtual void receive() = 0;

    virtual void handleData(const char* data, int size) = 0; // ���⼭�� ���� �����͸� ó�����ִµ� 

    virtual void close() = 0;
};
