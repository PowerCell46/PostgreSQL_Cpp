#include "iostream"
#include "DatabaseHandler.h"
#include "fstream"
#include "sstream"
#include "vector"
#include "limits"

#define BETWEEN_ROWS_SEPARATOR '.'
#define TABLE_ROW_SEPARATOR '-'
#define TABLE_COL_SEPARATOR '|'
#define END_OF_COL_SEPARATOR " |"
#define COMMA_SPACE_SEPARATOR std::string(", ")
#define ESCAPE std::string("esc")
#define EMPTY_VALUE std::string("N/A")
#define NO_COLUMN_FOUND(colName) (std::string("No column found with name ") + (colName) + std::string(".\n"))


std::string repeat(const char &, const std::string::size_type &);

std::string addRightPadding(const std::string &, const std::string::size_type &);

std::string createBetweenRowsRow(const std::string::size_type *, const int &);

std::string join(const std::string *, const int &, const std::string &);

std::string join(const std::vector<std::string> &, const std::string &);

std::vector<const char *> generateInsertParamValues(const std::vector<std::string> &);

bool isSqlDateFormatValid(const std::string &);

bool stringValueDoesNotContainInvalidChars(const std::string &);


DatabaseHandler::DatabaseHandler(PGconn *connection): connection(connection) {
}

int DatabaseHandler::SELECT_ALL_SQL_QUERY(const std::string &tableName, const std::string &outputFileNameEnv) const {
    const std::string selectQuery =
            std::string("SELECT * FROM ") + tableName + std::string(";");

    PGresult *queryResult = nullptr;

    queryResult = PQexec(connection, selectQuery.c_str());

    if (PQresultStatus(queryResult) != PGRES_TUPLES_OK) {
        // Not successful SQL query
        fprintf(stderr, "%s[%d]: Select failed: %s\n",
                __FILE__, __LINE__, PQresultErrorMessage(queryResult));
        return 1;
    }

    std::ofstream fileStream{outputFileNameEnv};

    auto *columnWidths = new std::string::size_type[PQnfields(queryResult)]{};

    // Calculate the width of every column (Table Col Names)
    for (int i = 0; i < PQnfields(queryResult); i++) {
        std::string currentColumnName{PQfname(queryResult, i)};
        columnWidths[i] = currentColumnName.length();
    }

    // Calculate the width of every column (Table Row Cols)
    for (int i = 0; i < PQntuples(queryResult); i++) {
        for (int j = 0; j < PQnfields(queryResult); j++) {
            std::string currentValue{PQgetvalue(queryResult, i, j)};
            columnWidths[j] = std::max(columnWidths[j], currentValue.length());
        }
    }

    // Calculate the total char number of a row
    std::string::size_type totalSymbolsSize = 1;
    for (int i = 0; i < PQnfields(queryResult); i++)
        totalSymbolsSize += columnWidths[i] + 2;

    // Print the table Head
    fileStream << repeat(TABLE_ROW_SEPARATOR, totalSymbolsSize) << '\n' << TABLE_COL_SEPARATOR;

    // Print the table Column names
    for (int i = 0; i < PQnfields(queryResult); i++) {
        std::string currentColumnName{PQfname(queryResult, i)};
        fileStream << addRightPadding(currentColumnName, columnWidths[i]) << END_OF_COL_SEPARATOR;
    }

    // Print the first in between row
    fileStream << '\n' << createBetweenRowsRow(columnWidths, PQnfields(queryResult)) << '\n';

    // Print the Table contents
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
    std::cout << "SELECT operation was successful.\n";

    delete columnWidths;

    fileStream.close();
    PQclear(queryResult);

    return 0;
}

