#pragma once
#include <libpq-fe.h>
#include <string>


class DatabaseHandler {
    PGconn *connection;

    static std::string readColumnValue(const Oid&, const std::string&);
    // std::string readTableName(); // implement
public:
    explicit DatabaseHandler(PGconn * connection);

    int SELECT_ALL_SQL_QUERY(const std::string &, const std::string &) const;

    int INSERT_SQL_QUERY(const std::string &) const;

    int UPDATE_SQL_QUERY(const std::string &) const;

    int DELETE_SQL_QUERY(const std::string &) const;

    int TRUNCATE_SQL_QUERY(const std::string &) const;

    int DROP_TABLE_SQL_QUERY(const std::string &) const;

    int DROP_DATABASE_SQL_QUERY(const std::string &) const;

    int CREATE_TABLE_SQL_QUERY(const std::string &) const;
};