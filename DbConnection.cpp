#include "DbConnection.h"
#include <cstdlib>
#include <fstream>
#include <iostream>

#define POSTGRE_SQL_PORT std::string("5432")
#define POSTGRE_SQL_DB_NAME std::string("working_project_db")

#define POSTGRE_SQL_ADMIN_ENV_NAME "POSTGRE_SQL_ADMIN"
#define POSTGRE_SQL_ADMIN_ENV_PASS "POSTGRE_SQL_PASS"


PGconn *DbConnection::getConnection() {
    const char *userEnv = std::getenv(POSTGRE_SQL_ADMIN_ENV_NAME);
    const char *passEnv = std::getenv(POSTGRE_SQL_ADMIN_ENV_PASS);

    if (userEnv == nullptr || passEnv == nullptr) {
        std::cerr
                << "Error: Environment variables " << POSTGRE_SQL_ADMIN_ENV_NAME << " or "
                << POSTGRE_SQL_ADMIN_ENV_PASS << " are not set.\n";
        return nullptr;
    }

    const std::string connectionString =
            std::string("postgresql://localhost?port=") + POSTGRE_SQL_PORT +
            std::string("&dbname=") + POSTGRE_SQL_DB_NAME +
            std::string("&user=") + std::string(userEnv) +
            std::string("&password=") + std::string(passEnv);

    // Connection
    //***************************************************************//
    PGconn *connection = PQconnectdb(connectionString.c_str());

    if (PQstatus(connection) != CONNECTION_OK) {
        // Problem with the Connection
        std::cout << "Connection to Database failed: " << PQerrorMessage(connection) << '\n';
        PQfinish(connection);

        return nullptr;
    }

    return connection;
}
