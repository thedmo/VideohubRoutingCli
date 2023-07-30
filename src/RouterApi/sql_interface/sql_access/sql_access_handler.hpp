#pragma once

#include <whereami++.h>
#include <sqlite3.h>
#include <istream>
#include <iostream>
#include <variant>


using Field =
std::variant<
	int,
	std::string,
	std::vector<char>,
	std::variant<std::monostate>
>;
using Row = std::vector<Field>;
using Table = std::vector<Row>;

	const std::string TEXT = "VARCHAR";
	const std::string INTEGER = "INT";
	const std::string BLOB = "BLOB";

/// <summary>
/// Functionality to directly talk to sqlite3 library (only for Internal use)
/// </summary>
namespace {
	template<typename T>
	using DataContainer = std::vector<T>;

	// direct communication with sqlite3 library
	class SqlCom {
	private:
		static inline sqlite3* m_database;

	public:

		/// <summary>
		/// Connects to database located at same location as executable. Creates it, if it does not exist
		/// </summary>
		/// <param name="dbName">name of the database</param>
		/// <returns>sqlite errorcode. 0 = SQLITE_OK</returns>
		static int ConnectToDatabase(const std::string dbName) {
			int result = 0;
			std::string path = whereami::getExecutablePath().dirname();
			path = path + "/" + dbName + ".db";
			result = sqlite3_open(path.c_str(), &m_database);

			return result;
		}

		/// <summary>
		/// Closes connection to last opened database
		/// </summary>
		/// <returns>sqlite errorcode. 0 = SQLITE_OK</returns>
		static int CloseConnection() {
			return sqlite3_close_v2(m_database);
		}

		static std::string GetLastErrMsg() {
			return sqlite3_errmsg(m_database);
		}

	private:
		/// <summary>
		/// Gets Field of ColumnName from result of query
		/// </summary>
		/// <param name="statement">sqlite3_stmt objectpointer of query</param>
		/// <param name="index">index of column to get name from</param>
		/// <returns>std::variant with name stored as string.</returns>
		static Field GetColumnNameField(sqlite3_stmt* statement, int index) {
			Field field = std::variant<std::monostate >();

			field = (std::string)((char*)sqlite3_column_name(statement, index));

			return field;
		}

		/// <summary>
		/// Gets Field of resulting table from query. Deduces returntype by using variant
		/// </summary>
		/// <param name="statement">sqlite3_stmt objectpointer of query</param>
		/// <param name="index">index of field in row of resultset</param>
		/// <returns>variant with data from field stored into it (INTEGER, VARCHAR or BLOB)</returns>
		static Field GetField(sqlite3_stmt* statement, int index) {
			Field field = std::variant<std::monostate >();

			// VARCHAR
			if (sqlite3_column_type(statement, index) == SQLITE_TEXT)
			{
				const unsigned char* textData = sqlite3_column_text(statement, index);
				if (textData) {
					field = std::string(reinterpret_cast<const char*>(textData));
				}
				else {
					field = "";  // Return an empty string if the value is NULL
				}
			}

			// INTEGER
			else if (sqlite3_column_type(statement, index) == SQLITE_INTEGER)
			{
				field = sqlite3_column_int(statement, index);
			}

			// BLOB
			else if (sqlite3_column_type(statement, index) == SQLITE_BLOB)
			{
				const std::vector<char> blobData(
					static_cast<const char*>(sqlite3_column_blob(statement, index)),
					static_cast<const char*>(sqlite3_column_blob(statement, index)) +
					sqlite3_column_bytes(statement, index));


				field = blobData;
			}

			// NULL
			else if (sqlite3_column_type(statement, index) == SQLITE_NULL)
			{
				field = std::variant<std::monostate>();
			}

			return field;
		}

		/// <summary>
		/// Query database with prepared statement
		/// </summary>
		/// <param name="statement">prepared statement</param>
		/// <param name="resultRowCount">storage for rowcount of result set</param>
		/// <param name="resultTable">Table storage of resultset: row==0 --> vector with Column Names, row>0 --> vector with Fields </param>
		/// <returns>result as int: 0 --> OK</returns>
		static int query(sqlite3_stmt* statement, int& resultRowCount, Table& resultTable) {
			resultRowCount = 0;

			while (sqlite3_step(statement) == SQLITE_ROW) {
				int columnCount = sqlite3_column_count(statement);

				// Fill Result Table
				if (resultRowCount == 0)
				{
					// Row 0 --> Column Names
					resultTable.push_back(Row());

					for (int i = 0; i < columnCount; i++) {
						Field field = GetColumnNameField(statement, i);
						resultTable[resultRowCount].push_back(field);
					}

					resultRowCount++;
				}

				// Rows>0 --> Fields
				resultTable.push_back(Row());

				for (size_t colNum = 0; colNum < columnCount; colNum++)
				{
					Field field = GetField(statement, colNum);
					resultTable[resultRowCount].push_back(field);
				}

				resultRowCount++;
			}
			return sqlite3_finalize(statement);
		}

