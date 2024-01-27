#ifndef CHAT_PROJECT_GROUP_H
#define CHAT_PROJECT_GROUP_H

#include "GroupUser.h"
#include <string>
#include <vector>

class Group {
public:
    Group(int id = -1, const std::string &name = "", const std::string &desc = "")
        : id(id), name(name), desc(desc) {}

    void setId(int id) {
        Group::id = id;
    }

    void setName(const std::string &name) {
        Group::name = name;
    }

    void setDesc(const std::string &desc) {
        Group::desc = desc;
    }

    int getId() const {
        return id;
    }

    const std::string &getName() const {
        return name;
    }

    const std::string &getDesc() const {
        return desc;
    }

    std::vector<GroupUser> &getUsers() {
        return users;
    }

private:
    int id;
    std::string name;
    std::string desc;
    std::vector<GroupUser> users;
};

#endif//CHAT_PROJECT_GROUP_H
