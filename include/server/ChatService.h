#ifndef CHAT_PROJECT_CHATSERVICE_H
#define CHAT_PROJECT_CHATSERVICE_H

#include <unordered_map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <mutex>
#include "json.hpp"

#include "UserModel.h"

using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

// Indicates the type of event callback method that handles the message
using MsgHandler = std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

// Chat Server Service Class: Using Singleton Pattern
class ChatService {
public:
    // Interface function to get the singleton object
    static ChatService* instance();

    // Handling of login operations
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // Handling of registration operations
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // One-to-one chat operations
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // Friend adding operations
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // Group chat creation operations
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // Group joining operations
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // Group chatting operations
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // Handling of logout operations
    void logout(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // Handling client exception exits
    void clientCloseException(const TcpConnectionPtr &conn);

    // Service reset method to handle server exception
    void reset();

    // Get corresponding handler from message
    MsgHandler getHandler(int msgid);

    // Get subscribed messages from the redis message queue
    void handleRedisSubscribeMessage(int, string);

private:
    ChatService();

    // Storing message ids and their corresponding service handlers
    std::unordered_map<int, MsgHandler> _msgHandlerMap;

    // Storing communication connections for online users
    std::unordered_map<int, TcpConnectionPtr> _userConnMap;

    // Define a mutual exclusion lock to ensure thread-safety of _userConnMap
    std::mutex _connMutex;

    // Data operation object
    UserModel _userModel;
};

#endif//CHAT_PROJECT_CHATSERVICE_H
