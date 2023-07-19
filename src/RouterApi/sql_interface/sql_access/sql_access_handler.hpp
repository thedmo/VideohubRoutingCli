#pragma once

#include <sql_access.hpp>
#include <istream>
#include <iostream>
#include <variant>


using namespace sqlAccess;
using Field =
std::variant<
	int,
	std::string,
	std::vector<char*>,
	std::variant<std::monostate>
>;
using Row = std::vector<Field>;
using Table = std::vector<Row>;


namespace SqliteHandler {
	template<typename T>
	using DataContainer = std::vector<T>;

	class SqlCom {
	private:
		static inline sqlite3* m_database;

	public:
		static inline const std::string V_CHAR = "VARCHAR";
		static inline const std::string INT = "INT";
		static inline const std::string BLOB = "BLOB";

		/// <summary>
		/// Connects to database located at same location as executable
		/// </summary>
		/// <param name="dbName">name of the database</param>
		/// <param name="db">database object</param>
		/// <returns>sqlite errorcode. 0 = SQLITE_OK</returns>
		static int ConnectToDatabase(const std::string dbName) {
			int result = 0;
			std::string path = whereami::getExecutablePath().dirname();
			path = path + "/" + dbName + ".db";
			result = sqlite3_open(path.c_str(), &m_database);

			return result;
		}

		/// <summary>
		/// Close connection to database
		/// </summary>
		/// <param name="db">raw pointer of databasse to be closed</param>
		/// <returns>sqlite errorcode. 0 = SQLITE_OK</returns>
		static int CloseConnection() {
			return sqlite3_close_v2(m_database);
		}

		static int Cleanup(sqlite3_stmt* statement) {
			int result;
			result = sqlite3_finalize(statement);
			if (result) return 1;

			return sqlite3_close_v2(m_database);
		}

	private:

		static Field GetColumnNameField(sqlite3_stmt* statement, int index) {
			Field field = std::variant<std::monostate >();

			field = (std::string)((char*)sqlite3_column_name(statement, index));

			return field;
		}