int DatabaseHandler::INSERT_SQL_QUERY(const std::string &tableName) const {
    const std::string selectQuery =
            std::string("SELECT * FROM ") + tableName + std::string(" LIMIT 1;");

    PGresult *queryResult = nullptr;
    queryResult = PQexec(connection, selectQuery.c_str());

    if (PQresultStatus(queryResult) != PGRES_TUPLES_OK) {
        // Not successful SQL query
        fprintf(stderr, "%s[%d]: Select failed: %s\n",
                __FILE__, __LINE__, PQresultErrorMessage(queryResult));
        return 1;
    }

    const int tableColumnsNumber = PQnfields(queryResult); // Number of Columns of the Table
    auto *tableNames = new std::string[tableColumnsNumber]{}; // Dynamic arr with the table names
    std::vector<std::string> insertValues; // Vector with the values
    bool skipId = false;

    for (int i = 0; i < tableColumnsNumber; i++) {
        // Read the column value and keep it
        std::string currentColumnName{PQfname(queryResult, i)};
        if (currentColumnName == "id") {
            // Skip id (Most times it's Serial)
            skipId = true;
            continue;
        }

        tableNames[i] = currentColumnName; // Add the column name to the arr
        std::string currentInsertValue = readColumnValue(PQftype(queryResult, i), currentColumnName, connection);
        // Add the read data
        if (currentInsertValue == EMPTY_VALUE) {
            std::cout << "INSERT failed: Cannot insert one or more of the values.\n";
            delete[] tableNames;
            PQclear(queryResult);
            return 1;
        }
        insertValues.push_back(currentInsertValue);
    }

    std::stringstream insertQueryStream{}; // Start to build the query
    insertQueryStream << "INSERT INTO " << tableName
            << " (" << join(tableNames, tableColumnsNumber, COMMA_SPACE_SEPARATOR)
            << ") VALUES (";

    for (int i = 0; i < tableColumnsNumber; i++) {
        // Add placeholders for the dynamic data to the query
        if (skipId && i == 0)
            continue;

        insertQueryStream << '$' << (skipId ? i : i + 1);
        if (i < tableColumnsNumber - 1)
            insertQueryStream << COMMA_SPACE_SEPARATOR;
    }
    insertQueryStream << ");";

    PGresult *insertResult = PQexecParams(
        connection,
        insertQueryStream.str().c_str(),
        tableColumnsNumber - (skipId ? 1 : 0),
        nullptr,
        generateInsertParamValues(insertValues).data(),
        nullptr,
        nullptr,
        0
    );

    if (PQresultStatus(insertResult) != PGRES_COMMAND_OK) {
        // Problem with the Insertion of data
        std::cerr << "INSERT failed: " << PQresultErrorMessage(insertResult) << std::endl;
    } else {
        std::cout << "INSERT operation was successful.\n";
    }

    delete[] tableNames;
    PQclear(queryResult);

    return 0;
}

int DatabaseHandler::UPDATE_SQL_QUERY(const std::string &tableName) const {
    const std::string selectQuery =
            std::string("SELECT * FROM ") + tableName + std::string(" LIMIT 1;");

    PGresult *queryResult = nullptr;
    queryResult = PQexec(connection, selectQuery.c_str());

    if (PQresultStatus(queryResult) != PGRES_TUPLES_OK) {
        // Not successful SQL query
        fprintf(stderr, "%s[%d]: Select failed: %s\n",
                __FILE__, __LINE__, PQresultErrorMessage(queryResult));
        return 1;
    }

    const char *paramValues[2];
    std::string updateColumn, updateValue{};

    std::cout << "Enter the name of the Column you wish to update:"; // Prompt the user to enter column
    std::cin >> updateColumn; // Read the Update column

    for (int i = 0; i < PQnfields(queryResult); i++) {
        const std::string currentColumnName{PQfname(queryResult, i)};

        if (currentColumnName == updateColumn) {
            // Read different type of data type
            updateValue = readColumnValue(PQftype(queryResult, i), currentColumnName, connection);
            if (updateValue == EMPTY_VALUE) {
                std::cout << "UPDATE failed: Cannot update one or more of the values.\n";
                PQclear(queryResult);
                return 1;
            }
            break;
        }
    }

    if (updateValue.empty()) {
        // TODO: Conflict if a diff data type returned ""
        std::cout << NO_COLUMN_FOUND(updateColumn);
        return 1;
    }

    std::string updateWhereColumn, updateWhereValue{};

    std::cout << "Enter the name of the column for the WHERE clause:"; // Prompt the user to enter column
    std::cin >> updateWhereColumn;

    for (int i = 0; i < PQnfields(queryResult); i++) {
        const std::string currentColumnName{PQfname(queryResult, i)};

        if (currentColumnName == updateWhereColumn) {
            // Read different type of data type
            updateWhereValue = readColumnValue(PQftype(queryResult, i), currentColumnName, connection);
            if (updateWhereValue == EMPTY_VALUE) {
                std::cout << "UPDATE failed: Cannot update one or more of the values.\n";
                PQclear(queryResult);
                return 1;
            }
            break;
        }
    }

    if (updateWhereValue.empty()) {
        // TODO: Conflict if a diff data type returned ""
        std::cout << NO_COLUMN_FOUND(updateWhereColumn);
        return 1;
    }

    paramValues[0] = updateValue.c_str();
    paramValues[1] = updateWhereValue.c_str();
    const std::string updateQuery =
            std::string("UPDATE ") + tableName + std::string(" SET ") +
            updateColumn + std::string(" = $1 WHERE ") + updateWhereColumn + std::string(" = $2;");

    PGresult *updateResult = PQexecParams(
        connection,
        updateQuery.c_str(),
        2, // Number of parameters
        nullptr, // Parameter types (NULL = infer from query)
        paramValues, // Parameter values
        nullptr, // Parameter lengths (NULL = assume text)
        nullptr, // Parameter formats (NULL = assume text)
        0 // Result format: 0 for text, 1 for binary
    );

    if (PQresultStatus(updateResult) != PGRES_COMMAND_OK) {
        std::cerr << "UPDATE failed: " << PQerrorMessage(connection) << std::endl;
    } else {
        std::cout << "UPDATE operation was successful.\n";
    }

    PQclear(updateResult);
    return 0;
}

