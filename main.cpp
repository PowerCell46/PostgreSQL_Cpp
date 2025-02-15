#include <iostream>
#include <libpq-fe.h>
#include <sstream>
#include <cstdlib>
#include <fstream>

#define POSTGRE_SQL_PORT std::string("5432")
#define POSTGRE_SQL_DB_NAME std::string("working_project_db")
#define DB_CONNECTION_STRING

#define BETWEEN_ROWS_SEPARATOR '.'
#define TABLE_ROW_SEPARATOR '-'
#define TABLE_COL_SEPARATOR '|'


std::string repeat(const char& ch, const std::string::size_type& times);
std::string addRightPadding(const std::string& valueStr, const std::string::size_type& size);


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
        std::ofstream fileStream{R"(C:\Programming\C++\PostgreSQL_Cpp\SQLresult.txt)"};

        auto* columnWidths = new std::string::size_type[PQnfields(queryResult)]{};

        // Print the Column Names
        for (int i = 0; i < PQnfields(queryResult); i++) {
            std::string currentColumnName{PQfname(queryResult, i)};
            columnWidths[i] = currentColumnName.length();
        }

        // Print the entries in the Table
        for (int i = 0; i < PQntuples(queryResult); i++) {
            for (int j = 0; j < PQnfields(queryResult); j++) {
                std::string currentValue{PQgetvalue(queryResult, i, j)};
                columnWidths[j] = std::max(columnWidths[j], currentValue.length());
            }
        }

        std::string::size_type totalSymbolsSize = 1;
        for (int i = 0; i < PQnfields(queryResult); i++) {
            totalSymbolsSize += columnWidths[i] + 2;
        }
        std::cout << totalSymbolsSize << '\n';

        fileStream << repeat(TABLE_ROW_SEPARATOR, totalSymbolsSize) << '\n' << TABLE_COL_SEPARATOR;

        for (int i = 0; i < PQnfields(queryResult); i++) {
            std::string currentColumnName{PQfname(queryResult, i)};
            fileStream << addRightPadding(currentColumnName, columnWidths[i]) << " |";
        }

        fileStream << '\n' << TABLE_COL_SEPARATOR << repeat(BETWEEN_ROWS_SEPARATOR, totalSymbolsSize - 2) << TABLE_COL_SEPARATOR << '\n' << TABLE_COL_SEPARATOR;

        for (int i = 0; i < PQntuples(queryResult); i++) {
            for (int j = 0; j < PQnfields(queryResult); j++) {
                std::string currentValue{PQgetvalue(queryResult, i, j)};
                fileStream << addRightPadding(currentValue, columnWidths[j]) << " |";
            }
            fileStream << '\n' << TABLE_COL_SEPARATOR;
        }
        fileStream << repeat(BETWEEN_ROWS_SEPARATOR, totalSymbolsSize - 2) << TABLE_COL_SEPARATOR << '\n';

        delete columnWidths;
        fileStream.close();
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


std::string addRightPadding(const std::string& valueStr, const std::string::size_type& size) {
    if (valueStr.length() == size)
        return valueStr;

    std::stringstream resultStream{};
    resultStream << valueStr;
    for (int i = 0; i < size - valueStr.length(); ++i)
        resultStream << ' ';

    return resultStream.str();
}