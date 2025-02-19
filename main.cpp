#include <iostream>
#include <libpq-fe.h>
#include <cstdlib>
#include <fstream>
#include "DatabaseHandler.h"


#define POSTGRE_SQL_PORT std::string("5432")
#define POSTGRE_SQL_DB_NAME std::string("working_project_db")

#define POSTGRE_SQL_ADMIN_ENV_NAME "POSTGRE_SQL_ADMIN"
#define POSTGRE_SQL_ADMIN_ENV_PASS "POSTGRE_SQL_PASS"
#define SELECT_OUTPUT_FILE_ENV "SELECT_OUTPUT_FILE"
#define TABLES_OUTPUT_FILE "TABLES_OUTPUT_FILE"


int main() {
    const char *userEnv = std::getenv(POSTGRE_SQL_ADMIN_ENV_NAME);
    const char *passEnv = std::getenv(POSTGRE_SQL_ADMIN_ENV_PASS);
    const char *selectOutputFileNameEnv = std::getenv(SELECT_OUTPUT_FILE_ENV);
    const char *tablesOutputFileEnv = std::getenv(TABLES_OUTPUT_FILE);

    if (userEnv == nullptr || passEnv == nullptr) {
        std::cerr
                << "Error: Environment variables " << POSTGRE_SQL_ADMIN_ENV_NAME << " or "
                << POSTGRE_SQL_ADMIN_ENV_PASS << " are not set.\n";
        return 1;
    }

    if (selectOutputFileNameEnv == nullptr) {
        std::cerr
                << "Error: Environment variable " << SELECT_OUTPUT_FILE_ENV << " is not set.\n";
        return 1;
    }

    if (tablesOutputFileEnv == nullptr) {
        std::cerr
                << "Error: Environment variable " << TABLES_OUTPUT_FILE << " is not set.\n";
        return 1;
    }

    const std::string connectionString =
            std::string("postgresql://localhost?port=") + POSTGRE_SQL_PORT +
            std::string("&dbname=") + POSTGRE_SQL_DB_NAME +
            std::string("&user=") + std::string(userEnv) +
            std::string("&password=") + std::string(passEnv);

    // Connection
    /******************************************************************************************************************/
    PGconn *connection = PQconnectdb(connectionString.c_str());

    if (PQstatus(connection) != CONNECTION_OK) {
        // Problem with the Connection
        std::cout << "Connection to Database failed: " << PQerrorMessage(connection) << '\n';
        PQfinish(connection);

        return 1;
    }

    DatabaseHandler database_handler{connection};

    // SQL operations
    /******************************************************************************************************************/
    const std::string tableName = DatabaseHandler::readTableName();

    database_handler.INSERT_SQL_QUERY(tableName);

    // database_handler.UPDATE_SQL_QUERY(tableName);

    // database_handler.DELETE_SQL_QUERY(tableName);

    // database_handler.CREATE_TABLE_SQL_QUERY(tableName);

    // database_handler.TRUNCATE_SQL_QUERY(tableName);

    // database_handler.DROP_TABLE_SQL_QUERY(tableName);

    // std::string databaseName;
    // std::cout << "Enter database name:";
    // std::cin >> databaseName;

    // database_handler.DROP_DATABASE_SQL_QUERY(databaseName);

    // database_handler.SELECT_ALL_SQL_QUERY(tableName, selectOutputFileNameEnv);

    // database_handler.SELECT_COLUMNS_SQL_QUERY(tableName, selectOutputFileNameEnv);

    database_handler.SELECT_ALL_TABLES_SQL_QUERY(tableName);

    PQfinish(connection);

    return 0;
}

