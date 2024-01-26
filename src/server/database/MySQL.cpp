#include "MySQL.h"
#include <muduo/base/Logging.h>

// Database configuration information
static const std::string server = "127.0.0.1";
static const std::string user = "root";
static const std::string password = "13175407";
static const std::string dbname = "chat";

MySQL::MySQL() {
    _conn = mysql_init(nullptr);
}

MySQL::~MySQL() {
    if (_conn != nullptr) {
        mysql_close(_conn);
    }
}

bool MySQL::connect() {
    MYSQL *p = mysql_real_connect(_conn, server.c_str(),
                                  user.c_str(),
                                  password.c_str(),
                                  dbname.c_str(), 3306,
                                  nullptr, 0);

    if (p != nullptr) {
        // The default encoding character for C and C++ code is ASCII,
        // if it is not set, the Chinese display pulled down from MySQL will be abnormal
        mysql_query(_conn, "set names gbk");
        LOG_INFO << "MySQL connection successful!";
    } else {
        LOG_INFO << "MySQL connection failed!";
    }

    return p;
}

bool MySQL::update(const std::string &sql) {
    if (mysql_query(_conn, sql.c_str())) {
        LOG_INFO << __FILE__ << ": " << __LINE__ << " : " << sql << " update failed!";
        return false;
    }

    return true;
}

MYSQL_RES *MySQL::query(const std::string &sql) {
    if (mysql_query(_conn, sql.c_str())) {
        LOG_INFO << __FILE__ << ": " << __LINE__ << " : " << sql << " query failed!";
        return nullptr;
    }

    return mysql_use_result(_conn);
}

MYSQL *MySQL::getConnection() {
    return _conn;
}
