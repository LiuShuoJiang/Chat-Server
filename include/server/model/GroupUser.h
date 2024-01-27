#ifndef CHAT_PROJECT_GROUPUSER_H
#define CHAT_PROJECT_GROUPUSER_H

#include "User.h"

// Group user with one more role information.
// It is directly inherited from the User class and reuses the other information of User.
class GroupUser : public User {
public:
    const std::string &getRole() const {
        return role;
    }

    void setRole(const std::string &role) {
        GroupUser::role = role;
    }

private:
    std::string role;
};


#endif//CHAT_PROJECT_GROUPUSER_H