	public:
		/// <summary>
		/// Query database with prepared statement
		/// </summary>
		/// <param name="statement">prepared statement</param>
		/// <param name="rowCount">reference to int to retrieve rowcount of result set</param>
		/// <returns>sqlite errcode. 0 = SQLITE_OK</returns>
		static int Query(sqlite3_stmt* statement, int& rowCount) {
			Table dataVariantVector;

			query(statement, rowCount, dataVariantVector);

			return sqlite3_errcode(m_database);
		}

		/// <summary>
		/// Query database with prepared statement
		/// </summary>
		/// <param name="statement">prepared statement</param>
		/// <param name="resultTable">reference to vector to retrieve data as strings</param>
		/// <returns>sqlite errcode. 0 = SQLITE_OK</returns>
		static int Query(sqlite3_stmt* statement, Table& resultTable) {
			int tempInt;

			query(statement, tempInt, resultTable);

			return sqlite3_errcode(m_database);
		}

		/// <summary>
		/// Query database with prepared statement
		/// </summary>
		/// <param name="statement">prepared statement</param>
		/// <returns>sqlite errcode. 0 = SQLITE_OK</returns>
		static int Query(sqlite3_stmt* statement) {
			int tempInt;
			Table resultTable;

			query(statement, tempInt, resultTable);

			return sqlite3_errcode(m_database);
		}

	private:
		/// <summary>
		/// Gets a statement objectpointer to query a database with a sqlite query string
		/// </summary>
		/// <param name="query">string with query for database. replace values to be bound with an ? (Don't forget to use Bind Method)</param>
		/// <param name="result">integer to catch errorcode. OK = 0</param>
		/// <returns>sqlite3_stmt objectpointer with statement</returns>
		static sqlite3_stmt* getStatement(const std::string query, int& result) {
			sqlite3_stmt* statement;
			result = sqlite3_prepare_v2(m_database, query.c_str(), -1, &statement, 0);
			return statement;
		}

	public:
		/// <summary>
		/// Gets a statement objectpointer to query a database with a sqlite query string
		/// </summary>
		/// <param name="queryStr">string with query for database. replace values to be bound with an ? (Don't forget to use Bind Method)</param>
		/// <returns>sqlite3_stmt objectpointer with statement</returns>
		static sqlite3_stmt* GetStatement(std::string queryStr) {
			int result = 0;
			return getStatement(queryStr, result);
		}

		/// <summary>
		/// Gets a statement objectpointer to query a database with a sqlite query string
		/// </summary>
		/// <param name="queryStr">string with query for database. replace values to be bound with an ? (Don't forget to use Bind Method)</param>
		/// <param name="result">integer to catch errorcode. OK = 0</param>
		/// <returns>sqlite3_stmt objectpointer with statement</returns>
		static sqlite3_stmt* GetStatement(std::string queryStr, int& result) {
			return getStatement(queryStr, result);
		}
	};

	/// <summary>
	/// Serializes containers into vectors of chars to be used as datablobs
	/// </summary>
	class DataSerializer {
	public:
		/// <summary>
		/// Serializes data into vector of chars
		/// </summary>
		/// <param name="vec">reference to data to be serialized</param>
		/// <returns>serialized datacontainer</returns>
		static std::vector<char> SerializeVectorString(const std::vector<std::string>& vec) {
			std::string serializedData;
			for (const std::string& str : vec) {
				serializedData += str + '\n';  // Use a newline character as a delimiter
			}
			return std::vector<char>(serializedData.begin(), serializedData.end());
		}

		/// <summary>
		/// Serializes data into vector of chars
		/// </summary>
		/// <param name="vec">reference to data to be serialized</param>
		/// <returns>serialized datacontainer</returns>
		static std::vector<char> SerializeVectorInt(const std::vector<int>& vec) {
			const char* buffer = reinterpret_cast<const char*>(vec.data());
			const size_t size = vec.size() * sizeof(int);
			return std::vector<char>(buffer, buffer + size);
		}

		/// <summary>
		/// Serializes data into vector of chars
		/// </summary>
		/// <param name="vec">reference to data to be serialized</param>
		/// <returns>serialized datacontainer</returns>
		static std::vector<char> SerializeVectorPair(const std::vector<std::pair<int, int>>& vec) {
			std::vector<char> serializedData;
			serializedData.reserve(vec.size() * sizeof(std::pair<int, int>));
			for (const auto& pair : vec) {
				const char* pairData = reinterpret_cast<const char*>(&pair);
				serializedData.insert(serializedData.end(), pairData, pairData + sizeof(std::pair<int, int>));
			}
			return serializedData;
		}
	};

