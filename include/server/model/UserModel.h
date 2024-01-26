#ifndef CHAT_PROJECT_USERMODEL_H
#define CHAT_PROJECT_USERMODEL_H

#include "User.h"

class UserModel {
public:
    // "add" method for User table
    bool insert(User &user);

    // query user info based on user id
    User query(int id);

    // update user status info
    bool updateState(User user);

    // reset user status info
    void resetState();
};

#endif//CHAT_PROJECT_USERMODEL_H
