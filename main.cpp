#include <iostream>
#include <libpq-fe.h>
#include <cstdlib>
#include "DbConnection.h"
#include "DatabaseHandler.h"

#define TABLES_OUTPUT_FILE "TABLES_OUTPUT_FILE"
#define SELECT_OUTPUT_FILE_ENV "SELECT_OUTPUT_FILE"


int main() {
    const char *tablesOutputFileEnv = std::getenv(TABLES_OUTPUT_FILE);
    if (tablesOutputFileEnv == nullptr) {
        std::cerr << "Error: Environment variable " << TABLES_OUTPUT_FILE << " is not set.\n";
        return 1;
    }

    const char *selectOutputFileNameEnv = std::getenv(SELECT_OUTPUT_FILE_ENV);
    if (selectOutputFileNameEnv == nullptr) {
        std::cerr << "Error: Environment variable " << SELECT_OUTPUT_FILE_ENV << " is not set.\n";
        return 1;
    }

    PGconn* connection = DbConnection::getConnection();
    if (connection == nullptr)
        return 1;

    DatabaseHandler database_handler{connection};

    // SQL operations
    /******************************************************************************************************************/
    const std::string tableName = DatabaseHandler::readTableName();

    // database_handler.INSERT_SQL_QUERY(tableName);

    // database_handler.UPDATE_SQL_QUERY(tableName);

    // database_handler.DELETE_SQL_QUERY(tableName);

    // database_handler.CREATE_TABLE_SQL_QUERY(tableName);

    // database_handler.TRUNCATE_SQL_QUERY(tableName);

    // database_handler.DROP_TABLE_SQL_QUERY(tableName);

    // std::string databaseName;
    // std::cout << "Enter database name:";
    // std::cin >> databaseName;

    // database_handler.DROP_DATABASE_SQL_QUERY(databaseName);

    // database_handler.EXECUTE_SQL_QUERY();

    database_handler.SELECT_ALL_SQL_QUERY(tableName, selectOutputFileNameEnv);

    // database_handler.SELECT_COLUMNS_SQL_QUERY(tableName, selectOutputFileNameEnv);

    // database_handler.SELECT_ALL_TABLES_SQL_QUERY(tableName);

    PQfinish(connection);

    return 0;
}