int DatabaseHandler::DELETE_SQL_QUERY(const std::string &tableName) const {
    const std::string selectQuery =
            std::string("SELECT * FROM ") + tableName + std::string(" LIMIT 1;");

    PGresult *queryResult = nullptr;
    queryResult = PQexec(connection, selectQuery.c_str());

    if (PQresultStatus(queryResult) != PGRES_TUPLES_OK) {
        // Not successful SQL query
        fprintf(stderr, "%s[%d]: Select failed: %s\n",
                __FILE__, __LINE__, PQresultErrorMessage(queryResult));
        return 1;
    }

    const char *paramValues[1];
    std::string deleteByColumn, deleteValue{};

    std::cout << "Enter the name of the column by you wish to delete:"; // Prompt the user to enter Delete column
    std::cin >> deleteByColumn; // Read the Delete by column

    for (int i = 0; i < PQnfields(queryResult); i++) {
        const std::string currentColumnName{PQfname(queryResult, i)};

        if (currentColumnName == deleteByColumn) {
            // Read different type of data type
            std::cout << "Enter " << currentColumnName << " value:";
            deleteValue = readColumnValue(PQftype(queryResult, i), currentColumnName, connection);
            if (deleteValue == EMPTY_VALUE) {
                std::cout << "DELETE failed: Cannot delete one or more of the values.\n";
                PQclear(queryResult);
                return 1;
            }
            break;
        }
    }

    if (deleteValue.empty()) {
        std::cout << NO_COLUMN_FOUND(deleteByColumn);
        return 0;
    }

    paramValues[0] = deleteValue.c_str();
    const std::string deleteQuery =
            std::string("DELETE FROM ") + tableName +
            std::string(" WHERE ") + deleteByColumn +
            std::string(" = $1;");

    PGresult *deleteResult = PQexecParams(
        connection,
        deleteQuery.c_str(),
        1, // Number of parameters
        nullptr, // Parameter types (NULL = infer from query)
        paramValues, // Parameter values
        nullptr, // Parameter lengths (NULL = assume text)
        nullptr, // Parameter formats (NULL = assume text)
        0 // Result format: 0 for text, 1 for binary
    );

    if (PQresultStatus(deleteResult) != PGRES_COMMAND_OK) {
        std::cerr << "DELETE failed: " << PQerrorMessage(connection) << std::endl;
    } else {
        std::cout << "DELETE operation was successful.\n";
    }

    PQclear(deleteResult);
    return 0;
}

int DatabaseHandler::TRUNCATE_SQL_QUERY(const std::string &tableName) const {
    const std::string truncateQuery =
            std::string("TRUNCATE TABLE ") + tableName +
            std::string(" RESTART IDENTITY CASCADE;");

    PGresult *truncateResult = PQexec(connection, truncateQuery.c_str());

    if (PQresultStatus(truncateResult) != PGRES_COMMAND_OK) {
        std::cerr << "TRUNCATE failed: " << PQerrorMessage(connection) << std::endl;
        return 1;
    }

    std::cout << "TRUNCATE operation was successful.\n";

    PQclear(truncateResult);
    return 0;
}

int DatabaseHandler::DROP_TABLE_SQL_QUERY(const std::string &tableName) const {
    const std::string dropTableQuery =
            std::string("DROP TABLE IF EXISTS ") + tableName +
            std::string(" CASCADE;");

    PGresult *dropTableResult = PQexec(connection, dropTableQuery.c_str());

    if (PQresultStatus(dropTableResult) != PGRES_COMMAND_OK) {
        std::cerr << "DROP TABLE failed: " << PQerrorMessage(connection) << std::endl;
        return 1;
    }
    std::cout << "DROP TABLE operation was successful.\n";

    PQclear(dropTableResult);
    return 0;
}

