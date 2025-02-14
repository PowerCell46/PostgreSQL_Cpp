#include <iostream>
#include <libpq-fe.h>
#include <sstream>
#include <cstdlib>

#define POSTGRE_SQL_PORT std::string("5432")
#define POSTGRE_SQL_DB_NAME std::string("working_project_db")
#define DB_CONNECTION_STRING

#define BETWEEN_ROWS_SEPARATOR '.'
#define TABLE_ROW_SEPARATOR '-'
#define TABLE_COL_SEPARATOR '|'


std::string repeat(const char& ch, const std::string::size_type& times);


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

    /******************************************************************************************************************/

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
        // printf("Get %d has %d fields\n", PQntuples(queryResult), PQnfields(queryResult));

        // Initialize a vector(dynamic arr) for every column, calculate the max length of every col, and add a padding to the elements to be centered
        // Be careful with the even odd (how they will be positioned)

        std::string::size_type tableWidthChars = 1;
        // Print the Column Names
        std::cout << "| ";
        for (int i = 0; i < PQnfields(queryResult); i++) {
            std::string currentColumnName{PQfname(queryResult, i)};
            tableWidthChars += currentColumnName.size() + 3;
            std::cout << currentColumnName << " | ";
        }
        std::cout << '\n' << TABLE_COL_SEPARATOR << repeat(TABLE_ROW_SEPARATOR, tableWidthChars - 2) << TABLE_COL_SEPARATOR << '\n';

        // Print the entries in the Table
        for (int i = 0; i < PQntuples(queryResult); i++) {
            std::cout << "| ";
            for (int j = 0; j < PQnfields(queryResult); j++) {
                std::string currentValue{PQgetvalue(queryResult, i, j)};
                std::cout << currentValue << " | ";
            }
            std::cout << '\n' << TABLE_COL_SEPARATOR << repeat(BETWEEN_ROWS_SEPARATOR, tableWidthChars - 2) << TABLE_COL_SEPARATOR << '\n';
            // last one should be skipped
        }
        std::cout << repeat(TABLE_ROW_SEPARATOR, tableWidthChars) << std::endl;
    }

    PQclear(queryResult);

    return 0;
}


std::string repeat(const char& ch, const std::string::size_type& times) {
    std::stringstream strStream{};

    for (int i = 0; i < times; ++i)
        strStream << ch;

    return strStream.str();
}