#ifndef CHAT_PROJECT_PUBLIC_H
#define CHAT_PROJECT_PUBLIC_H

// common file for server and client
enum EnMsgType {
    LOGIN_MSG = 1,    // login message
    LOGIN_MSG_ACK,    // login response message
    LOGOUT_MSG,       // logout message
    REG_MSG,          // register message
    REG_MSG_ACK,      // register response message
    ONE_CHAT_MSG,     // chat message
    ADD_FRIEND_MSG,   // add friend message
    CREATE_GROUP_MSG, // create new group
    ADD_GROUP_MSG,    // add to group
    GROUP_CHAT_MSG,   // group chat
};

#endif//CHAT_PROJECT_PUBLIC_H