	/// <summary>
	/// deserialization of containertypes from vector<char> blobs to specified datatypes 
	/// </summary>
	class DataDeserializer {
	public:
		/// <summary>
		/// Convert vector with chars into vector with deserialized data
		/// </summary>
		/// <param name="serializedData">data to deserialize</param>
		/// <returns>vector with deserialized data</returns>
		static std::vector<std::string> DeserializeVectorString(const std::vector<char>& serializedData) {
			std::vector<std::string> deserializedData;
			std::string str(serializedData.begin(), serializedData.end());
			std::istringstream iss(str);
			std::string item;
			while (std::getline(iss, item, '\n')) {  // Assuming newline character as the delimiter
				deserializedData.push_back(item);
			}
			return deserializedData;
		}

		/// <summary>
		/// Convert vector with chars into vector with deserialized data
		/// </summary>
		/// <param name="serializedData">data to deserialize</param>
		/// <returns>vector with deserialized data</returns>
		static std::vector<int> DeserializeVectorInt(const std::vector<char>& serializedData) {
			const int* buffer = reinterpret_cast<const int*>(serializedData.data());
			const size_t size = serializedData.size() / sizeof(int);
			return std::vector<int>(buffer, buffer + size);
		}

		/// <summary>
		/// Convert vector with chars into vector with deserialized data
		/// </summary>
		/// <param name="serializedData">data to deserialize</param>
		/// <returns>vector with deserialized data</returns>
		static std::vector<std::pair<int, int>> DeserializeVectorPair(const std::vector<char>& serializedData) {
			std::vector<std::pair<int, int>> deserializedData;
			const char* data = serializedData.data();
			const size_t dataSize = serializedData.size();
			const size_t pairSize = sizeof(std::pair<int, int>);
			if (dataSize % pairSize == 0) {
				const std::pair<int, int>* pairs = reinterpret_cast<const std::pair<int, int>*>(data);
				const size_t numPairs = dataSize / pairSize;
				for (size_t i = 0; i < numPairs; ++i) {
					deserializedData.push_back(pairs[i]);
				}
			}
			return deserializedData;
		}
	};

	/// <summary>
	/// Bind Values to sqlite3_stmt objectpointers
	/// </summary>
	class ValueBinder : private DataSerializer {
	public:
		/// <summary>
		/// Bind Value into given statement
		/// </summary>
		/// <param name="stmt">sqlite3_stmt objectpointer</param>
		/// <param name="index">index of value (+1 for every ? in a query string. starts at 1</param>
		/// <param name="value">value to bind into statement</param>
		/// <returns>int; 0 = OK</returns>
		static int Bind(sqlite3_stmt* stmt, int index, int value) {
			return sqlite3_bind_int(stmt, index, value);
		}

		/// <summary>
		/// Bind Value into given statement
		/// </summary>
		/// <param name="stmt">sqlite3_stmt objectpointer</param>
		/// <param name="index">index of value (+1 for every ? in a query string. starts at 1</param>
		/// <param name="value">value to bind into statement</param>
		/// <returns>int; 0 = OK</returns>
		static int Bind(sqlite3_stmt* stmt, int index, const std::string& value) {
			return sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_STATIC);
		}

		/// <summary>
		/// Bind Value into given statement
		/// </summary>
		/// <param name="stmt">sqlite3_stmt objectpointer</param>
		/// <param name="index">index of value (+1 for every ? in a query string. starts at 1</param>
		/// <param name="value">value to bind into statement</param>
		/// <returns>int; 0 = OK</returns>
		static int Bind(sqlite3_stmt* stmt, int index, const std::vector<std::string>& container) {
			auto serializedData = SerializeVectorString(container);

			return sqlite3_bind_blob(stmt, index, serializedData.data(), static_cast<int>(serializedData.size()), SQLITE_TRANSIENT);
		}

		/// <summary>
		/// Bind Value into given statement
		/// </summary>
		/// <param name="stmt">sqlite3_stmt objectpointer</param>
		/// <param name="index">index of value (+1 for every ? in a query string. starts at 1</param>
		/// <param name="value">value to bind into statement</param>
		/// <returns>int; 0 = OK</returns>
		static int Bind(sqlite3_stmt* stmt, int index, const std::vector<int>& container) {
			auto serializedData = SerializeVectorInt(container);

			return sqlite3_bind_blob(stmt, index, serializedData.data(), static_cast<int>(serializedData.size()), SQLITE_TRANSIENT);
		}

		/// <summary>
		/// Bind Value into given statement
		/// </summary>
		/// <param name="stmt">sqlite3_stmt objectpointer</param>
		/// <param name="index">index of value (+1 for every ? in a query string. starts at 1</param>
		/// <param name="value">value to bind into statement</param>
		/// <returns>int; 0 = OK</returns>
		static int Bind(sqlite3_stmt* stmt, int index, const std::vector<std::pair<int, int>>& container) {
			auto serializedData = SerializeVectorPair(container);

			return sqlite3_bind_blob(stmt, index, serializedData.data(), static_cast<int>(serializedData.size()), SQLITE_TRANSIENT);
		}
	};
}

