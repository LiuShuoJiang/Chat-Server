#ifndef CHAT_PROJECT_USER_H
#define CHAT_PROJECT_USER_H

#include <string>

class User {
public:
    User(int id = -1, const std::string name = "",
         const std::string password = "",
         const std::string state = "offline")
        : id(id), name(name), password(password), state(state) {}

    int getId() const {
        return id;
    }

    void setId(int id) {
        User::id = id;
    }

    const std::string &getName() const {
        return name;
    }

    void setName(const std::string &name) {
        User::name = name;
    }

    const std::string &getPassword() const {
        return password;
    }

    void setPassword(const std::string &password) {
        User::password = password;
    }

    const std::string &getState() const {
        return state;
    }

    void setState(const std::string &state) {
        User::state = state;
    }

protected:
    int id;
    std::string name;
    std::string password;
    std::string state;
};

#endif//CHAT_PROJECT_USER_H
