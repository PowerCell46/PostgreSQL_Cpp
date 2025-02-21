#include "iostream"
#include "DatabaseHandler.h"
#include "SHA256.h"
#include "fstream"
#include "sstream"
#include "vector"
#include "limits"

#define BETWEEN_ROWS_SEPARATOR '.'
#define TABLE_ROW_SEPARATOR '-'
#define TABLE_COL_SEPARATOR '|'
#define END_OF_COL_SEPARATOR " |"
#define COMMA_SPACE_SEPARATOR std::string(", ")
#define ID_COL_NAME std::string("id")
#define ESCAPE std::string("esc")
#define TABLE std::string("Table")
#define COLUMN std::string("Column")
#define EMPTY_VALUE std::string("N/A")
#define SELECT_TABLE_NAMES_COL_TITLE std::string("Table Name")
#define VARCHAR_CODE_VALUE 1043
#define NO_COLUMN_FOUND(colName) (std::string("No column found with name ") + (colName) + std::string(".\n"))


// Repeat character *n* number of times
std::string repeat(const char &ch, const std::string::size_type &times);

// Adding spaces till the str reaches the size
std::string addRightPadding(const std::string &valueStr, const std::string::size_type &size);

// Create a whole in between entries row for printing the table
std::string createBetweenRowsRow(const std::string::size_type *columnWidths, const int &colWidthsSize);

// Join vector by a separator
std::string join(const std::vector<std::string> &elements, const std::string &separator);

std::vector<const char *> generateInsertParamValues(const std::vector<std::string> &);

// Validate Date str
bool isSqlDateFormatValid(const std::string &dateStr);

// Check whether a str contains invalid characters
bool stringValueDoesNotContainInvalidChars(const std::string &);


DatabaseHandler::DatabaseHandler(PGconn *connection): connection(connection) {
}


int DatabaseHandler::SELECT_ALL_TABLES_SQL_QUERY(const std::string &outputFileNamePath) const {
    std::ofstream fileStream{outputFileNamePath};

    const std::string selectTableNamesQuery
            = "SELECT tablename FROM pg_catalog.pg_tables WHERE schemaname = 'public';";

    PGresult *queryResult = PQexec(connection, selectTableNamesQuery.c_str());

    if (PQresultStatus(queryResult) != PGRES_TUPLES_OK) {
        // Not successful SQL query
        fprintf(stderr, "%s[%d]: Select failed: %s\n",
                __FILE__, __LINE__, PQresultErrorMessage(queryResult));
        return 1;
    }

    auto biggestCharWidth = SELECT_TABLE_NAMES_COL_TITLE.length() + 1;
    // Find the longest str in order to calculate the width of the column
    for (int i = 0; i < PQntuples(queryResult); ++i) {
        std::string tableName(PQgetvalue(queryResult, i, 0));
        if (tableName.length() > biggestCharWidth)
            biggestCharWidth = tableName.length();
    }

    // Table Head
    fileStream << repeat(TABLE_ROW_SEPARATOR, biggestCharWidth + 2) << '\n';

    // Table Column Name
    fileStream
            << TABLE_COL_SEPARATOR << addRightPadding(SELECT_TABLE_NAMES_COL_TITLE, biggestCharWidth)
            << TABLE_COL_SEPARATOR << '\n'
            << TABLE_COL_SEPARATOR << repeat(BETWEEN_ROWS_SEPARATOR, biggestCharWidth)
            << TABLE_COL_SEPARATOR << '\n';

    // Table Data
    for (int i = 0; i < PQntuples(queryResult); ++i) {
        std::string tableName(PQgetvalue(queryResult, i, 0));
        fileStream
                << TABLE_COL_SEPARATOR << addRightPadding(tableName, biggestCharWidth) << TABLE_COL_SEPARATOR << '\n'
                << TABLE_COL_SEPARATOR << repeat(BETWEEN_ROWS_SEPARATOR, biggestCharWidth) << TABLE_COL_SEPARATOR <<
                '\n';
    }

    // Table Tail
    fileStream << repeat(TABLE_ROW_SEPARATOR, biggestCharWidth + 2) << '\n';

    PQclear(queryResult);

    return 0;
}

