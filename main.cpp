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
#define END_OF_COL_SEPARATOR " |"
#define COMMA_SPACE_SEPARATOR std::string(", ")


std::string repeat(const char &, const std::string::size_type &);

std::string addRightPadding(const std::string &, const std::string::size_type &);

std::string createBetweenRowsRow(const std::string::size_type *, const int &);


int main() {
    const char *userEnv = std::getenv("POSTGRE_SQL_ADMIN");
    const char *passEnv = std::getenv("POSTGRE_SQL_PASS");
    const char *outputFileNameEnv = std::getenv("OUTPUT_FILE");

    if (userEnv == nullptr || passEnv == nullptr) {
        std::cerr << "Error: Environment variables POSTGRE_SQL_ADMIN or POSTGRE_SQL_PASS are not set." << std::endl;
        return 1;
    }

    if (outputFileNameEnv == nullptr) {
        std::cerr << "Error: Environment variable OUTPUT_FILE is not set." << std::endl;
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

    // TODO: move to a method, add to a class with SQL methods (header file with .cpp impls)
#if 0
    PGresult *queryResult = nullptr;
    queryResult = PQexec(connection, "SELECT * FROM pc;");

    if (PQresultStatus(queryResult) != PGRES_TUPLES_OK) { // Not successful SQL query
        fprintf(stderr, "%s[%d]: Select failed: %s\n",
                __FILE__, __LINE__, PQresultErrorMessage(queryResult));

    } else {
        std::ofstream fileStream{outputFileNameEnv};

        auto *columnWidths = new std::string::size_type[PQnfields(queryResult)]{};

        // Calculate the width of every column (table col names)
        for (int i = 0; i < PQnfields(queryResult); i++) {
            std::string currentColumnName{PQfname(queryResult, i)};
            columnWidths[i] = currentColumnName.length();
        }

        // Calculate the width of every column (table row cols)
        for (int i = 0; i < PQntuples(queryResult); i++) {
            for (int j = 0; j < PQnfields(queryResult); j++) {
                std::string currentValue{PQgetvalue(queryResult, i, j)};
                columnWidths[j] = std::max(columnWidths[j], currentValue.length());
            }
        }

        // Calculate the total char number for a row
        std::string::size_type totalSymbolsSize = 1;
        for (int i = 0; i < PQnfields(queryResult); i++) {
            totalSymbolsSize += columnWidths[i] + 2;
        }

        // Print the table Head
        fileStream << repeat(TABLE_ROW_SEPARATOR, totalSymbolsSize) << '\n' << TABLE_COL_SEPARATOR;


        // Print the table Column names
        for (int i = 0; i < PQnfields(queryResult); i++) {
            std::string currentColumnName{PQfname(queryResult, i)};
            fileStream << addRightPadding(currentColumnName, columnWidths[i]) << END_OF_COL_SEPARATOR;
        }

        // Print the first in between row
        fileStream << '\n' << createBetweenRowsRow(columnWidths, PQnfields(queryResult)) << '\n';

        for (int i = 0; i < PQntuples(queryResult); i++) {
            fileStream << TABLE_COL_SEPARATOR;
            for (int j = 0; j < PQnfields(queryResult); j++) {
                std::string currentValue{PQgetvalue(queryResult, i, j)};
                // Print the current row
                fileStream << addRightPadding(currentValue, columnWidths[j]) << END_OF_COL_SEPARATOR;
            }
            // Print the in between row
            fileStream << '\n' << createBetweenRowsRow(columnWidths, PQnfields(queryResult)) << '\n';
        }

        // Print the table tail
        fileStream << repeat(TABLE_ROW_SEPARATOR, totalSymbolsSize) << '\n';

        delete columnWidths;
        fileStream.close();
    }

    PQclear(queryResult);
#endif

// #if 0
    std::string brand, model, processor, storageType, gpu, purchaseDate;
    int ramSize, storageSize;
    double price;

    std::cout << "Enter brand:";
    std::cin >> brand;

    std::cout << "Enter model:";
    std::cin >> model;

    std::cout << "Enter processor:";
    std::cin >> processor;

    std::cout << "Enter RAM size:";
    std::cin >> ramSize;

    std::cout << "Enter Storage size:";
    std::cin >> storageSize;

    std::cout << "Enter Storage type:";
    std::cin >> storageType;

    std::cout << "Enter GPU:";
    std::cin >> gpu;

    std::cout << "Enter price:";
    std::cin >> price;

    std::cout << "Enter purchase date:";
    std::cin >> purchaseDate;

    // Parameterized query
    const char *paramValues[] = {
        brand.c_str(), model.c_str(),
        processor.c_str(), std::to_string(ramSize).c_str(),
        std::to_string(storageSize).c_str(), storageType.c_str(), gpu.c_str(),
        std::to_string(price).c_str(), purchaseDate.c_str()
    };

    PGresult *res = PQexecParams(connection,
                                 "INSERT INTO pc (brand, model, processor, ram_size, storage_size, storage_type, gpu, price, purchase_date) "
                                 "VALUES ($1, $2, $3, $4::int, $5::int, $6, $7, $8::float, $9::date);",
                                 9, // Number of parameters
                                 NULL, // Let PostgreSQL infer parameter types
                                 paramValues,
                                 NULL, NULL, 0 // No binary format
    );

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "INSERT failed: " << PQresultErrorMessage(res) << std::endl;

    } else {
        std::cout << "Record inserted successfully!" << std::endl;
    }

    PQclear(res);
    PQfinish(connection);
// #endif

    return 0;
}


std::string repeat(const char &ch, const std::string::size_type &times) {
    std::stringstream strStream{};

    for (int i = 0; i < times; ++i)
        strStream << ch;

    return strStream.str();
}


std::string addRightPadding(const std::string &valueStr, const std::string::size_type &size) {
    if (valueStr.length() == size)
        return valueStr;

    std::stringstream resultStream{};
    resultStream << valueStr;

    for (int i = 0; i < size - valueStr.length(); ++i)
        resultStream << ' ';

    return resultStream.str();
}


std::string createBetweenRowsRow(const std::string::size_type *columnWidths, const int &colWidthsSize) {
    std::stringstream resultStream{};
    resultStream << TABLE_COL_SEPARATOR;

    for (int i = 0; i < colWidthsSize; ++i)
        resultStream << repeat(BETWEEN_ROWS_SEPARATOR, columnWidths[i] + 1) << TABLE_COL_SEPARATOR;

    return resultStream.str();
}
