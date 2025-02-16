#include <iostream>
#include <libpq-fe.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>


#define POSTGRE_SQL_PORT std::string("5432")
#define POSTGRE_SQL_DB_NAME std::string("working_project_db")

#define POSTGRE_SQL_ADMIN_ENV_NAME "POSTGRE_SQL_ADMIN"
#define POSTGRE_SQL_ADMIN_ENV_PASS "POSTGRE_SQL_PASS"
#define OUTPUT_FILE_ENV "OUTPUT_FILE"

#define BETWEEN_ROWS_SEPARATOR '.'
#define TABLE_ROW_SEPARATOR '-'
#define TABLE_COL_SEPARATOR '|'
#define END_OF_COL_SEPARATOR " |"
#define COMMA_SPACE_SEPARATOR std::string(", ")
#define ESCAPE std::string("esc")


std::string repeat(const char &, const std::string::size_type &);

std::string addRightPadding(const std::string &, const std::string::size_type &);

std::string createBetweenRowsRow(const std::string::size_type *, const int &);

std::string join(const std::string *, const int &, const std::string &);

std::string join(const std::vector<std::string> &, const std::string &);

std::vector<const char *> generateInsertParamValues(const std::vector<std::string> &);

bool isSqlDateFormatValid(const std::string &);


int SELECT_ALL_SQL_QUERY(PGconn *, const std::string &, const std::string &);

int INSERT_SQL_QUERY(PGconn *, const std::string &);

int UPDATE_SQL_QUERY(PGconn *, const std::string &);

int DELETE_SQL_QUERY(PGconn *, const std::string &);