/// <summary>
/// Wrapper for sqlite3 library.
/// </summary>
namespace SqliteHandler {
	/// <summary>
	/// class to gather infromation about tables in databases
	/// </summary>
	class TableInfo : private SqlCom, private ValueBinder {
	public:
		/// <summary>
		/// Compares types of columns in tables against specific columntype
		/// </summary>
		/// <param name="dbName">name of db</param>
		/// <param name="tableName">name of table</param>
		/// <param name="columnName">name of column</param>
		/// <param name="typeStr">name of type to compare agains</param>
		/// <returns>int; 0 = OK</returns>
		static int CompareColumnType(std::string dbName, std::string tableName, std::string columnName, std::string typeStr) {
			int result = 0;
			Table resultSet;
			std::string queryStr = "PRAGMA table_info (" + tableName + ");";

			auto statement = GetStatement(queryStr, result);
			if (result) return 1;

			result = Query(statement, resultSet);
			if (result) return 1;

			size_t nameIndex = 0;
			size_t typeIndex = 0;

			for (size_t colNum = 0; colNum < resultSet[0].size(); colNum++)
			{
				if (std::get<std::string>(resultSet[0][colNum]) == "name")
				{
					nameIndex = colNum;
				}
				if (std::get<std::string>(resultSet[0][colNum]) == "type")
				{
					typeIndex = colNum;
				}
			}

			if (nameIndex == 0 && typeIndex == 0) return 1;

			for (size_t i = 1; i < resultSet.size(); i++)
			{
				if (std::get<std::string>(resultSet[i][nameIndex]) == columnName && std::get<std::string>(resultSet[i][typeIndex]) == typeStr)
				{
					return 0;
				}
			}

			return 1;
		}

		/// <summary>
		/// Get Names of columns in table
		/// </summary>
		/// <param name="dbName">db name</param>
		/// <param name="tableName">name of table</param>
		/// <param name="result">reference to int to store resultcode 0 = OK</param>
		/// <returns>vector of strings with names of columns</returns>
		static std::vector<std::string> GetColumnNames(std::string dbName, std::string tableName, int& result) {
			std::vector<std::string> columns;
			Table resultSet;

			result = SqlCom::ConnectToDatabase(dbName);
			if (result) return columns;

			std::string queryStr = "PRAGMA table_info (" + tableName + ");";
			auto statement = GetStatement(queryStr, result);
			if (result) return columns.push_back(""), columns;

			result = Query(statement, resultSet);
			if (result) return columns.push_back(""), columns;

			size_t nameIndex = 0;

			for (size_t colNum = 0; colNum < resultSet[0].size(); colNum++)
			{
				if (std::get<std::string>(resultSet[0][colNum]) == "name")
				{
					nameIndex = colNum;
				}
			}

			for (size_t i = 1; i < resultSet.size(); i++)
			{
				auto columnName = std::get<std::string>(resultSet[i][nameIndex]);

				columns.push_back(columnName);
			}

			result = SqlCom::CloseConnection();
			if (result) return columns;

			return columns;
		}

		/// <summary>
		/// Check, if column exists in specified table
		/// </summary>
		/// <param name="dbName">name of database</param>
		/// <param name="tableName">name of table</param>
		/// <param name="columnNameToLookFor">name of column to look for</param>
		/// <returns>1 = Does exist, 0 = Does not exist</returns>
		static int CheckIfColumnExists(std::string dbName, std::string tableName, std::string columnNameToLookFor) {
			int result = 0;

			auto columnNames = GetColumnNames(dbName, tableName, result);
			if (result) return 1;

			for (auto& columnName : columnNames) {
				if (columnNameToLookFor == columnName) {
					return 1; // does exist
				}
			}
			return 0; // does not exist
		}

		/// <summary>
		/// Checks if row with specified data already exists
		/// </summary>
		/// <param name="dbName">name of db</param>
		/// <param name="tableName">name of table</param>
		/// <param name="colName">name of column to determine row by</param>
		/// <param name="fieldValue">value to look for in given column</param>
		/// <returns>1 = Exists, 0 = Not Existing</returns>
		static int CheckIfRowExists(const std::string dbName, const std::string tableName, const std::string colName, const std::string fieldValue) {
			int result;

			result = CheckIfColumnExists(dbName, tableName, colName);
			if (!result) return 1;

			result = SqlCom::ConnectToDatabase(dbName);
			if (result) return 1;

			std::string queryStr = "SELECT * FROM " + tableName + " WHERE " + colName + " = ?;";
			auto statement = SqlCom::GetStatement(queryStr, result);
			if (result) return 1;

			result = ValueBinder::Bind(statement, 1, fieldValue);
			if (result) return 1;

			Table resultSet;
			result = SqlCom::Query(statement, resultSet);
			if (result) return 1;

			if (resultSet.size() > 0) return 1; // does exist

			return 0; // does not exist
		}
	}; // class TableInfo