int DatabaseHandler::SELECT_ALL_SQL_QUERY(const std::string &tableName, const std::string &outputFilePath) const {
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

    fileWriteSelectQueryResult(outputFilePath, queryResult);

    PQclear(queryResult);

    return 0;
}

int DatabaseHandler::SELECT_COLUMNS_SQL_QUERY(const std::string &tableName, const std::string &outputFilePath) const {
    // Make a query to get the column names
    std::string selectQuery =
            std::string("SELECT * FROM ") + tableName + std::string(" LIMIT 1;");

    PGresult *queryResult = nullptr;

    queryResult = PQexec(connection, selectQuery.c_str());

    if (PQresultStatus(queryResult) != PGRES_TUPLES_OK) {
        // Not successful SQL query
        fprintf(stderr, "%s[%d]: Select failed: %s\n",
                __FILE__, __LINE__, PQresultErrorMessage(queryResult));
        return 1;
    }

    std::vector<std::string> selectColumnNames;

    // Read column names, if they exist, add them to the vector
    while (true) {
        std::string currentSelectedColumn = readDatabaseIdentifier(COLUMN);
        if (currentSelectedColumn == ESCAPE)
            break;

        bool found = false;

        for (int i = 0; i < PQnfields(queryResult); i++) {
            std::string columnName{PQfname(queryResult, i)};

            if (columnName == currentSelectedColumn) {
                selectColumnNames.push_back(currentSelectedColumn);
                found = true;
                break;
            }
        }
        if (!found)
            std::cout << NO_COLUMN_FOUND(currentSelectedColumn);
    }

    selectQuery =
            std::string("SELECT ") + join(selectColumnNames, COMMA_SPACE_SEPARATOR) +
            std::string(" FROM ") + tableName + std::string(";");

    queryResult = PQexec(connection, selectQuery.c_str());

    if (PQresultStatus(queryResult) != PGRES_TUPLES_OK) {
        // Not successful SQL query
        fprintf(stderr, "%s[%d]: Select failed: %s\n",
                __FILE__, __LINE__, PQresultErrorMessage(queryResult));
        return 1;
    }

    fileWriteSelectQueryResult(outputFilePath, queryResult);

    PQclear(queryResult);
    return 0;
}

int DatabaseHandler::INSERT_SQL_QUERY(const std::string &tableName) const {
    // Make a query to get the column names
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

    std::vector<std::string> tableNames; // Vector with the table names
    tableNames.reserve(tableColumnsNumber);
    std::vector<std::string> insertValues; // Vector with the values
    insertValues.reserve(tableColumnsNumber);

    for (int i = 0; i < tableColumnsNumber; i++) {
        // Read the column value and keep it
        std::string currentColumnName{PQfname(queryResult, i)};

        if (currentColumnName == ID_COL_NAME) {
            std::string insertIdChoice = readColumnValue(
                VARCHAR_CODE_VALUE, "whether id should be included: (y/n)", connection);

            if (insertIdChoice != "y" && insertIdChoice != "Y") {
                std::cout << "Id column is being skipped.\n";
                continue;
            }
        }

        tableNames.push_back(currentColumnName); // Add the column name
        std::string currentInsertValue = readColumnValue(PQftype(queryResult, i), currentColumnName, connection);
        if (currentInsertValue == EMPTY_VALUE) {
            std::cout << "INSERT failed: Cannot insert one or more of the values.\n";
            PQclear(queryResult);
            return 1;
        }
        // Add the read data
        insertValues.push_back(currentInsertValue);
    }

    std::stringstream insertQueryStream{}; // Start to build the query
    insertQueryStream << "INSERT INTO " << tableName
            << " (" << join(tableNames, COMMA_SPACE_SEPARATOR)
            << ") VALUES (";

    const int actualTableColumnsNumber = static_cast<int>(tableNames.size()); // (Because of the id skip)
    for (int i = 0; i < actualTableColumnsNumber; i++) {
        // Add placeholders for the dynamic data to the query
        insertQueryStream << '$' << i + 1;
        if (i < actualTableColumnsNumber - 1)
            insertQueryStream << COMMA_SPACE_SEPARATOR;
    }
    insertQueryStream << ");";

    std::cout << insertQueryStream.str();

    const PGresult *insertResult = PQexecParams(
        connection,
        insertQueryStream.str().c_str(),
        actualTableColumnsNumber,
        nullptr,
        generateInsertParamValues(insertValues).data(),
        nullptr,
        nullptr,
        0
    );

    if (PQresultStatus(insertResult) != PGRES_COMMAND_OK) {
        // Problem with the Insertion of data
        std::cerr << "INSERT failed: " << PQresultErrorMessage(insertResult) << std::endl;

        PQclear(queryResult);
        return 1;
    }

    std::cout << "INSERT operation was successful.\n";

    PQclear(queryResult);
    return 0;
}

