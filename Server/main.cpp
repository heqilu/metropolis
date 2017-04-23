#include "Server.h"

using namespace EveTrex;

int main()
{
    auto server = new Server("127.0.0.1", 12000);
    server->run();
    server->waitToFinish();

    delete server;

    return 0;
}