	/// <summary>
	/// Creation or Modification of Tables in a database 
	/// </summary>
	class DbMod : private SqlCom, private ValueBinder, private TableInfo {
	public:
		/// <summary>
		/// Creates a table with a primary key. Database gets created if not existing
		/// </summary>
		/// <param name="dbName">name of database to be used</param>
		/// <param name="tableName">name of table</param>
		/// <param name="primaryKeyName"></param>
		/// <param name="primaryKeyType"></param>
		/// <returns>int; OK = 0</returns>
		static int CreateTableWithPrimaryKey(std::string dbName, std::string tableName, std::string primaryKeyName, const std::string primaryKeyType) {
			int result;
			std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " (" + primaryKeyName + " " + primaryKeyType + " PRIMARY KEY);";
			auto database = SqlCom::ConnectToDatabase(dbName);
			auto statement = SqlCom::GetStatement(query);

			result = SqlCom::Query(statement);
			if (result) return 1;

			return SqlCom::CloseConnection();
		}

		/// <summary>
		/// Creates a table with a foreign key. Table with primary key must exist for this
		/// </summary>
		/// <param name="dbName">name of database</param>
		/// <param name="tableName">name of table</param>
		/// <param name="foreignKeyColumnName">name of key column</param>
		/// <param name="foreignKeyColumnType">valuetype of keycolumn</param>
		/// <param name="primaryKeyTableName">name table with primary key in it</param>
		/// <param name="primaryKeyColumnName">name of column from table with primary key</param>
		/// <returns>int; OK = 0</returns>
		static int CreateTableWithForeignKey(std::string dbName, std::string tableName, std::string foreignKeyColumnName, std::string foreignKeyColumnType, std::string primaryKeyTableName, std::string primaryKeyColumnName) {
			int result;
			std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " (" + foreignKeyColumnName + " " + foreignKeyColumnType + ", FOREIGN KEY(" + foreignKeyColumnName + ") REFERENCES " + primaryKeyTableName + "(" + primaryKeyColumnName + "));";
			auto database = SqlCom::ConnectToDatabase(dbName);
			auto statement = SqlCom::GetStatement(query);

			result = SqlCom::Query(statement);
			if (result) return 1;

			return SqlCom::CloseConnection();
		}

		/// <summary>
		/// Creates Table with primary and foreign key. Table with referenced primary key must exist for this
		/// </summary>
		/// <param name="dbName">name of database</param>
		/// <param name="tableName">name of new table</param>
		/// <param name="primaryKeyColumn">name of column with primary key</param>
		/// <param name="primaryType">type of column with primary key</param>
		/// <param name="foreignKeyColumn">name of column with foreign key</param>
		/// <param name="foreignKeyType">type of column with foreign key</param>
		/// <param name="foreignKeyTable">name of table which foreign key refers to</param>
		/// <param name="foreignKeyColumnRef">name of column which foreign key refers to</param>
		/// <returns>int; 0 = OK </returns>
		static int CreateTablePrimaryForeign(
			std::string dbName,
			std::string tableName,
			std::string primaryKeyColumn,
			std::string primaryType,
			std::string foreignKeyColumn,
			std::string foreignKeyType,
			std::string foreignKeyTable,
			std::string foreignKeyColumnRef
		) {
			int result;

			std::string query =
				"CREATE TABLE IF NOT EXISTS " +
				tableName +
				" (" +
				primaryKeyColumn +
				" " +
				primaryType +
				" PRIMARY KEY, " +
				foreignKeyColumn +
				" " +
				foreignKeyType +
				", FOREIGN KEY(" +
				foreignKeyColumn +
				") REFERENCES " +
				foreignKeyTable +
				"(" +
				foreignKeyColumnRef +
				"));";

			//std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " (" + foreignKeyColumnName + " " + foreignKeyColumnType + ", FOREIGN KEY(" + foreignKeyColumnName + ") REFERENCES " + primaryKeyTableName + "(" + primaryKeyColumnName + "));";
			auto database = SqlCom::ConnectToDatabase(dbName);
			auto statement = SqlCom::GetStatement(query);

			result = SqlCom::Query(statement);
			if (result) return 1;

			return SqlCom::CloseConnection();
		}

		/// <summary>
		/// Adds new column to table
		/// </summary>
		/// <param name="dbName">name of database</param>
		/// <param name="tableName">name of table</param>
		/// <param name="columnName">name of column</param>
		/// <param name="columnType">type of column</param>
		/// <returns>int; OK = 0 </returns>
		static int AddColumn(std::string dbName, std::string tableName, std::string columnName, std::string columnType) {
			int result;

			result = CheckIfColumnExists(dbName, tableName, columnName);
			if (result)	return 1; // return 1, if already existing

			result = SqlCom::ConnectToDatabase(dbName);
			if (result)	return 1;

			auto queryStr = "ALTER TABLE " + tableName + " ADD COLUMN " + columnName + " " + columnType + ";";
			auto statement = SqlCom::GetStatement(queryStr, result);
			if (result)	return 1;

			result = SqlCom::Query(statement);
			if (result)	return 1;

			return SqlCom::CloseConnection();
		}

