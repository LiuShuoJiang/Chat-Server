#include "ChatService.h"
#include "Public.h"
#include <muduo/base/Logging.h>
using namespace muduo;

ChatService *ChatService::instance() {
    static ChatService service;
    return &service;
}

ChatService::ChatService() {
    // Registration of callbacks for handling events related to basic service management for users
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    //_msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    //_msgHandlerMap.insert({LOGOUT_MSG, std::bind(&ChatService::logout, this, _1, _2, _3)});

    // Registration of callbacks for event processing related to group operations management
    //_msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    //_msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    //_msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    // Connect to Redis server

}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int id = js["id"].get<int>();
    std::string pwd = js["password"];

    User user = _userModel.query(id);

    if (user.getId() == id && user.getPassword() == pwd) {
        if (user.getState() == "online") {
            // This user is already logged in, repeat logins are not allowed
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "This account has been online. Input another one!";
            conn->send(response.dump());

        } else {
            // Login successful, record user connection information
            {
                std::lock_guard<std::mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }

            // Subscribe channel(id) to redis after successful login of "id" user


            // Update user status info: offline -> online
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();


            conn->send(response.dump());
        }

    } else {
        // If the use doesn't exist or the user exists but the password is invalid, login failed
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "ID or password is invalid!";
        conn->send(response.dump());
    }
}

void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    std::string name = js["name"];
    std::string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPassword(pwd);
    bool state = _userModel.insert(user);

    if (state) {
        // Registration success
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    } else {
        // Registration failure
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

void ChatService::reset() {
    // set the "online" user to "offline"
    _userModel.resetState();
}

MsgHandler ChatService::getHandler(int msgid) {
    auto it = _msgHandlerMap.find(msgid);
    // Logging of errors: if msgid does not have a corresponding event handler callback
    if (it == _msgHandlerMap.end()) {
        // Returns a default handler, empty operation
        return [=](const TcpConnectionPtr &, json &, Timestamp) {
            LOG_ERROR << "Message ID: " << msgid << " cannot find handler!";
        };
    } else {
        return _msgHandlerMap[msgid];
    }
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn) {
    User user;
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it) {
            if (it->second == conn) {
                // Remove user connection information from the map table
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    // User logout, which is the equivalent of going offline: unsubscribe channel in redis


    // Update user status info
    if (user.getId() != -1) {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int toId = js["toid"].get<int>();
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        auto it = _userConnMap.find(toId);
        if (it != _userConnMap.end()) {
            // if toId is online, then forward the message
            it->second->send(js.dump());
            return;
        }
    }

    // check if toId is online
    User user = _userModel.query(toId);
    if (user.getState() == "online") {

        return;
    }

    // toId is offline, store offline message

}
