#include "ChatServer.h"
#include "ChatService.h"
#include "json.hpp"
#include <functional>
#include <string>
using namespace std::placeholders;
using json = nlohmann::json;

/*
 * loop: event loop
 * listenAddr: IP + port
 * nameArg: name of server
 * */
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop) {
    // Registering callbacks to the server for the creation and disconnection of user connections
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

    // Registering callbacks to the server for user read and write events
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // Set the number of threads on the server side
    // Here we set up 1 IO thread to handle user connections and 3 worker threads to handle event reads and writes
    _server.setThreadNum(4);
}


ChatServer::~ChatServer() = default;


void ChatServer::start() {
    _server.start();
}


void ChatServer::onConnection(const TcpConnectionPtr &conn) {
    if (!conn->connected()) {// If the client is disconnected
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}


void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time) {
    std::string buf = buffer->retrieveAllAsString();
    // Deserialization of data
    json js = json::parse(buf);

    // Through js["msgid"], we can get service handler, and pass: conn, js, time...
    // The goal is to completely decouple the code of the network module from the code of the service module
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    // Execute the corresponding service processing by calling back to the event handler bound to the message
    msgHandler(conn, js, time);
}
