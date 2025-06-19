#pragma once
#include <string>
#include "Packet.h"
class ISender {
public:
    virtual ~ISender() = default;

    virtual bool init(const std::string& address, int port) = 0;

    virtual void start() = 0;

    virtual void run() = 0;

    virtual void close() = 0;

};