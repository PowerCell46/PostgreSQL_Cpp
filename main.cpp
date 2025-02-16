#include <iostream>
#include <libpq-fe.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>


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

std::string join(const std::string *, const int &, const std::string &);

std::vector<const char *> generateInsertParamValues(const std::vector<std::string> &);

bool isSqlDateFormatValid(const std::string &);


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

    // Connection
    /******************************************************************************************************************/
    PGconn *connection = PQconnectdb(connectionString.c_str());

    if (PQstatus(connection) != CONNECTION_OK) {
        // Problem with the Connection
        std::cout << "Connection to database failed: " << PQerrorMessage(connection) << std::endl;

        PQfinish(connection);
        return 1;
    }

    // SELECT * FROM pc;
    /******************************************************************************************************************/
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

    // INSERT INTO pc;
    /******************************************************************************************************************/
    // TODO: move to a method, add to a class with SQL methods (header file with .cpp impls)
    #if 0
    PGresult *queryResult = nullptr;
    queryResult = PQexec(connection, "SELECT * FROM pc;");

    if (PQresultStatus(queryResult) != PGRES_TUPLES_OK) {
        // Not successful SQL query
        fprintf(stderr, "%s[%d]: Select failed: %s\n",
                __FILE__, __LINE__, PQresultErrorMessage(queryResult));
    } else {
        auto *tableNames = new std::string[PQnfields(queryResult)]{}; // Dynamic arr with the table names
        std::vector<std::string> insertValues; // Vector with the values
        bool skipId = false;

        for (int i = 0; i < PQnfields(queryResult); i++) {
            // Read the column value and keep it
            std::string currentColumnName{PQfname(queryResult, i)};
            if (currentColumnName == "id") {
                skipId = true;
                continue;
            }
            tableNames[i] = currentColumnName; // Add the column name to the arr

            std::cout << "Enter " << currentColumnName << ':'; // Prompt the user for input

            switch (PQftype(queryResult, i)) {
                // Read different data types
                case 1043: {
                    // VARCHAR
                    if (std::cin.peek() == '\n')
                        std::cin.ignore();

                    std::string currentValue;
                    std::getline(std::cin, currentValue);

                    insertValues.push_back(currentValue);
                    break;
                }
                case 23: {
                    // INT4
                    int currentValue;
                    std::cin >> currentValue;

                    insertValues.push_back(std::to_string(currentValue));
                    break;
                }
                case 1700: {
                    // DECIMAL, NUMERIC
                    double currentValue;
                    std::cin >> currentValue;

                    insertValues.push_back(std::to_string(currentValue));
                    break;
                }
                case 1082: {
                    // DATE
                    // TODO: VALIDATE THE DATE FORMAT yyyy-MM-dd
                    std::string currentValue;
                    std::cin >> currentValue;

                    if (isSqlDateFormatValid(currentValue))
                        insertValues.push_back(currentValue);
                    else
                        std::cout << "Invalid date entered!"; // TODO: No value added ATM to the insert request
                    break;
                }
                default: {
                    // TODO: add other type cases or write default behaviour
                }
            }
        }

        std::stringstream insertQueryStream{}; // Start to build the query
        insertQueryStream << "INSERT INTO pc (" << join(tableNames, PQnfields(queryResult), ", ") << ") VALUES (";

        for (int i = 0; i < PQnfields(queryResult); i++) {
            if (skipId && i == 0)
                continue;

            insertQueryStream << '$' << (skipId ? i : i + 1);
            if (i < PQnfields(queryResult) - 1)
                insertQueryStream << ", ";
        }
        insertQueryStream << ");";

        std::cout << insertQueryStream.str() << "\n";

        PGresult *insertResult = PQexecParams(
            connection,
            insertQueryStream.str().c_str(),
            PQnfields(queryResult) - (skipId ? 1 : 0),
            NULL,
            generateInsertParamValues(insertValues).data(),
            NULL,
            NULL,
            0
        );

        if (PQresultStatus(insertResult) != PGRES_COMMAND_OK) {
            // Problem with the Insertion of data
            std::cerr << "INSERT failed: " << PQresultErrorMessage(insertResult) << std::endl;

        } else {
            std::cout << "Record inserted successfully into " << "pc" << "!" << std::endl;
        }

        delete[] tableNames;
    }

    PQfinish(connection);
    #endif

    // DELETE FROM pc;
    /******************************************************************************************************************/
    // TODO: move to a method, add to a class with SQL methods (header file with .cpp impls)
