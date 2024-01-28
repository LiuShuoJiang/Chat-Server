#include "ChatServer.h"
#include "ChatService.h"
#include <iostream>
#include <csignal>
#include <cstdlib>

// Reset user's status information
// at the end of terminating server with CTRL+C
void resetHandler(int) {
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Command is invalid! Example: ./ChatServer 127.0.0.1 6000" << std::endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr(ip, port);

    ChatServer server(&loop, addr, "Chat Server");

    server.start();
    loop.loop();

    return 0;
}
