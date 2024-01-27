#include "FriendModel.h"
#include "MySQL.h"

void FriendModel::insert(int userid, int friendid) {
    char sql[1024] = {0};
    sprintf(sql, "INSERT INTO friend VALUES(%d, %d)", userid, friendid);

    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
}

std::vector<User> FriendModel::query(int userid) {
    char sql[1024] = {0};

    sprintf(sql, "SELECT a.id, a.name, a.state FROM user a INNER JOIN friend b ON b.friendid = a.id WHERE b.userid = %d", userid);

    std::vector<User> vec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            // Put all offline messages for userid into vec and return them
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}
