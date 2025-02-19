#pragma once
#include <libpq-fe.h>
#include <string>


class DatabaseHandler {
    PGconn *connection;

    static std::string readColumnValue(const Oid &dataType, const std::string &columnName, PGconn *connection);

    static int writeSelectQueryResult(const std::string &outputFileNameEnv, const PGresult *queryResult);

    static std::string readDatabaseIdentifier(const std::string &identifierType);

public:
    explicit DatabaseHandler(PGconn *connection);

    int SELECT_ALL_TABLES_SQL_QUERY(const std::string &outputFileNamePath) const;

    int SELECT_ALL_SQL_QUERY(const std::string &tableName, const std::string &outputFileNamePath) const;

    int SELECT_COLUMNS_SQL_QUERY(const std::string &tableName, const std::string &outputFileNamePath) const;

    int INSERT_SQL_QUERY(const std::string &tableName) const;

    int UPDATE_SQL_QUERY(const std::string &tableName) const;

    int DELETE_SQL_QUERY(const std::string &tableName) const;

    int CREATE_TABLE_SQL_QUERY(const std::string &tableName) const;

    int TRUNCATE_SQL_QUERY(const std::string &tableName) const;

    int DROP_TABLE_SQL_QUERY(const std::string &tableName) const;

    int DROP_DATABASE_SQL_QUERY(const std::string &databaseName) const;

    static std::string readTableName();
};
