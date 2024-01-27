#include "OfflineMessageModel.h"
#include "MySQL.h"

void OfflineMessageModel::insert(int userid, const std::string &msg) {
    char sql[1024] = {0};
    sprintf(sql, "INSERT INTO offlinemessage VALUES(%d, '%s')", userid, msg.c_str());

    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
}

void OfflineMessageModel::remove(int userid) {
    char sql[1024] = {0};
    sprintf(sql, "DELETE FROM offlinemessage WHERE userid = %d", userid);

    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
}

std::vector<std::string> OfflineMessageModel::query(int userid) {
    char sql[1024] = {0};
    sprintf(sql, "SELECT message FROM offlinemessage WHERE userid = %d", userid);

    std::vector<std::string> vec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            // Put all offline messages for userid into vec and return them
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                vec.emplace_back(row[0]);
            }
            mysql_free_result(res);
            return vec;
        }
    }

    return vec;
}
