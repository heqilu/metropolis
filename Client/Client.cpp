#include "Client.h"
#include <assert.h>

using namespace MyGame::Sample; // Specified in the schema.

namespace EveTrex {

Client::Client(string address, uint16 port) :
    _address(address),
    _port(port),
    _running(false),
    _context(nullptr)
{
    _context = zmq_ctx_new();
}

EveTrex::Client::~Client()
{
    zmq_ctx_destroy(_context);
}

void Client::serve()
{
    void *socket = zmq_socket(_context, ZMQ_REQ);
    string fulladd = "tcp://" + _address + ":" + std::to_string(_port);
    int rc = zmq_connect(socket, fulladd.c_str());
    assert(rc == 0);

    int timeout = 2000;
    _running = true;
    while (_running) {
        flatbuffers::FlatBufferBuilder builder(1024);
        string instruction = "*IDLE*";
        bool sendMore = false;
        //sendMore = getSendData(builder, instruction);

        if (sendMore) {
            rc = zmq_send(socket, instruction.data(), instruction.size() + 1, ZMQ_SNDMORE);
            rc = zmq_send(socket, builder.GetBufferPointer(), builder.GetSize() + 1, 0);
        }
        else
            rc = zmq_send(socket, instruction.data(), instruction.size() + 1, 0);

#ifdef DUMP
        if (rc != -1)
            printf("send[%d] %s\n", rc, zmq_msg_data(sendMsg));
        else
            printf("send timeout %d\n", timeout);
#endif

        zmq_pollitem_t items[] = {
            { socket, 0, ZMQ_POLLIN, 0 },
        };
       
        int rc = zmq_poll(items, 1, timeout);
        assert(rc >= 0);

        if (items[0].revents & ZMQ_POLLIN) {
            zmq_msg_t recvMsg;
            zmq_msg_init(&recvMsg);
            zmq_msg_recv(&recvMsg, socket, 0);
            char* buffer = (char*)zmq_msg_data(&recvMsg);
            zmq_msg_close(&recvMsg);
#ifdef DUMP
            if (rc != -1)
                printf("receive[%d] %s\n", rc, buffer);
            else
                printf("receive timeout %d\n", timeout);
#endif
        }
        else
            printf("no message is polled in\n");
    }

    zmq_close(socket);
}

bool Client::getSendData(flatbuffers::FlatBufferBuilder& builder, string& instruction)
{
    auto weapon_one_name = builder.CreateString("Sword");
    short weapon_one_damage = 3;
    auto weapon_two_name = builder.CreateString("Axe");
    short weapon_two_damage = 5;
    // Use the `CreateWeapon` shortcut to create Weapons with all the fields set.
    auto sword = CreateWeapon(builder, weapon_one_name, weapon_one_damage);
    auto axe = CreateWeapon(builder, weapon_two_name, weapon_two_damage);

    auto name = builder.CreateString("Orc");
    unsigned char treasure[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    auto inventory = builder.CreateVector(treasure, 10);

    std::vector<flatbuffers::Offset<Weapon>> weapons_vector;
    weapons_vector.push_back(sword);
    weapons_vector.push_back(axe);
    auto weapons = builder.CreateVector(weapons_vector);

    // Set his hit points to 300 and his mana to 150.
    int hp = 300;
    int mana = 150;
    // Finally, create the monster using the `CreateMonster` helper function
    // to set all fields.
    auto pos = Vec3(1.0f, 2.0f, 3.0f);
    auto orc = CreateMonster(builder, &pos, mana, hp, name,
        inventory, Color_Red, weapons, Equipment_Weapon,
        axe.Union());

    builder.Finish(orc);

    instruction = "@MONSTER@";
    return true;
}

void Client::stop()
{
    _running = false;
}

void Client::waitToFinish()
{
    if (_thread)
        _thread->join();
}

void Client::run()
{
    _thread = new thread(&Client::serve, this);
}

}