int DatabaseHandler::UPDATE_SQL_QUERY(const std::string &tableName) const {
    // Make a query to get the column names
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
        std::cout << NO_COLUMN_FOUND(updateColumn);

        PQclear(queryResult);
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
        std::cout << NO_COLUMN_FOUND(updateWhereColumn);

        PQclear(queryResult);
        return 1;
    }

    const char *paramValues[2]{
        updateValue.c_str(),
        updateWhereValue.c_str()
    };

    const std::string updateQuery =
            std::string("UPDATE ") + tableName + std::string(" SET ") + updateColumn +
            std::string(" = $1 WHERE ") + updateWhereColumn + std::string(" = $2;");

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

        PQclear(updateResult);
        return 1;
    }

    std::cout << "UPDATE operation was successful.\n";

    PQclear(updateResult);
    return 0;
}

int DatabaseHandler::DELETE_SQL_QUERY(const std::string &tableName) const {
    // Make a query to get the column names
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

        PQclear(queryResult);
        return 0;
    }

    const char *paramValues[1]{
        deleteValue.c_str()
    };

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

        PQclear(deleteResult);
        return 1;
    }

    std::cout << "DELETE operation was successful.\n";

    PQclear(deleteResult);
    return 0;
}

int DatabaseHandler::EXECUTE_SQL_QUERY() const {
    if (!validateUserCredentials())
        return 1;

    std::string customQuery;
    std::cout << "Enter SQL query to be executed:";
    std::getline(std::cin, customQuery);

    PGresult *customQueryResult = nullptr;
    customQueryResult = PQexec(connection, customQuery.c_str());

    if (PQresultStatus(customQueryResult) != PGRES_COMMAND_OK) {
        // SELECT has other error type
        std::cerr << "CUSTOM QUERY failed: " << PQerrorMessage(connection) << std::endl;
        PQclear(customQueryResult);
        return 1;
    }

    std::cout << "CUSTOM QUERY operation was successful.\n";

    PQclear(customQueryResult);
    return 0;
}

