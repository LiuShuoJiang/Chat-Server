#include "ChatServer.h"
#include "ChatService.h"
#include <iostream>
#include <csignal>

// Reset user's status information
// at the end of terminating server with CTRL+C
void resetHandler(int) {
    ChatService::instance()->reset();
    exit(0);
}

int main() {


    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr("127.0.0.1", 6000);

    ChatServer server(&loop, addr, "Chat Server");

    server.start();
    loop.loop();

    return 0;
}
