#include "GroupModel.h"
#include "MySQL.h"

bool GroupModel::createGroup(Group &group) {
    char sql[1024] = {0};

    sprintf(sql, "INSERT INTO allgroup(groupname, groupdesc) VALUES('%s', '%s')",
            group.getName().c_str(), group.getDesc().c_str());

    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}

void GroupModel::addGroup(int userid, int groupid, std::string role) {
    char sql[1024] = {0};
    sprintf(sql, "INSERT INTO groupuser VALUES(%d, %d, '%s')", groupid, userid, role.c_str());

    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
}

std::vector<Group> GroupModel::queryGroups(int userid) {
    /*
     * 1. first, according to the userid,
     * query the groupuser table
     * to find out the information of the group to which the user belongs.
     * 2. and then according to the group information,
     * query the userid of all users belonging to the group;
     * but also do a multi-table lookup with user table,
     * to find out the user's details
     * */
    char sql[1024] = {0};
    sprintf(sql, "SELECT a.id, a.groupname, a.groupdesc FROM allgroup a INNER JOIN groupuser b ON a.id = b.groupid WHERE b.userid = %d", userid);

    std::vector<Group> groupVec;

    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            // query all the group info of userid
            while ((row = mysql_fetch_row(res)) != nullptr) {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }

    // query group's user info
    for (Group &group: groupVec) {
        sprintf(sql, "SELECT a.id, a.name, a.state, b.grouprole FROM user a INNER JOIN groupuser b ON b.userid = a.id WHERE b.groupid = %d", group.getId());

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }

    return groupVec;
}

std::vector<int> GroupModel::queryGroupUsers(int userid, int groupid) {
    char sql[1024] = {0};
    sprintf(sql, "SELECT userid FROM groupuser WHERE groupid = %d AND userid != %d", groupid, userid);

    std::vector<int> idVec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }

    return idVec;
}
