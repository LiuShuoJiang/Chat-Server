#ifndef CHAT_PROJECT_GROUPMODEL_H
#define CHAT_PROJECT_GROUPMODEL_H

#include "Group.h"

// Operational interface methods for maintaining group information
class GroupModel {
public:
    // create group
    bool createGroup(Group &group);

    // join group
    void addGroup(int userid, int groupid, std::string role);

    // query group info of the user
    std::vector<Group> queryGroups(int userid);

    // According to the specified groupid, query group userid list (except for userid)
    // mainly used for group chat business, to send messages to other members of the group.
    std::vector<int> queryGroupUsers(int userid, int groupid);
};

#endif//CHAT_PROJECT_GROUPMODEL_H
