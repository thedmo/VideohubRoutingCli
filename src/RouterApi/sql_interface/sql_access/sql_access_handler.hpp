#pragma once

#include <sql_access.hpp>
#include <istream>
#include <iostream>


using namespace sqlAccess;

namespace SqliteHandler {
	template<typename T>
	using DataContainer = std::vector<T>;

	class SqlCom {
	private:
		static inline sqlite3* m_database;
		//static inline sql_access sql;

	public:
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
		/// <summary>
		/// Query Database with prepared statement
		/// </summary>
		/// <param name="statement">prepared statement</param>
		/// <returns>sqlite errcode. 0 = SQLITE_OK</returns>
		static int query(sqlite3_stmt* statement, int& resultRowCount,  std::vector<std::string>& stringVec) {
			resultRowCount = 0;

			while (sqlite3_step(statement) == SQLITE_ROW) {

				resultRowCount++;
				int columnCount = sqlite3_column_count(statement);
				if (columnCount < 1)
				{
					continue;
				}

				if (sqlite3_column_type(statement, 1) == SQLITE_TEXT)
				{
					auto chars = (char*)sqlite3_column_text(statement, 1);

					std::string str(chars);

					stringVec.push_back(str);
				}
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
			std::vector<std::string> tempStringVec;
			return query(statement, rowCount, tempStringVec);
		}

		/// <summary>
		/// Query database with prepared statement
		/// </summary>
		/// <param name="dbName">name of database</param>
		/// <param name="statement">prepared statement</param>
		/// <param name="stringVec">reference to vector to retrieve data as strings</param>
		/// <returns>sqlite errcode. 0 = SQLITE_OK</returns>
		static int Query(sqlite3_stmt* statement, std::vector<std::string>& stringVec) {
			int tempInt;
			return query(statement, tempInt, stringVec);
		}

		/// <summary>
		/// Query database with prepared statement
		/// </summary>
		/// <param name="dbName">name of database</param>
		/// <param name="statement">prepared statement</param>
		/// <returns>sqlite errcode. 0 = SQLITE_OK</returns>
		static int Query(sqlite3_stmt* statement) {
			int tempInt;
			std::vector<std::string> tempStringVec;

			return query(statement, tempInt, tempStringVec);
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


	class DataSetter : private ValueBinder, private SqlCom {
	public:

		DataSetter() {};
		static int InsertRow(const std::string dbName, const std::string& table, const std::string& column, std::string value) {
			int result;
			int rowCount;
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
			int result;

			result = SqlCom::ConnectToDatabase(dbName);
			if (result) return 1;

			std::string query = "DELETE FROM " + tableName + " WHERE " + rowKeyColumnName + " = ?";
			auto statement = SqlCom::GetStatement(query, result);
			if (result) return 1;

			result = Bind(statement, 1, rowKeyValue);
			if (result) return 1;

			return SqlCom::Query(statement);
		}

	private:
		// gibt kein Fehler zurück wenn typ nicht übereinstimmt (TODO: Funktionalität für column typecheck einbauen)

		/// <summary>
		/// Helperfunction that Stores Data into a Sqlite database. deduces datatype
		/// </summary>
		/// <returns>int, should be 0</returns>
		template <typename ValueType, typename IdentifierType>
		static int storeData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName, const ValueType& value,
			const std::string& identifierColumnName, const IdentifierType& identifierValue) {

			int result;
			auto database = SqlCom::ConnectToDatabase(dbName);
			std::string query = "UPDATE " + tableName + " SET " + valueColumnName + " = ? WHERE " + identifierColumnName + " = ?";

			auto stmt = SqlCom::GetStatement(query, result);
			if (result) return 1;

			result = Bind(stmt, 1, value);  // Bind the value
			if (result) return 1;

			result = Bind(stmt, 2, identifierValue);  // Bind the identifier value
			if (result) return 1;

			result = SqlCom::Query(stmt);
			if (result) return 1;

			return SqlCom::CloseConnection(/*database*/);
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
			return storeData(dbName, tableName, valueColumnName, value, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores data into database
		/// </summary>
		/// <param name="value">int</param>
		/// <returns>int, should be 0</returns>
		static int StoreData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName, int value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeData(dbName, tableName, valueColumnName, value, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores data into database
		/// </summary>
		/// <param name="value">vector(string)</param>
		/// <returns>int, should be 0</returns>
		static int StoreData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName, const std::vector<std::string>& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeData(dbName, tableName, valueColumnName, value, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores data into database
		/// </summary>
		/// <param name="value">vector(int)</param>
		/// <returns>int, should be 0</returns>
		static int StoreData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName, const std::vector<int>& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeData(dbName, tableName, valueColumnName, value, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores data into database
		/// </summary>
		/// <param name="value">pair(int,int)</param>
		/// <returns>int, should be 0</returns>
		static int StoreData(const std::string dbName, const std::string& tableName, const std::string& valueColumnName,
			const std::vector<std::pair<int, int>>& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeData(dbName, tableName, valueColumnName, value, identifierColumnName, identifierValue);
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

	class DatabaseChanger : private SqlCom, private ValueBinder {
	public:
		static inline const std::string V_CHAR = "VARCHAR";
		static inline const std::string INT = "INT";
		static inline const std::string BLOB = "BLOB";

		DatabaseChanger() {  };

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

			result = SqlCom::ConnectToDatabase(dbName);
			if (result) return;

			std::string query = "PRAGMA table_info(" + tableName + ");";

			auto statement = SqlCom::GetStatement(query, result);
			if (result) return;

			result = SqlCom::Query(statement, columns);
			if (result) return;

			return columns;
		}

	public:
		static int AddColumn(std::string dbName, std::string tableName, std::string columnName, std::string columnType) {
			int result;

			std::vector<std::string> columns = GetColumnNames(dbName, tableName, result);

			for (auto column : columns) {
				if (column == columnName)
				{
					return 0;
				}
			}

			result = SqlCom::ConnectToDatabase(dbName);
			auto queryStr = "ALTER TABLE " + tableName + " ADD COLUMN " + columnName + " " + columnType + ";";

			auto statement = SqlCom::GetStatement(queryStr, result);
			if (result)	return 1;

			result = SqlCom::Query(statement);
			if (result)	return 1;

			return SqlCom::CloseConnection();
		}
	}; // Class DatabaseChanger

} // namespace SqliteHandler