int main() {
    const char *userEnv = std::getenv(POSTGRE_SQL_ADMIN_ENV_NAME);
    const char *passEnv = std::getenv(POSTGRE_SQL_ADMIN_ENV_PASS);
    const char *outputFileNameEnv = std::getenv(OUTPUT_FILE_ENV);

    if (userEnv == nullptr || passEnv == nullptr) {
        std::cerr
                << "Error: Environment variables " << POSTGRE_SQL_ADMIN_ENV_NAME << " or "
                << POSTGRE_SQL_ADMIN_ENV_PASS << " are not set." << std::endl;
        return 1;
    }

    if (outputFileNameEnv == nullptr) {
        std::cerr
                << "Error: Environment variable " << OUTPUT_FILE_ENV << " is not set." << std::endl;
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

    std::string tableName;
    std::cout << "Enter table name query:";
    std::cin >> tableName;

    // INSERT_SQL_QUERY(connection, tableName);

    // UPDATE_SQL_QUERY(connection, tableName);

    // DELETE_SQL_QUERY(connection, tableName);

    SELECT_ALL_SQL_QUERY(connection, tableName, outputFileNameEnv);

    // CREATE TABLE ...;
    /******************************************************************************************************************/
    // TODO: move to a method, add to a class with SQL methods (header file with .cpp impls)
# if 0
    std::string createTableName;

    std::cout << "Enter Table name:";
    std::cin >> createTableName;

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
            + createTableName + std::string(" (")
            + join(columnDefinitions, COMMA_SPACE_SEPARATOR) + std::string(");");

    PGresult* createTableResult =  PQexec(connection, createTableQuery.c_str());

    if (PQresultStatus(createTableResult) != PGRES_COMMAND_OK) {
        std::cerr << "CREATE TABLE failed: " << PQerrorMessage(connection) << '\n';

    } else {
        std::cout << "Table '" << createTableName << "' created successfully.\n";
    }

    PQclear(createTableResult);
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

    // DROP DATABASE ...;
    /******************************************************************************************************************/
    // TODO: move to a method, add to a class with SQL methods (header file with .cpp impls)
#if 0
    std::string databaseName;

    std::cout << "Enter Database name:";
    std::cin >> databaseName;

    const std::string dropDatabaseQuery = std::string("DROP DATABASE IF EXISTS ") + databaseName + std::string(";");

    PGresult* dropDatabaseResult = PQexec(connection, dropDatabaseQuery.c_str());

    if (PQresultStatus(dropDatabaseResult) != PGRES_COMMAND_OK) {
        std::cerr << "DROP DATABASE failed: " << PQerrorMessage(connection) << std::endl;

    } else {
        std::cout << "Database " << databaseName << " dropped successfully.\n";
    }

    PQclear(dropDatabaseResult);
#endif

    PQfinish(connection);

    return 0;
}


int SELECT_ALL_SQL_QUERY(PGconn *connection, const std::string &tableName, const std::string &outputFileNameEnv) {
    const std::string selectQuery = std::string("SELECT * FROM ") + tableName + std::string(";");

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

    // Calculate the width of every column (Table Col names)
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

int INSERT_SQL_QUERY(PGconn *connection, const std::string &tableName) {
    const std::string selectQuery = std::string("SELECT * FROM ") + tableName + std::string(" LIMIT 1;");

    PGresult *queryResult = nullptr;
    queryResult = PQexec(connection, selectQuery.c_str());

    if (PQresultStatus(queryResult) != PGRES_TUPLES_OK) {
        // Not successful SQL query
        fprintf(stderr, "%s[%d]: Select failed: %s\n",
                __FILE__, __LINE__, PQresultErrorMessage(queryResult));
        return 1;
    }

    const int tableColumnsNumber = PQnfields(queryResult);
    auto *tableNames = new std::string[tableColumnsNumber]{}; // Dynamic arr with the table names
    std::vector<std::string> insertValues; // Vector with the values
    bool skipId = false;

    for (int i = 0; i < tableColumnsNumber; i++) {
        // Read the column value and keep it
        std::string currentColumnName{PQfname(queryResult, i)};
        if (currentColumnName == "id") {
            skipId = true;
            continue;
        }
        tableNames[i] = currentColumnName; // Add the column name to the arr

        std::cout << "Enter " << currentColumnName << ':'; // Prompt the user for input

        // TODO: Add validations to the entered data
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
                std::string currentValue;
                std::cin >> currentValue;

                if (isSqlDateFormatValid(currentValue))
                    insertValues.push_back(currentValue);
                else
                    std::cout << "Invalid date entered!";

                break;
            }
            default: {
                // TODO: add other type cases or write default behaviour
            }
        }
    }

    std::stringstream insertQueryStream{}; // Start to build the query
    insertQueryStream << "INSERT INTO " << tableName
            << " (" << join(tableNames, tableColumnsNumber, COMMA_SPACE_SEPARATOR) << ") VALUES (";

    for (int i = 0; i < tableColumnsNumber; i++) {
        // Add placeholders for the dynamic data to the query
        if (skipId && i == 0)
            continue;

        insertQueryStream << '$' << (skipId ? i : i + 1);
        if (i < tableColumnsNumber - 1)
            insertQueryStream << COMMA_SPACE_SEPARATOR;
    }
    insertQueryStream << ");";

    std::cout << insertQueryStream.str() << "\n";

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

int UPDATE_SQL_QUERY(PGconn *connection, const std::string &tableName) {
    const std::string selectQuery = std::string("SELECT * FROM ") + tableName + std::string(" LIMIT 1;");

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
            std::cout << "Enter " << currentColumnName << " value:";

            // TODO: Add validations to the entered data
            switch (PQftype(queryResult, i)) {
                case 1043: {
                    // VARCHAR
                    std::string currentValue;

                    if (std::cin.peek() == '\n')
                        std::cin.ignore();
                    std::getline(std::cin, currentValue);

                    updateValue = currentValue;
                    break;
                }
                case 23: {
                    // INT4
                    int currentValue;
                    std::cin >> currentValue;

                    updateValue = std::to_string(currentValue);
                    break;
                }
                case 1700: {
                    // DECIMAL, NUMERIC
                    double currentValue;
                    std::cin >> currentValue;

                    updateValue = std::to_string(currentValue);
                    break;
                }
                case 1082: {
                    // DATE
                    std::string currentValue;
                    std::cin >> currentValue;

                    if (isSqlDateFormatValid(currentValue))
                        updateValue = currentValue;
                    else
                        std::cout << "Invalid date entered!";

                    break;
                }
                default: {
                    // TODO: add other type cases or write default behaviour
                }
            }
            break;
        }
    }

    if (updateValue.empty()) {
        std::cout << "No column found with name " << updateColumn << ".\n";
        return 1;
    }

    std::string updateWhereColumn, updateWhereValue{};

    std::cout << "Enter the name of the column for the WHERE clause:"; // Prompt the user to enter column
    std::cin >> updateWhereColumn;

    for (int i = 0; i < PQnfields(queryResult); i++) {
        const std::string currentColumnName{PQfname(queryResult, i)};

        if (currentColumnName == updateWhereColumn) {
            // Read different type of data type
            std::cout << "Enter " << currentColumnName << " value:";

            // TODO: Add validations to the entered data
            switch (PQftype(queryResult, i)) {
                case 1043: {
                    // VARCHAR
                    std::string currentValue;

                    if (std::cin.peek() == '\n')
                        std::cin.ignore();
                    std::getline(std::cin, currentValue);

                    updateWhereValue = currentValue;
                    break;
                }
                case 23: {
                    // INT4
                    int currentValue;
                    std::cin >> currentValue;

                    updateWhereValue = std::to_string(currentValue);
                    break;
                }
                case 1700: {
                    // DECIMAL, NUMERIC
                    double currentValue;
                    std::cin >> currentValue;

                    updateWhereValue = std::to_string(currentValue);
                    break;
                }
                case 1082: {
                    // DATE
                    std::string currentValue;
                    std::cin >> currentValue;

                    if (isSqlDateFormatValid(currentValue))
                        updateWhereValue = currentValue;
                    else
                        std::cout << "Invalid date entered!";

                    break;
                }
                default: {
                    // TODO: add other type cases or write default behaviour
                }
            }
            break;
        }
    }

    if (updateWhereValue.empty()) {
        std::cout << "No column found with name " << updateColumn << ".\n";
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

int DELETE_SQL_QUERY(PGconn *connection, const std::string &tableName) {
    const std::string selectQuery = std::string("SELECT * FROM ") + tableName + std::string(" LIMIT 1;");

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

            // TODO: Add validations to the entered data
            switch (PQftype(queryResult, i)) {
                case 1043: {
                    // VARCHAR
                    std::string currentValue;

                    if (std::cin.peek() == '\n')
                        std::cin.ignore();
                    std::getline(std::cin, currentValue);

                    deleteValue = currentValue;
                    break;
                }
                case 23: {
                    // INT4
                    int currentValue;
                    std::cin >> currentValue;

                    deleteValue = std::to_string(currentValue);
                    break;
                }
                case 1700: {
                    // DECIMAL, NUMERIC
                    double currentValue;
                    std::cin >> currentValue;

                    deleteValue = std::to_string(currentValue);
                    break;
                }
                case 1082: {
                    // DATE
                    std::string currentValue;
                    std::cin >> currentValue;

                    if (isSqlDateFormatValid(currentValue))
                        deleteValue = currentValue;
                    else
                        std::cout << "Invalid date entered!";

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