		/// <summary>
		/// Adds a new row to a table
		/// </summary>
		/// <param name="dbName">name of db</param>
		/// <param name="table">name of table</param>
		/// <param name="column">name of key column</param>
		/// <param name="value">value of column</param>
		/// <returns>int; OK = 0</returns>
		static int InsertRow(const std::string dbName, const std::string& table, const std::string& column, std::string value) {
			int result = 0;
			int rowCount = 0;
			std::string queryStr;

			result = CheckIfRowExists(dbName, table, column, value);
			if (result) return 1;

			result = SqlCom::ConnectToDatabase(dbName);
			if (result) return 1;

			queryStr = "INSERT OR IGNORE INTO " + table + " (" + column + ") VALUES (?)";
			auto statement = SqlCom::GetStatement(queryStr, result);
			if (result) return 1;

			result = Bind(statement, 1, value);

			result = SqlCom::Query(statement);
			if (result) return 1;

			return SqlCom::CloseConnection();
		}

		/// <summary>
		/// Adds a new row to a table, does not check, if row with key identifier already exists
		/// </summary>
		/// <param name="dbName">name of db</param>
		/// <param name="table">name of table</param>
		/// <param name="column">name of key column</param>
		/// <param name="value">value of column</param>
		/// <returns>int; OK = 0</returns>
		static int InsertRowNoCheck(const std::string dbName, const std::string& table, const std::string& column, std::string value) {
			int result = 0;
			int rowCount = 0;
			std::string queryStr;

			result = SqlCom::ConnectToDatabase(dbName);
			if (result) return 1;

			queryStr = "INSERT OR IGNORE INTO " + table + " (" + column + ") VALUES (?)";
			auto statement = SqlCom::GetStatement(queryStr, result);
			if (result) return 1;

			result = Bind(statement, 1, value);

			result = SqlCom::Query(statement);
			if (result) return 1;

			return SqlCom::CloseConnection();
		}
		/// <summary>
		/// Removes an existing row from a table
		/// </summary>
		/// <param name="dbName">name of database</param>
		/// <param name="tableName">name of table</param>
		/// <param name="rowKeyColumnName">name of column to determine row(s) (WHERE)</param>
		/// <param name="rowKeyValue">value to determine by, which rows that should be deleted</param>
		/// <returns>int; 0 = OK</returns>
		static int RemoveRow(const std::string dbName, const std::string& tableName, const std::string& rowKeyColumnName, std::string rowKeyValue) {
			int result = 0;

			result = CheckIfRowExists(dbName, tableName, rowKeyColumnName, rowKeyValue);
			if (!result) return 1;

			result = SqlCom::ConnectToDatabase(dbName);
			if (result) return 1;

			std::string query = "DELETE FROM " + tableName + " WHERE " + rowKeyColumnName + " = ?";
			auto statement = SqlCom::GetStatement(query, result);
			if (result) return 1;

			result = ValueBinder::Bind(statement, 1, rowKeyValue);
			if (result) return 1;

			return SqlCom::Query(statement);
		}
	}; // Class DbMod

	/// <summary>
	/// To set and update data in table of a database
	/// </summary>
	class DataSetter : private ValueBinder, private SqlCom, private TableInfo {
	private:
		/// <summary>
		/// templated method to store data into table of database
		/// </summary>
		/// <typeparam name="ValueType">value to be stored</typeparam>
		/// <typeparam name="IdentifierType">type of column to determine location of field</typeparam>
		/// <param name="dbName">name of database</param>
		/// <param name="tableName">name of table</param>
		/// <param name="valueColumnName">name of column for value to be stored</param>
		/// <param name="value">value to be stored</param>
		/// <param name="valueType">type of value to be stored</param>
		/// <param name="identifierColumnName">column for checking, where to store data into</param>
		/// <param name="identifierValue">value for idColumn to check against</param>
		/// <returns>int; 0 = OK</returns>
		template <typename ValueType, typename IdentifierType>
		static int storeData(
			const std::string dbName,
			const std::string& tableName,
			const std::string& valueColumnName,
			const ValueType& value,
			const std::string valueType,
			const std::string& identifierColumnName,
			const IdentifierType& identifierValue
		) {
			int result;
			auto database = SqlCom::ConnectToDatabase(dbName);
			std::string query = "UPDATE " + tableName + " SET " + valueColumnName + " = ? WHERE " + identifierColumnName + " = ?";

			result = CompareColumnType(dbName, tableName, valueColumnName, valueType);
			if (result) return 1;

			auto stmt = SqlCom::GetStatement(query, result);
			if (result) return 1;

			result = Bind(stmt, 1, value);  // Bind the value
			if (result) return 1;

			result = Bind(stmt, 2, identifierValue);  // Bind the identifier value
			if (result) return 1;

			result = SqlCom::Query(stmt);
			if (result) return 1;

			return SqlCom::CloseConnection();
		}

