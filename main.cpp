#include <iostream>
#include <libpq-fe.h>
#include <sstream>
#include <cstdlib>

#define POSTGRE_SQL_PORT std::string("5432")
#define POSTGRE_SQL_DB_NAME std::string("working_project_db")
#define DB_CONNECTION_STRING


int main() {
    const char* userEnv = std::getenv("POSTGRE_SQL_ADMIN");
    const char* passEnv = std::getenv("POSTGRE_SQL_PASS");

    if (userEnv == nullptr || passEnv == nullptr) {
        std::cerr << "Error: Environment variables POSTGRE_SQL_ADMIN or POSTGRE_SQL_PASS are not set." << std::endl;
        return 1;
    }

    const std::string connectionString =
            std::string("postgresql://localhost?port=") + POSTGRE_SQL_PORT +
            std::string("&dbname=") + POSTGRE_SQL_DB_NAME +
            std::string("&user=") + std::string(userEnv) +
            std::string("&password=") + std::string(passEnv);

    PGconn *connection = PQconnectdb(connectionString.c_str());

    if (PQstatus(connection) != CONNECTION_OK) {
        // Problem with the Connection
        std::cout << "Connection to database failed: " << PQerrorMessage(connection) << std::endl;

        PQfinish(connection);
        return 1;
    }

    PGresult *queryResult = nullptr;
    queryResult = PQexec(connection, "SELECT * FROM pc;");

    if (PQresultStatus(queryResult) != PGRES_TUPLES_OK) { // Not successful SQL query
        fprintf(stderr, "%s[%d]: Select failed: %s\n",
                __FILE__, __LINE__, PQresultErrorMessage(queryResult));

    } else {
        printf("Get %d has %d fields\n", PQntuples(queryResult), PQnfields(queryResult));

        // Print the Column Names
        for (int i = 0; i < PQnfields(queryResult); i++) {
            printf("%s | ", PQfname(queryResult, i));
        }
        putchar('\n');

        // Print the entries in the Table
        for (int i = 0; i < PQntuples(queryResult); i++) {
            for (int j = 0; j < PQnfields(queryResult); j++) {
                printf("%s | ", PQgetvalue(queryResult, i, j));
            }
            putchar('\n');
        }
    }

    PQclear(queryResult);

    return 0;
}
