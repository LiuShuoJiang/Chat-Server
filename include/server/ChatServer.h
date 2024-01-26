#ifndef CHAT_SERVER_CHATSERVER_H
#define CHAT_SERVER_CHATSERVER_H

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
using namespace muduo;
using namespace muduo::net;

// Main Class for Chat Server
class ChatServer {
public:
    // Initializing the ChatServer object
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);

    ~ChatServer();

    // Start the service
    void start();


private:
    // (Core) A callback function that reports connection-related information
    // and specializes in handling user connections and disconnections.
    void onConnection(const TcpConnectionPtr &);

    // (Core) A callback function to report information about read/write events,
    // specializing in handling user read/write events
    void onMessage(const TcpConnectionPtr &, Buffer *, Timestamp);

    TcpServer _server;// Combining TcpServer objects
    EventLoop *_loop; // Pointer to EventLoop event loop objects
};

#endif//CHAT_SERVER_CHATSERVER_H
