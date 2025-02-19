#pragma once
#include <libpq-fe.h>
#include <string>


class DatabaseHandler {
    PGconn *connection;

    // Read different types of cells, validate and parse them to a str
    static std::string readColumnValue(const Oid &dataType, const std::string &columnName, PGconn *connection);

    // Write to a file a SELECT query result
    static int fileWriteSelectQueryResult(const std::string &outputFileNameEnv, const PGresult *queryResult);

    static std::string readDatabaseIdentifier(const std::string &identifierType);

    bool validateUserCredentials() const;

public:
    explicit DatabaseHandler(PGconn *connection);

    int SELECT_ALL_TABLES_SQL_QUERY(const std::string &outputFileNamePath) const;

    // SELECT * FROM *tableName*;
    int SELECT_ALL_SQL_QUERY(const std::string &tableName, const std::string &outputFilePath) const;

    // SELECT *a*,*b*,*c* FROM *tableName*;
    int SELECT_COLUMNS_SQL_QUERY(const std::string &tableName, const std::string &outputFilePath) const;

    int INSERT_SQL_QUERY(const std::string &tableName) const;

    int UPDATE_SQL_QUERY(const std::string &tableName) const;

    int DELETE_SQL_QUERY(const std::string &tableName) const;

    int EXECUTE_SQL_QUERY() const;

    int CREATE_TABLE_SQL_QUERY(const std::string &tableName) const;

    int TRUNCATE_SQL_QUERY(const std::string &tableName) const;

    int DROP_TABLE_SQL_QUERY(const std::string &tableName) const;

    int DROP_DATABASE_SQL_QUERY(const std::string &databaseName) const;

    static std::string readTableName();
};