	public:
		/// <summary>
		/// Stores Data into table of database
		/// </summary>
		/// <param name="dbName">name of db</param>
		/// <param name="tableName">name of table</param>
		/// <param name="valueColumnName">column of value to be stored</param>
		/// <param name="value">value to be stored</param>
		/// <param name="identifierColumnName">column to determine location of value to be stored</param>
		/// <param name="identifierValue">value from column to determine location of value to be stored</param>
		/// <returns>int; 0 = OK </returns>
		static int StoreData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName, const std::string& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeData(dbName, tableName, valueColumnName, value, TEXT, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores Data into table of database
		/// </summary>
		/// <param name="dbName">name of db</param>
		/// <param name="tableName">name of table</param>
		/// <param name="valueColumnName">column of value to be stored</param>
		/// <param name="value">value to be stored</param>
		/// <param name="identifierColumnName">column to determine location of value to be stored</param>
		/// <param name="identifierValue">value from column to determine location of value to be stored</param>
		/// <returns>int; 0 = OK </returns>
		static int StoreData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName, int value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeData(dbName, tableName, valueColumnName, value, INTEGER, identifierColumnName, identifierValue);
		}
		/// <summary>
		/// Stores Data into table of database
		/// </summary>
		/// <param name="dbName">name of db</param>
		/// <param name="tableName">name of table</param>
		/// <param name="valueColumnName">column of value to be stored</param>
		/// <param name="value">value to be stored</param>
		/// <param name="identifierColumnName">column to determine location of value to be stored</param>
		/// <param name="identifierValue">value from column to determine location of value to be stored</param>
		/// <returns>int; 0 = OK </returns>
		static int StoreData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName, const std::vector<std::string>& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeData(dbName, tableName, valueColumnName, value, BLOB, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores Data into table of database
		/// </summary>
		/// <param name="dbName">name of db</param>
		/// <param name="tableName">name of table</param>
		/// <param name="valueColumnName">column of value to be stored</param>
		/// <param name="value">value to be stored</param>
		/// <param name="identifierColumnName">column to determine location of value to be stored</param>
		/// <param name="identifierValue">value from column to determine location of value to be stored</param>
		/// <returns>int; 0 = OK </returns>
		static int StoreData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName, const std::vector<int>& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeData(dbName, tableName, valueColumnName, value, BLOB, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores Data into table of database
		/// </summary>
		/// <param name="dbName">name of db</param>
		/// <param name="tableName">name of table</param>
		/// <param name="valueColumnName">column of value to be stored</param>
		/// <param name="value">value to be stored</param>
		/// <param name="identifierColumnName">column to determine location of value to be stored</param>
		/// <param name="identifierValue">value from column to determine location of value to be stored</param>
		/// <returns>int; 0 = OK </returns>
		static int StoreData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName,
			const std::vector<std::pair<int, int>>& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeData(dbName, tableName, valueColumnName, value, BLOB, identifierColumnName, identifierValue);
		}
	};

	/// <summary>
	/// To get data out of tables from a database
	/// </summary>
	class DataGetter : private SqlCom, private DataDeserializer, private ValueBinder {
	private:
		/// <summary>
		/// Extracts data from Field variant
		/// </summary>
		/// <param name="field">reference of variant object</param>
		/// <param name="fieldData">reference of object to store data into</param>
		/// <returns>int; 0 = OK</returns>
		static int GetFieldData(
			Field& field,
			std::string& fieldData
		) {
			try {
				fieldData = std::get<std::string>(field);
			}
			catch (const std::exception&)
			{
				return 1;
			}

			if (fieldData.empty()) return 1;

			return 0;
		}

		/// <summary>
		/// Extracts data from Field variant
		/// </summary>
		/// <param name="field">reference of variant object</param>
		/// <param name="fieldData">reference of object to store data into</param>
		/// <returns>int; 0 = OK</returns>
		static int GetFieldData(
			Field& field,
			int& fieldData
		) {
			try {
				fieldData = std::get<int>(field);
			}
			catch (const std::exception&) {
				return 1;
			}

			return 0;
		}

		/// <summary>
		/// Extracts data from Field variant
		/// </summary>
		/// <param name="field">reference of variant object</param>
		/// <param name="fieldData">reference of object to store data into</param>
		/// <returns>int; 0 = OK</returns>
		static int GetFieldData(
			Field& field,
			std::vector<int>& fieldData
		) {

			std::vector<char> blob = std::get<std::vector<char>>(field);
			if (blob.empty()) return 1;

			try {
				fieldData = DeserializeVectorInt(blob);
			}
			catch (const std::exception&) {
				return 1;
			}

			if (fieldData.empty()) return 1;

			return 0;
		}

