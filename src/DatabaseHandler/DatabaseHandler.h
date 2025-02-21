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

    // SELECT tablename FROM pg_catalog.pg_tables WHERE schemaname = 'public';
    int SELECT_ALL_TABLES_SQL_QUERY(const std::string &outputFileNamePath) const;

    // SELECT * FROM *tableName*;
    int SELECT_ALL_SQL_QUERY(const std::string &tableName, const std::string &outputFilePath) const;

    // SELECT (*a*,*b*,*c*) FROM *tableName*;
    int SELECT_COLUMNS_SQL_QUERY(const std::string &tableName, const std::string &outputFilePath) const;

    // INSERT INTO *tableName* (*a*,*b*,*c*) VALUES (*a.a*,*b.b*,*c.c*);
    int INSERT_SQL_QUERY(const std::string &tableName) const;

    // UPDATE *tableName* SET *a* = ... WHERE *b* = ...;
    int UPDATE_SQL_QUERY(const std::string &tableName) const;

    // DELETE FROM *tableName* WHERE *a* = ...;
    int DELETE_SQL_QUERY(const std::string &tableName) const;

    int EXECUTE_SQL_QUERY() const;

    // CREATE TABLE *tableName* (*a..*,*b..*,*c..*); TODO: add validateCredentials()
    int CREATE_TABLE_SQL_QUERY(const std::string &tableName) const;

    // TRUNCATE TABLE *tableName* RESTART IDENTITY CASCADE;
    int TRUNCATE_SQL_QUERY(const std::string &tableName) const;

    // DROP TABLE IF EXISTS *tableName* CASCADE;
    int DROP_TABLE_SQL_QUERY(const std::string &tableName) const;

    // DROP DATABASE IF EXISTS *tableName*;
    int DROP_DATABASE_SQL_QUERY(const std::string &databaseName) const;

    static std::string readTableName();
};
