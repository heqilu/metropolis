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
    while (_running) {
        zmq_msg_t recvMsg;
        zmq_msg_init(&recvMsg);
        rc = zmq_msg_recv(&recvMsg, socket, 0);
        if (rc != -1)
            processReceivedMessage(&recvMsg);
        zmq_msg_close(&recvMsg);
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