int DatabaseHandler::CREATE_TABLE_SQL_QUERY(const std::string &tableName) const {
    // TODO: Add permissions -> only auth users can create tables
    std::vector<std::string> columnDefinitions;

    while (true) {
        std::cout << "Enter column definition line or '" << ESCAPE << "':";

        std::string currentColumnDefinition;
        std::getline(std::cin, currentColumnDefinition);
        if (currentColumnDefinition == ESCAPE)
            break;

        columnDefinitions.push_back(currentColumnDefinition);
    }

    const std::string createTableQuery =
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

int DatabaseHandler::TRUNCATE_SQL_QUERY(const std::string &tableName) const {
    const std::string truncateQuery =
            std::string("TRUNCATE TABLE ") + tableName +
            std::string(" RESTART IDENTITY CASCADE;");

    PGresult *truncateResult = PQexec(connection, truncateQuery.c_str());

    if (PQresultStatus(truncateResult) != PGRES_COMMAND_OK) {
        std::cerr << "TRUNCATE failed: " << PQerrorMessage(connection) << '\n';
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
        std::cerr << "DROP TABLE failed: " << PQerrorMessage(connection) << '\n';
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
        std::cerr << "DROP DATABASE failed: " << PQerrorMessage(connection) << '\n';
        return 1;
    }

    std::cout << "DROP DATABASE operation was successful.\n";

    PQclear(dropDatabaseResult);
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

                        if (safeValue.length() >= 2 && safeValue.front() == '\'' && safeValue.back() == '\'') {
                            safeValue = safeValue.substr(1, safeValue.length() - 2);
                        }
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

std::string DatabaseHandler::readDatabaseIdentifier(const std::string &identifierType) {
    std::string identifierName;
    while (true) {
        std::cout << "Enter " << identifierType << " name:";
        std::getline(std::cin, identifierName);

        if (identifierName.empty()) {
            std::cout << "Invalid " << identifierType << " name.\n";
            continue;
        }

        if (stringValueDoesNotContainInvalidChars(identifierName))
            return identifierName;

        std::cout << "Invalid " << identifierType << " name.\n";
    }
}

std::string DatabaseHandler::readTableName() {
    return readDatabaseIdentifier(TABLE);
}

int DatabaseHandler::fileWriteSelectQueryResult(const std::string &outputFileNameEnv, const PGresult *queryResult) {
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
    return 0;
}

bool DatabaseHandler::validateUserCredentials() const {
    const std::string username = readColumnValue(VARCHAR_CODE_VALUE, "Username", connection);
    const std::string passwordHash = SHA256::hash(readColumnValue(VARCHAR_CODE_VALUE, "Password", connection));

    const char *paramValues[2]{username.c_str(), passwordHash.c_str()};

    const std::string selectAccountQuery =
            std::string("SELECT * FROM account WHERE username = $1 AND password = $2;");

    const PGresult *selectAccountResult = PQexecParams(
        connection,
        selectAccountQuery.c_str(),
        2, // Number of parameters
        nullptr, // Parameter types (NULL = infer from query)
        paramValues, // Parameter values
        nullptr, // Parameter lengths (NULL = assume text)
        nullptr, // Parameter formats (NULL = assume text)
        0 // Result format: 0 for text, 1 for binary
    );

    if (PQresultStatus(selectAccountResult) != PGRES_TUPLES_OK) {
        std::cerr << "SELECT failed: " << PQerrorMessage(connection) << std::endl;
        return false;
    }

    if (!PQntuples(selectAccountResult))
        std::cout << "Access Denied. Not authorized.\n";

    return PQntuples(selectAccountResult); // if there isn't such an entry -> length 0; else 1
}

std::string repeat(const char &ch, const std::string::size_type &times) {
    std::stringstream strStream{};

    for (int i = 0; i < times; ++i)
        strStream << ch;

    return strStream.str();
}


std::string addRightPadding(const std::string &valueStr, const std::string::size_type &size) {
    if (valueStr.length() <= size)
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

    constexpr char DASH_CHAR = '-';
    constexpr char ZERO_CHAR = '0';
    constexpr char NINE_CHAR = '9';
    return
            dateStr[4] == DASH_CHAR && dateStr[7] == DASH_CHAR
            && dateStr[0] >= ZERO_CHAR && dateStr[0] <= NINE_CHAR
            && dateStr[1] >= ZERO_CHAR && dateStr[1] <= NINE_CHAR
            && dateStr[2] >= ZERO_CHAR && dateStr[2] <= NINE_CHAR
            && dateStr[3] >= ZERO_CHAR && dateStr[3] <= NINE_CHAR
            && dateStr[5] >= ZERO_CHAR && dateStr[5] <= NINE_CHAR
            && dateStr[6] >= ZERO_CHAR && dateStr[6] <= NINE_CHAR
            && dateStr[8] >= ZERO_CHAR && dateStr[8] <= NINE_CHAR
            && dateStr[9] >= ZERO_CHAR && dateStr[9] <= NINE_CHAR;
}


bool stringValueDoesNotContainInvalidChars(const std::string &strValue) {
    const std::string VALID_CHARS_STR = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
    return strValue
           .find_first_not_of(VALID_CHARS_STR) == std::string::npos;
}