		/// <summary>
		/// Extracts data from Field variant
		/// </summary>
		/// <param name="field">reference of variant object</param>
		/// <param name="fieldData">reference of object to store data into</param>
		/// <returns>int; 0 = OK</returns>
		static int GetFieldData(
			Field& field,
			std::vector<std::string>& fieldData
		) {
			std::vector<char> blob = std::get<std::vector<char>>(field);
			if (blob.empty()) return 1;

			try {
				fieldData = DeserializeVectorString(blob);
			}
			catch (const std::exception&) {
				return 1;
			}

			if (fieldData.empty()) return 1;

			return 0;
		}

		/// <summary>
		/// Extracts data from Field variant
		/// </summary>
		/// <param name="field">reference of variant object</param>
		/// <param name="fieldData">reference of object to store data into</param>
		/// <returns>int; 0 = OK</returns>
		static int GetFieldData(
			Field& field,
			std::vector<std::pair<int, int>>& fieldData
		) {
			std::vector<char> blob = std::get<std::vector<char>>(field);
			if (blob.empty()) return 1;

			try {
				fieldData = DeserializeVectorPair(blob);
			}
			catch (const std::exception&) {
				return 1;
			}
			if (fieldData.empty()) return 1;

			return 0;
		}

		/// <summary>
		/// templated method to load data from table in database
		/// </summary>
		/// <typeparam name="Type">Type of field. deduced by reference given</typeparam>
		/// <param name="stmt">statement to query database</param>
		/// <param name="dataVector">vector of data to load from database</param>
		/// <returns>int; 0 = OK </returns>
		template <typename Type>
		static int loadFromTable(
			sqlite3_stmt* stmt,
			std::vector<Type>& dataVector
		) {
			int result = 0;
			Table resultSet;

			result = SqlCom::Query(stmt, resultSet);
			if (result) return 1;

			if (resultSet.empty()) return 1;

			for (size_t i = 1; i < resultSet.size(); i++)
			{
				Row& row = resultSet[i];

				if (row.empty())
				{
					continue;
				}

				Field field = row[0];
				Type fieldVar;

				result = GetFieldData(field, fieldVar);
				if (result) return 1;

				dataVector.push_back(fieldVar);
			}

			return 0;
		}

	public:
		/// <summary>
		/// Loads data from database
		/// </summary>
		/// <typeparam name="Ret_Type">Type of loaded data. deduced by objectreference</typeparam>
		/// <typeparam name="IdentifierType">type of column in table thich is use to determine the location of the value to load</typeparam>
		/// <param name="dbName">name of db</param>
		/// <param name="table">name of table</param>
		/// <param name="valueColumn">column, from where the value should be loaded</param>
		/// <param name="identifierColumn">column to identify location of desired value</param>
		/// <param name="identifierValue">value to determine row of desired value</param>
		/// <param name="dataVector">reference to vector of objects to store results into. used to deduce Ret_Type</param>
		/// <returns>int; 0 = OK </returns>
		template <typename Ret_Type, typename IdentifierType>
		static int LoadData(
			const std::string dbName,
			const std::string table,
			const std::string valueColumn,
			std::string identifierColumn,
			const IdentifierType identifierValue,
			Ret_Type& dataVector
		) {
			int result = 0;
			//std::string queryStr = "SELECT " + valueColumn + " FROM " + table +
			//	" WHERE " + identifierColumn + " =?;";

			std::string queryStr = "SELECT " + valueColumn + " FROM " + table +
				" WHERE " + identifierColumn + " = '" + identifierValue + "';";

			result = SqlCom::ConnectToDatabase(dbName);
			if (result) return 1;

			auto stmt = SqlCom::GetStatement(queryStr, result);
			if (result) return 1;

			//result = Bind(stmt, 1, identifierValue);
			//if (result) return 1;

			result = loadFromTable(stmt, dataVector);
			if (result) return 1;

			return SqlCom::CloseConnection();
		}

		/// <summary>
		/// Loads data from database
		/// </summary>
		/// <typeparam name="Ret_Type">Type of loaded data. deduced by objectreference</typeparam>
		/// <param name="dbName">name of db</param>
		/// <param name="table">name of table</param>
		/// <param name="valueColumn">column, from where the value should be loaded</param>
		/// <param name="dataVector">reference to vector of objects to store results into. used to deduce Ret_Type</param>
		/// <returns>int; 0 = OK </returns>
		template <typename Ret_Type>
		static int LoadData(
			const std::string dbName,
			const std::string table,
			const std::string valueColumn,
			Ret_Type& dataVector
		) {
			int result;

			std::string queryStr = "SELECT " + valueColumn + " FROM " + table;
			result = SqlCom::ConnectToDatabase(dbName);
			if (result) return 1;

			sqlite3_stmt* stmt = SqlCom::GetStatement(queryStr, result);
			if (result) return 1;

			result = loadFromTable(stmt, dataVector);
			if (result) return 1;

			return SqlCom::CloseConnection();
		}
	};
} // namespace SqliteHandler