#ifndef CHAT_PROJECT_MYSQL_H
#define CHAT_PROJECT_MYSQL_H

#include <mysql/mysql.h>
#include <string>

// Database Operation Class
class MySQL {
public:
    // Initialize database connection
    MySQL();

    // Release Database Connection
    ~MySQL();

    // Connect to Database
    bool connect();

    // Update Operation
    bool update(const std::string &sql);

    // Query Operation
    MYSQL_RES *query(const std::string &sql);

    // Get Connection
    MYSQL *getConnection();

private:
    MYSQL *_conn;
};

#endif//CHAT_PROJECT_MYSQL_H