#if 0
    PGresult *queryResult = nullptr;
    queryResult = PQexec(connection, "SELECT * FROM pc;");

    if (PQresultStatus(queryResult) != PGRES_TUPLES_OK) {
        // Not successful SQL query
        fprintf(stderr, "%s[%d]: Select failed: %s\n",
                __FILE__, __LINE__, PQresultErrorMessage(queryResult));

    } else {
        const char* paramValues[1];
        std::string deleteByColumn, deleteValue{};

        std::cout << "Enter EXACT name of the column by you wish to delete:"; // Prompt the user to enter delete column
        std::cin >> deleteByColumn; // Read the delete by column

        for (int i = 0; i < PQnfields(queryResult); i++) {
            const std::string currentColumnName{PQfname(queryResult, i)};

            if (currentColumnName == deleteByColumn) { // Read different type of data type
                std::cout << "Enter " << currentColumnName << " value:";

                switch (PQftype(queryResult, i)) {
                    case 1043: { // VARCHAR
                        std::string currentValue;

                        if (std::cin.peek() == '\n')
                            std::cin.ignore();
                        std::getline(std::cin, currentValue);

                        deleteValue = currentValue;
                        break;
                    }
                    case 23: { // INT4
                        int currentValue;
                        std::cin >> currentValue;

                        deleteValue = std::to_string(currentValue);
                        break;
                    }
                    case 1700: { // DECIMAL, NUMERIC
                        double currentValue;
                        std::cin >> currentValue;

                        deleteValue = std::to_string(currentValue);
                        break;
                    }
                    case 1082: { // DATE
                        std::string currentValue;
                        std::cin >> currentValue;

                        if (isSqlDateFormatValid(currentValue))
                            deleteValue = currentValue;
                        else
                            // TODO: nothing happens ATM
                        break;
                    }
                    default: {
                        // TODO: add other type cases or write default behaviour
                    }
                }
                break;
            }
        }

        if (deleteValue.empty()) {
            std::cout << "No column found with name " << deleteByColumn << "!";
            return 0;
        }

        paramValues[0] = deleteValue.c_str();
        const std::string deleteQuery = std::string("DELETE FROM pc WHERE ") + deleteByColumn + std::string(" = $1;");

        PGresult *deleteResult = PQexecParams(
            connection,
            deleteQuery.c_str(),
            1, // Number of parameters
            NULL, // Parameter types (NULL = infer from query)
            paramValues, // Parameter values
            NULL, // Parameter lengths (NULL = assume text)
            NULL, // Parameter formats (NULL = assume text)
            0 // Result format: 0 for text, 1 for binary
        );

        if (PQresultStatus(deleteResult) != PGRES_COMMAND_OK) {
            std::cerr << "DELETE failed: " << PQerrorMessage(connection) << std::endl;

        } else {
            std::cout << "Record deleted successfully.\n";
        }

        PQclear(deleteResult);

    }
#endif

    // TRUNCATE TABLE ...;
    /******************************************************************************************************************/
    // TODO: move to a method, add to a class with SQL methods (header file with .cpp impls)
#if 0
    std::string tableName;
    std::cout << "Enter Table name:";
    std::cin >> tableName;

    std::string truncateQuery = std::string("TRUNCATE TABLE ") + tableName + std::string(" RESTART IDENTITY CASCADE;");

    PGresult* truncateResult = PQexec(connection, truncateQuery.c_str());

    if (PQresultStatus(truncateResult) != PGRES_COMMAND_OK) {
        std::cerr << "TRUNCATE failed: " << PQerrorMessage(connection) << std::endl;

    } else {
        std::cout << "Table " << tableName << " truncated successfully\n";
    }

    PQclear(truncateResult);
#endif

    // DROP TABLE ...;
    /******************************************************************************************************************/
    // TODO: move to a method, add to a class with SQL methods (header file with .cpp impls)
#if 0
    std::string tableName;
    std::cout << "Enter Table name:";
    std::cin >> tableName;

    const std::string dropTableQuery = std::string("DROP TABLE IF EXISTS ") + tableName + std::string(" CASCADE;");

    PGresult* dropTableResult = PQexec(connection, dropTableQuery.c_str());

    if (PQresultStatus(dropTableResult) != PGRES_COMMAND_OK) {
        std::cerr << "DROP TABLE failed: " << PQerrorMessage(connection) << std::endl;

    } else {
        std::cout << "Table " << tableName << " dropped successfully.\n";
    }

    PQclear(dropTableResult);
#endif

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


std::string join(const std::string *elements, const int &size, const std::string &separator) {
    if (size < 1)
        return "";

    std::stringstream resultStream{};

    for (int i = 0; i < size - 1; ++i) {
        if (elements[i].empty())
            continue;
        resultStream << elements[i] << separator;
    }
    resultStream << elements[size - 1];

    return resultStream.str();
}


std::vector<const char *> generateInsertParamValues(const std::vector<std::string> &insertValues) {
    std::vector<const char *> paramValues;
    paramValues.reserve(insertValues.size());

    for (const auto &value: insertValues)
        paramValues.push_back(value.c_str());

    return paramValues;
}


bool isSqlDateFormatValid(const std::string &dateStr) {
    if (dateStr.length() != 10)
        return false;

    return
            dateStr[4] == '-' && dateStr[7] == '-'
            && dateStr[0] >= '0' && dateStr[0] <= '9'
            && dateStr[1] >= '0' && dateStr[1] <= '9'
            && dateStr[2] >= '0' && dateStr[2] <= '9'
            && dateStr[3] >= '0' && dateStr[3] <= '9'
            && dateStr[5] >= '0' && dateStr[5] <= '9'
            && dateStr[6] >= '0' && dateStr[6] <= '9'
            && dateStr[8] >= '0' && dateStr[8] <= '9'
            && dateStr[9] >= '0' && dateStr[9] <= '9';
}
