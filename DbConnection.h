#pragma once
#include <libpq-fe.h>


class DbConnection {
public:
    static PGconn * getConnection();

    static const char * getSelectTablesFilePath();

    static const char * getSelectQueryFilePath();
};