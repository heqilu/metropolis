#include "Client.h"

using namespace EveTrex;

int main()
{
    auto client = new Client("127.0.0.1", 12000);
    client->run();
    client->waitToFinish();

    delete client;
    return 0;
}