int DatabaseHandler::DROP_DATABASE_SQL_QUERY(const std::string &databaseName) const {
    const std::string dropDatabaseQuery =
            std::string("DROP DATABASE IF EXISTS ") + databaseName +
            std::string(";");

    PGresult *dropDatabaseResult = PQexec(connection, dropDatabaseQuery.c_str());

    if (PQresultStatus(dropDatabaseResult) != PGRES_COMMAND_OK) {
        std::cerr << "DROP DATABASE failed: " << PQerrorMessage(connection) << std::endl;
        return 1;
    }

    std::cout << "DROP DATABASE operation was successful.\n";

    PQclear(dropDatabaseResult);
    return 0;
}

int DatabaseHandler::CREATE_TABLE_SQL_QUERY(const std::string &tableName) const {
    std::vector<std::string> columnDefinitions;

    std::cin.ignore();
    while (true) {
        std::cout << "Enter column definition line or '" << ESCAPE << "':";

        std::string currentColumnDefinition;
        std::getline(std::cin, currentColumnDefinition);
        if (currentColumnDefinition == ESCAPE)
            break;

        columnDefinitions.push_back(currentColumnDefinition);
    }

    std::string createTableQuery =
            std::string("CREATE TABLE IF NOT EXISTS ")
            + tableName + std::string(" (")
            + join(columnDefinitions, COMMA_SPACE_SEPARATOR) + std::string(");");

    PGresult *createTableResult = PQexec(connection, createTableQuery.c_str());

    if (PQresultStatus(createTableResult) != PGRES_COMMAND_OK) {
        std::cerr << "CREATE TABLE failed: " << PQerrorMessage(connection) << '\n';
        return 1;
    }

    std::cout << "CREATE TABLE operation was successful.\n";

    PQclear(createTableResult);
    return 0;
}

std::string DatabaseHandler::readColumnValue(const Oid &dataType, const std::string &columnName, PGconn *connection) {
    switch (dataType) {
        case 1043: {
            // VARCHAR
            std::string currentValue;
            while (true) {
                std::cout << "Enter " << columnName << ": ";

                while (std::cin.peek() == '\n')
                    std::cin.ignore();

                std::getline(std::cin, currentValue);

                if (!currentValue.empty()) {
                    if (char *escapedValue = PQescapeLiteral(connection, currentValue.c_str(), currentValue.length())) {
                        std::string safeValue(escapedValue);
                        PQfreemem(escapedValue);
                        return safeValue;
                    }
                }
                std::cout << "Invalid VARCHAR value entered.\n";
            }
        }
        case 23: {
            // INT4
            int currentValue;

            while (true) {
                std::cout << "Enter " << columnName << ":";
                std::cin >> currentValue;

                if (std::cin.fail()) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid INT4 value entered.\n";
                } else {
                    break;
                }
            }

            return std::to_string(currentValue);
        }
        case 1700: {
            // DECIMAL, NUMERIC
            double currentValue;
            while (true) {
                std::cout << "Enter " << columnName << ":";
                std::cin >> currentValue;

                if (std::cin.fail()) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid DECIMAL/NUMERIC value entered.\n";
                } else {
                    break;
                }
            }

            return std::to_string(currentValue);
        }
        case 1082: {
            // DATE
            std::string currentValue;
            while (true) {
                std::cout << "Enter " << columnName << ":";
                std::cin >> currentValue;

                if (isSqlDateFormatValid(currentValue))
                    break;
                std::cout << "Invalid DATE format. Use yyyy-MM-dd.\n";
            }
            return currentValue;
        }
        default: {
            std::cout << "Value Type not supported at the moment.\n";
            return EMPTY_VALUE;
        }
    }
}

std::string DatabaseHandler::readTableName() {
    std::string tableName;
    while (true) {
        std::cout << "Enter Table name:";
        std::getline(std::cin, tableName);

        if (tableName.empty()) {
            std::cout << "Invalid table name.\n";
            continue;
        }

        if (stringValueDoesNotContainInvalidChars(tableName))
            return tableName;

        std::cout << "Invalid table name.\n";
    }
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


std::string join(const std::vector<std::string> &elements, const std::string &separator) {
    if (elements.empty())
        return "";

    std::stringstream resultsStream{};
    for (int i = 0; i < elements.size() - 1; ++i)
        resultsStream << elements.at(i) << separator;
    resultsStream << elements.at(elements.size() - 1);

    return resultsStream.str();
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


bool stringValueDoesNotContainInvalidChars(const std::string &strValue) {
    return strValue
           .find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") == std::string::npos;
}