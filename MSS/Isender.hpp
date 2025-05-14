#pragma once
#include "Packet.hpp"
#include <string>

class Isender{
public:
    virtual ~Isender() = default;

    virtual bool init(const std::string& address, int port) = 0;

    virtual void start() = 0;

    virtual void run() = 0;

    virtual bool send(const char* data, int length) = 0;

    virtual void close() = 0;
};

