#pragma once
#include <libpq-fe.h>


class DbConnection {
public:
    static PGconn * getConnection();
};