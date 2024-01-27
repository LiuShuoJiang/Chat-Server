#ifndef CHAT_PROJECT_FRIENDMODEL_H
#define CHAT_PROJECT_FRIENDMODEL_H

#include "User.h"
#include <vector>

// Interface methods for maintaining friend information
class FriendModel {
public:
    // add friend relationship
    void insert(int userid, int friendid);

    // return friend list
    std::vector<User> query(int userid);
};

#endif//CHAT_PROJECT_FRIENDMODEL_H
