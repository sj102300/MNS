#pragma once
#include "Packet.hpp"
#include <string>

// Port: 10001(발사 명령), 10002(발사 수행), 10003(격추 성공), 10004(자폭), 10005(비상 폭파)

class IReceiver {
public:

    virtual ~IReceiver() = default;

    virtual bool init(const std::string& address, int port) = 0;

    virtual void start() = 0;

    virtual void receive() = 0;

    virtual void handleData(const char* data, int size) = 0; // 여기서는 받은 데이터를 처리해주는데 

    virtual void close() = 0;
};
