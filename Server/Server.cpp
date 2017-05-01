#include "Server.h"
#include <cstdio>
#include <assert.h>

#ifdef _WINDOWS_
#undef min
#undef max
#endif

#include "monster_generated.h"

using namespace MyGame::Sample; // Specified in the schema.

namespace EveTrex {

#define ZMQ_PREPARE_STRING(msg, data, size) \
zmq_msg_init(&msg) && printf("zmq_msg_init: %s\n", zmq_strerror(errno)); \
zmq_msg_init_size (&msg, size + 1) && printf("zmq_msg_init_size: %s\n",zmq_strerror(errno)); \
memcpy(zmq_msg_data(&msg), data, size + 1);

const string MsgHeader::Idle = "*IDLE*";
const string MsgHeader::WriteMonster = "@MONSTER@";

Server::Server(string address, uint16 port) :
    _address(address),
    _port(port),
    _running(false),
    _context(nullptr)
{
    _context = zmq_ctx_new();
}

EveTrex::Server::~Server()
{
    zmq_ctx_destroy(_context);
}

void Server::serve()
{
    void *socket = zmq_socket(_context, ZMQ_REP);
    string fulladd = "tcp://" + _address + ":" + std::to_string(_port);

    int timeout = 5000;
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(socket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));

    int rc = zmq_bind(socket, fulladd.c_str());
    assert(rc == 0);

    _running = true;

    int64_t more;
    size_t more_size = sizeof more;
    while (_running) {
        zmq_msg_t msgHeader, msgData;
        zmq_msg_init(&msgHeader);
        zmq_msg_init(&msgData);

        // All messages are grouped as header and data, but data could be null
        rc = zmq_msg_recv(&msgHeader, socket, 0);
        if (rc != -1)
            processReceivedHeader(&msgHeader);

        zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);

        if (more) {
            rc = zmq_msg_recv(&msgData, socket, 0);
            if (rc != -1)
                processReceivedMessage(&msgData);
        }

        zmq_msg_close(&msgData);
        zmq_msg_close(&msgHeader);
#ifdef DUMP
        if (rc != -1)
            printf("receive[%d] %s\n", rc, buffer);
        else
            printf("receive timeout %d\n", timeout);
#endif

        zmq_msg_t sendMsg;
        getSendData(&sendMsg);
        rc = zmq_msg_send(&sendMsg, socket, 0);
        zmq_msg_close(&sendMsg);
#ifdef DUMP
        if (rc != -1)
            printf("send[%d] %s\n", rc, message.c_str());
        else
            printf("send timeout %d\n", timeout);
#endif
    }

    zmq_close(socket);
}

bool Server::isRead(const char* header)
{
    if (header == nullptr)
        return false;

    if (header[0] == '*')
        return true;

    return false;
}

bool Server::isWrite(const char* header)
{
    if (header == nullptr)
        return false;

    if (header[0] == '@')
        return true;

    return false;
}

void Server::processReceivedMessage(zmq_msg_t* msg)
{
    char* buffer = (char*)zmq_msg_data(msg);
    uint8_t *buffer_pointer = (uint8_t*)buffer;

    auto monster = GetMonster(buffer_pointer);

    auto hp = monster->hp();
    auto mana = monster->mana();
    auto name = monster->name()->c_str();

    printf("received monster %s HP[%d] MANA[%d]\n", name, hp, mana);
}


// return value indicates if there should be more message followed
bool Server::processReceivedHeader(zmq_msg_t * msg)
{
    char* buffer = (char*)zmq_msg_data(msg);
    if (isWrite(buffer))
    {
        if (strncmp(buffer, MsgHeader::WriteMonster.c_str(), MsgHeader::WriteMonster.size()) == 0) {
            printf("Client wants to do %s\n", buffer);
            return true;
        }
    }
    
    printf("Client only wants to %s\n", buffer);
    return false;
}

void Server::getSendData(zmq_msg_t * msg)
{
    string idle("*IDLE*");
    zmq_msg_init_size(msg, idle.size() + 1);
    memcpy(zmq_msg_data(msg), idle.c_str(), idle.size() + 1);
}

void Server::stop()
{
    _running = false;
}

void Server::waitToFinish()
{
    if (_thread)
        _thread->join();
}

void Server::run()
{
    _thread = new thread(&Server::serve, this);
}

}