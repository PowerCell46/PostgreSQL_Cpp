#include <iostream>
#include <libpq-fe.h>
#include "src/DatabaseHandler/DatabaseHandler.h"
#include "src/DbConnection/DbConnection.h"


int main() {
    const char *selectTablesOutputFileEnv = DbConnection::getSelectTablesFilePath();
    const char *selectQueryFileNameEnv = DbConnection::getSelectQueryFilePath();

    const DbConnection dbConnection = DbConnection();
    PGconn* connection = dbConnection.getConnection();

    if (connection == nullptr || selectTablesOutputFileEnv == nullptr || selectQueryFileNameEnv == nullptr)
        return 1;

    DatabaseHandler database_handler{connection};

    // SQL operations
    /*************************************************************/

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

    database_handler.SELECT_ALL_SQL_QUERY(tableName, selectQueryFileNameEnv);

    // database_handler.SELECT_COLUMNS_SQL_QUERY(tableName, selectQueryFileNameEnv);

    // database_handler.SELECT_ALL_TABLES_SQL_QUERY(selectTablesOutputFileEnv);

    return 0;
}