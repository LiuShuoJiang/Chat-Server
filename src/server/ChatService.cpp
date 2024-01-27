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
    _msgHandlerMap.insert({LOGIN_MSG, [this](auto && PH1, auto && PH2, auto && PH3) { login(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); }});
    _msgHandlerMap.insert({REG_MSG, [this](auto && PH1, auto && PH2, auto && PH3) { reg(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); }});
    _msgHandlerMap.insert({ONE_CHAT_MSG, [this](auto && PH1, auto && PH2, auto && PH3) { oneChat(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); }});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, [this](auto && PH1, auto && PH2, auto && PH3) { addFriend(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); }});
    _msgHandlerMap.insert({LOGOUT_MSG, [this](auto && PH1, auto && PH2, auto && PH3) { logout(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); }});

    // Registration of callbacks for event processing related to group operations management
    _msgHandlerMap.insert({CREATE_GROUP_MSG, [this](auto && PH1, auto && PH2, auto && PH3) { createGroup(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); }});
    _msgHandlerMap.insert({ADD_GROUP_MSG, [this](auto && PH1, auto && PH2, auto && PH3) { addGroup(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); }});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, [this](auto && PH1, auto && PH2, auto && PH3) { groupChat(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); }});

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

            // query if the user has offline message
            std::vector<std::string> vec = _offlineMsgModel.query(id);
            if (!vec.empty()) {
                response["offlinemsg"] = vec;
                // Delete all the offline messages of the user after reading the user's offline messages
                _offlineMsgModel.remove(id);
            }

            // query the user's friend info and return
            std::vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty()) {
                std::vector<std::string> friendVec;
                for (User &u : userVec) {
                    json friendJs;
                    friendJs["id"] = u.getId();
                    friendJs["name"] = u.getName();
                    friendJs["state"] = u.getState();
                    friendVec.push_back(friendJs.dump());
                }
                response["friends"] = friendVec;
            }

            // query the user's group info
            std::vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty()) {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                std::vector<std::string> groupV;
                for (Group &group : groupuserVec) {
                    json groupJson;
                    groupJson["id"] = group.getId();
                    groupJson["groupname"] = group.getName();
                    groupJson["groupdesc"] = group.getDesc();

                    std::vector<std::string> userV;
                    for (GroupUser &u: group.getUsers()) {
                        json js;
                        js["id"] = u.getId();
                        js["name"] = u.getName();
                        js["state"] = u.getState();
                        js["role"] = u.getRole();
                        userV.push_back(js.dump());
                    }
                    groupJson["users"] = userV;
                    groupV.push_back(groupJson.dump());
                }

                response["groups"] = groupV;
            }

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
    _offlineMsgModel.insert(toId, js.dump());
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userid = js["id"].get<int>();
    // May need to check if friendid exists (omit here)
    int friendid = js["friendid"].get<int>();

    _friendModel.insert(userid, friendid);
}

void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userid = js["id"].get<int>();
    std::string name = js["groupname"];
    std::string desc = js["groupdesc"];

    // store newly created group info
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group)) {
        // store group creator info
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    std::vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    std::lock_guard<std::mutex> lock(_connMutex);
    for (int id : useridVec) {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end()) {
            it->second->send(js.dump());
        } else {
            User user = _userModel.query(id);
            if (user.getState() == "online") {

            } else {
                // store offline message
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}

void ChatService::logout(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userid = js["id"].get<int>();
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end()) {
            _userConnMap.erase(it);
        }
    }

    // User logout, which is the equivalent of just going offline and unsubscribing from the channel in redis


    // Update user status info
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}
