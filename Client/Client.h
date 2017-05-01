#pragma once

#include "common.h"
#include "zmq.h"

#ifdef _WINDOWS_
#undef min
#undef max
#endif

#include "monster_generated.h"
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
        // return value tells if builder has some data to send
        bool getSendData(flatbuffers::FlatBufferBuilder& builder, string& instruction);

        string  _address;
        uint16  _port;
        bool    _running;
        void*   _context;
        thread* _thread;
    };

}