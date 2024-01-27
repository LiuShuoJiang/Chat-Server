#ifndef CHAT_PROJECT_OFFLINEMESSAGEMODEL_H
#define CHAT_PROJECT_OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>

// Provide interface methods for manipulating offline message tables
class OfflineMessageModel {
public:
    // store offline message of users
    void insert(int userid, const std::string &msg);

    // delete offline message of users
    void remove(int userid);

    // query offline message of users
    std::vector<std::string> query(int userid);
};

#endif//CHAT_PROJECT_OFFLINEMESSAGEMODEL_H