		static Field GetField(sqlite3_stmt* statement, int index) {
			Field field = std::variant<std::monostate >();

			// VARCHAR
			if (sqlite3_column_type(statement, index) == SQLITE_TEXT)
			{
				//char* chars = (char*)sqlite3_column_text(statement, colNum);
				field = (std::string)((char*)sqlite3_column_text(statement, index));
			}

			// INT
			else if (sqlite3_column_type(statement, index) == SQLITE_INTEGER)
			{
				field = sqlite3_column_int(statement, index);
			}

			// BLOB
			else if (sqlite3_column_type(statement, index) == SQLITE_BLOB)
			{
				field = (char*)sqlite3_column_blob(statement, index);
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
		/// <param name="dbName">name of database</param>
		/// <param name="statement">prepared statement</param>
		/// <param name="resultRowCount">reference to int to retrieve rowcount of result set</param>
		/// <returns>sqlite errcode. 0 = SQLITE_OK</returns>
		static int Query(sqlite3_stmt* statement, int& rowCount) {
			Table dataVariantVector;
			return query(statement, rowCount, dataVariantVector);
		}

		/// <summary>
		/// Query database with prepared statement
		/// </summary>
		/// <param name="dbName">name of database</param>
		/// <param name="statement">prepared statement</param>
		/// <param name="resultTable">reference to vector to retrieve data as strings</param>
		/// <returns>sqlite errcode. 0 = SQLITE_OK</returns>
		static int Query(sqlite3_stmt* statement, Table& dataVariantVector) {
			int tempInt;
			return query(statement, tempInt, dataVariantVector);
		}

		/// <summary>
		/// Query database with prepared statement
		/// </summary>
		/// <param name="dbName">name of database</param>
		/// <param name="statement">prepared statement</param>
		/// <returns>sqlite errcode. 0 = SQLITE_OK</returns>
		static int Query(sqlite3_stmt* statement) {
			int tempInt;
			Table dataVariantVector;

			return query(statement, tempInt, dataVariantVector);
		}

	private:
		static sqlite3_stmt* getStatement(const std::string query, int& result) {
			sqlite3_stmt* statement;
			result = sqlite3_prepare_v2(m_database, query.c_str(), -1, &statement, 0);
			return statement;
		}

	public:
		/// <summary>
		/// Get Statement for query string
		/// </summary>
		/// <param name="dbName">name of database statement is for</param>
		/// <param name="queryStr">string with sql query</param>
		/// <returns>sqlite3 statement pointer</returns>
		static sqlite3_stmt* GetStatement(/*sqlite3* database, */std::string queryStr) {
			int result;
			return getStatement(queryStr, result);
		}

		/// <summary>
		/// Get Statement for query string
		/// </summary>
		/// <param name="dbName">name of database statement is for</param>
		/// <param name="queryStr">string with sql query</param>
		/// <returns>sqlite3 statement pointer</returns>
		static sqlite3_stmt* GetStatement(std::string queryStr, int& result) {
			return getStatement(queryStr, result);
		}
	};

	// Serialize containers to insert the into database
	class DataSerializer {
	public:
		// Vector with strings
		static std::vector<char> SerializeVectorString(const std::vector<std::string>& vec) {
			std::string serializedData;
			for (const std::string& str : vec) {
				serializedData += str + '\n';  // Use a newline character as a delimiter
			}
			return std::vector<char>(serializedData.begin(), serializedData.end());
		}

		// Vector with ints
		static std::vector<char> SerializeVectorInt(const std::vector<int>& vec) {
			const char* buffer = reinterpret_cast<const char*>(vec.data());
			const size_t size = vec.size() * sizeof(int);
			return std::vector<char>(buffer, buffer + size);
		}

		// Vector with pairs of ints
		static std::vector<char> SerializeVectorTuple(const std::vector<std::tuple<int, int>>& vec) {
			std::vector<char> serializedData;
			serializedData.reserve(vec.size() * sizeof(std::tuple<int, int>));
			for (const auto& tuple : vec) {
				const char* tupleData = reinterpret_cast<const char*>(&tuple);
				serializedData.insert(serializedData.end(), tupleData, tupleData + sizeof(std::pair<int, int>));
			}
			return serializedData;
		}

		// Vector with pairs of ints
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

	// deserialization of containertypes from database
	class DataDeserializer {
	public:
		// vector<string>
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

		// vector<int>
		static std::vector<int> DeserializeVectorInt(const std::vector<char>& serializedData) {
			const int* buffer = reinterpret_cast<const int*>(serializedData.data());
			const size_t size = serializedData.size() / sizeof(int);
			return std::vector<int>(buffer, buffer + size);
		}

		// Deserialization for vector<pair<int, int>>
		static std::vector<std::pair<int, int>> deserializeVectorPair(const std::vector<char>& serializedData) {
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

	// Bind values
	class ValueBinder : private DataSerializer {
	public:
		// Bind an integer value
		static int Bind(sqlite3_stmt* stmt, int index, int value) {
			return sqlite3_bind_int(stmt, index, value);
		}

		// Bind a string value
		static int Bind(sqlite3_stmt* stmt, int index, const std::string& value) {
			return sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_STATIC);
		}

		// Bind a vector of strings
		static int Bind(sqlite3_stmt* stmt, int index, const std::vector<std::string>& container) {
			auto serializedData = SerializeVectorString(container);

			return sqlite3_bind_blob(stmt, index, serializedData.data(), static_cast<int>(serializedData.size()), SQLITE_TRANSIENT);
		}

		// Bind a vector of integers
		static int Bind(sqlite3_stmt* stmt, int index, const std::vector<int>& container) {
			auto serializedData = SerializeVectorInt(container);

			return sqlite3_bind_blob(stmt, index, serializedData.data(), static_cast<int>(serializedData.size()), SQLITE_TRANSIENT);
		}

		// Bind a vector of pair of two integers
		static int Bind(sqlite3_stmt* stmt, int index, const std::vector<std::pair<int, int>>& container) {
			auto serializedData = SerializeVectorPair(container);

			return sqlite3_bind_blob(stmt, index, serializedData.data(), static_cast<int>(serializedData.size()), SQLITE_TRANSIENT);
		}
	};

	class DbMod : private SqlCom, private ValueBinder {
	public:


		DbMod() {  };

		static int CreateTableWithPrimaryKey(std::string dbName, std::string tableName, std::string primaryKeyName, const std::string primaryKeyType) {
			int result;
			std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " (" + primaryKeyName + " " + primaryKeyType + " PRIMARY KEY);";
			auto database = SqlCom::ConnectToDatabase(dbName);
			auto statement = SqlCom::GetStatement(query);

			result = SqlCom::Query(statement);
			if (result) return 1;

			return SqlCom::CloseConnection();
		}

		static int CreateTableWithForeignKey(std::string dbName, std::string tableName, std::string foreignKeyColumnName, std::string foreignKeyColumnType, std::string primaryKeyTableName, std::string primaryKeyColumnName) {
			int result;
			std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " (" + foreignKeyColumnName + " " + foreignKeyColumnType + ", FOREIGN KEY(" + foreignKeyColumnName + ") REFERENCES " + primaryKeyTableName + "(" + primaryKeyColumnName + "));";
			auto database = SqlCom::ConnectToDatabase(dbName);
			auto statement = SqlCom::GetStatement(query);

			result = SqlCom::Query(statement);
			if (result) return 1;

			return SqlCom::CloseConnection();
		}

	private:
		static std::vector<std::string> GetColumnNames(std::string dbName, std::string tableName, int& result) {
			std::vector<std::string> columns;
			Table queryResultSet;

			result = SqlCom::ConnectToDatabase(dbName);
			if (result) return columns;

			std::string query = "SELECT * FROM " + tableName + ";";

			auto statement = SqlCom::GetStatement(query, result);
			if (result) return columns;

			result = SqlCom::Query(statement, queryResultSet);
			if (result) return columns;

			if (queryResultSet.empty()) return columns.push_back(""), columns;

			Row rowZero = queryResultSet[0];
			for (size_t i = 0; i < rowZero.size(); i++)
			{
				auto& field = rowZero[i];
				columns.push_back(std::get<std::string>(field));
			}

			result = SqlCom::CloseConnection();
			if (result) return columns;

			return columns;
		}

	protected:
		/// <summary>
		/// Check, if columnName is in specified table
		/// </summary>
		/// <param name="dbName">name of database</param>
		/// <param name="tableName">name of table</param>
		/// <param name="columnNameToLookFor">name of column to look for</param>
		/// <returns>0 if name of column is found in table</returns>
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

	public:
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

	}; // Class DbMod

	class DataSetter : private ValueBinder, private SqlCom, private DbMod {
	public:
		DataSetter() {};

		static int InsertRow(const std::string dbName, const std::string& table, const std::string& column, std::string value) {
			int result = 0;
			int rowCount = 0;
			std::string queryStr;
			auto database = SqlCom::ConnectToDatabase(dbName);

			queryStr = "INSERT OR IGNORE INTO " + table + " (" + column + ") VALUES (?)";
			auto statement = SqlCom::GetStatement(queryStr);

			Bind(statement, 1, value);

			result = SqlCom::Query(statement);
			if (result) return 1;

			return SqlCom::CloseConnection();
		}

		static int RemoveRow(const std::string dbName, const std::string& tableName, const std::string& rowKeyColumnName, std::string rowKeyValue) {
			int result = 0;

			result = DbMod::CheckIfColumnExists(dbName, tableName, rowKeyColumnName);
			if (!result) return 1;

			result = DbMod::CheckIfRowExists(dbName, tableName, rowKeyColumnName, rowKeyValue);
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

	private:
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
		/// Helperfunction that Stores Data into a Sqlite database. deduces datatype
		/// </summary>
		/// <returns>int, should be 0</returns>
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
		// Overloaded functions to store data into database

		/// <summary>
		/// Stores data into database
		/// </summary>
		/// <param name="value">string</param>
		/// <returns>int, should be 0</returns>
		static int StoreData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName, const std::string& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeData(dbName, tableName, valueColumnName, value, SqlCom::V_CHAR, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores data into database
		/// </summary>
		/// <param name="value">int</param>
		/// <returns>int, should be 0</returns>
		static int StoreData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName, int value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeData(dbName, tableName, valueColumnName, value, SqlCom::INT, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores data into database
		/// </summary>
		/// <param name="value">vector(string)</param>
		/// <returns>int, should be 0</returns>
		static int StoreData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName, const std::vector<std::string>& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeData(dbName, tableName, valueColumnName, value, SqlCom::BLOB, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores data into database
		/// </summary>
		/// <param name="value">vector(int)</param>
		/// <returns>int, should be 0</returns>
		static int StoreData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName, const std::vector<int>& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeData(dbName, tableName, valueColumnName, value, SqlCom::BLOB, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores data into database
		/// </summary>
		/// <param name="value">pair(int,int)</param>
		/// <returns>int, should be 0</returns>
		static int StoreData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName,
			const std::vector<std::pair<int, int>>& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeData(dbName, tableName, valueColumnName, value, SqlCom::BLOB, identifierColumnName, identifierValue);
		}
	};

	// Retrieve Data from database
	class DataGetter : private SqlCom, private DataDeserializer, private ValueBinder {

	public:
		DataGetter() {};

	private:
		// int
		static int retrieveData(sqlite3_stmt* statement, int& data, int columnIndex) {
			data = sqlite3_column_int(statement, columnIndex);
			return 0;
		}

		// string
		static int retrieveData(sqlite3_stmt* statement, std::string& data, int columnIndex) {
			const unsigned char* textData = sqlite3_column_text(statement, columnIndex);
			if (textData) {
				data = std::string(reinterpret_cast<const char*>(textData));
			}
			else {
				data = "";  // Return an empty string if the value is NULL
				return 1;
			}

			return 0;
		}

		// vector int
		static int retrieveData(sqlite3_stmt* statement, std::vector<int>& data, int columnIndex) {
			const std::vector<char> blobData(
				static_cast<const char*>(sqlite3_column_blob(statement, columnIndex)),
				static_cast<const char*>(sqlite3_column_blob(statement, columnIndex)) +
				sqlite3_column_bytes(statement, columnIndex));
			if (blobData.empty()) {
				return 1;
			}

			data = DeserializeVectorInt(blobData);
			return 0;
		}

		// vector string
		static int retrieveData(sqlite3_stmt* statement, std::vector<std::string>& data, int columnIndex) {
			const std::vector<char> blobData(
				static_cast<const char*>(sqlite3_column_blob(statement, columnIndex)),
				static_cast<const char*>(sqlite3_column_blob(statement, columnIndex)) +
				sqlite3_column_bytes(statement, columnIndex));
			if (blobData.empty()) {
				return 1;
			}

			data = DeserializeVectorString(blobData);
			return 0;
		}

		// vector pair int int
		static int retrieveData(sqlite3_stmt* statement, std::vector<std::pair<int, int>>& data, int columnIndex) {
			const std::vector<char> blobData(
				static_cast<const char*>(sqlite3_column_blob(statement, columnIndex)),
				static_cast<const char*>(sqlite3_column_blob(statement, columnIndex)) +
				sqlite3_column_bytes(statement, columnIndex));
			if (blobData.empty()) {
				return 1;
			}

			data = deserializeVectorPair(blobData);
			return 0;
		}

		// Helperfunction to load data
		template <typename ValueType>
		static int loadData(
			sqlite3_stmt* stmt,
			std::vector<ValueType>& dataVector
		) {
			int result;
			int sql_result;

			do {
				sql_result = sqlite3_step(stmt);

				ValueType data;

				result = retrieveData(stmt, data, 0);  // Assuming the value is in the first column (index 0)
				if (result != 0) {
					return 1;
				}

				dataVector.push_back(data);

			} while (sql_result != SQLITE_DONE);

			sqlite3_finalize(stmt);
			return 0;
		}

	public:
		// Templated with identifier
		template <typename Ret_Type, typename IdentifierType>
		static int LoadData(
			const std::string dbName,
			const std::string table,
			const std::string valueColumn,
			std::string identifierColumn,
			const IdentifierType& identifierValue,
			Ret_Type& dataVector
		) {
			std::string queryStr = "SELECT " + valueColumn + " FROM " + table +
				" WHERE " + identifierColumn + " = ?";
			auto database = SqlCom::ConnectToDatabase(dbName);
			auto stmt = SqlCom::GetStatement(queryStr);

			Bind(stmt, 1, identifierValue);
			loadData(stmt, dataVector);
			return SqlCom::CloseConnection();
		}

		// Templated without identifier
		template <typename Ret_Type>
		static int LoadData(
			const std::string dbName,
			const std::string table,
			const std::string valueColumn,
			Ret_Type& dataVector
		) {
			std::string queryStr = "SELECT " + valueColumn + " FROM " + table;
			auto database = SqlCom::ConnectToDatabase(dbName);

			sqlite3_stmt* stmt = SqlCom::GetStatement(queryStr);

			loadData(stmt, dataVector);
			return SqlCom::CloseConnection();
		}
	};


} // namespace SqliteHandler