#pragma once

#include "zmq.h"
#include "common.h"
#include <string>
#include <thread>

using std::string;
using std::thread;

namespace EveTrex {


class MsgHeader
{
public:
    const static string Idle;
    const static string WriteMonster;
};

class Peer
{

};

class Server
{
public:
    Server(string address, uint16 port);
    ~Server();

    void run();
    void stop();
    void waitToFinish();

private:
    void serve();
    void processReceivedMessage(zmq_msg_t* msg);
    bool processReceivedHeader(zmq_msg_t* msg);

    bool isRead(const char* header);
    bool isWrite(const char* header);
    void getSendData(zmq_msg_t* msg);

    string  _address;
    uint16  _port;
    bool    _running;
    void*   _context;
    thread* _thread;
};

}