#pragma once
#include <libpq-fe.h>


class DbConnection {
    PGconn *connection = nullptr;

public:
    DbConnection();

    PGconn *getConnection() const;

    // Read from ENV variable the path for the .txt file for displaying the Db tables
    static const char *getSelectTablesFilePath();

    // Read from ENV variable the path for the .txt file for displaying the SELECT queries
    static const char *getSelectQueryFilePath();

    ~DbConnection();
};
