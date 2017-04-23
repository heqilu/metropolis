#pragma once

#include "common.h"
#include "zmq.h"
#include <string>
#include <thread>

using std::string;
using std::thread;

namespace EveTrex {

    class Client
    {
    public:
        Client(string address, uint16 port);
        ~Client();

        void run();
        void stop();
        void waitToFinish();

    private:
        void serve();
        void getSendData(zmq_msg_t* msg);

        string  _address;
        uint16  _port;
        bool    _running;
        void*   _context;
        thread* _thread;
    };

}