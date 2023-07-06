#pragma once

#include <sql_access.hpp>

namespace SqliteHandler {
	template<typename T>
	using DataContainer = std::vector<T>;

	// Serialize containers to insert the into database
	class DataSerializer {
	public:
		// Vector with strings
		static std::vector<char> serializeVectorString(const std::vector<std::string>& vec) {
			std::string serializedData;
			for (const std::string& str : vec) {
				serializedData += str + '\n';  // Use a newline character as a delimiter
			}
			return std::vector<char>(serializedData.begin(), serializedData.end());
		}

		// Vector with ints
		static std::vector<char> serializeVectorInt(const std::vector<int>& vec) {
			const char* buffer = reinterpret_cast<const char*>(vec.data());
			const size_t size = vec.size() * sizeof(int);
			return std::vector<char>(buffer, buffer + size);
		}

		// Vector with tuples with ints
		static std::vector<char> serializeVectorTuple(const std::vector<std::tuple<int, int>>& vec) {
			std::vector<char> serializedData;
			serializedData.reserve(vec.size() * sizeof(std::tuple<int, int>));
			for (const auto& tuple : vec) {
				const char* tupleData = reinterpret_cast<const char*>(&tuple);
				serializedData.insert(serializedData.end(), tupleData, tupleData + sizeof(std::tuple<int, int>));
			}
			return serializedData;
		}
	};

	// deserialization of containertypes from database
	class DataDeserializer {
	public:
		// vector<string>
		static std::vector<std::string> deserializeVectorString(const std::vector<char>& serializedData) {
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
		static std::vector<int> deserializeVectorInt(const std::vector<char>& serializedData) {
			const int* buffer = reinterpret_cast<const int*>(serializedData.data());
			const size_t size = serializedData.size() / sizeof(int);
			return std::vector<int>(buffer, buffer + size);
		}

		// vector<tuple<int,int>>
		static std::vector<std::tuple<int, int>> deserializeVectorTuple(const std::vector<char>& serializedData) {
			std::vector<std::tuple<int, int>> deserializedData;
			const char* data = serializedData.data();
			const size_t size = serializedData.size();
			const size_t tupleSize = sizeof(std::tuple<int, int>);
			if (size % tupleSize == 0) {
				for (size_t i = 0; i < size; i += tupleSize) {
					const std::tuple<int, int>* tuple = reinterpret_cast<const std::tuple<int, int>*>(data + i);
					deserializedData.push_back(*tuple);
				}
			}
			return deserializedData;
		}
	};

	// Bind values
	class ValueBinder : DataSerializer{
	public:
		// Bind an integer value
		static void bindValue(sqlite3_stmt* stmt, int index, int value) {
			sqlite3_bind_int(stmt, index, value);
		}

		// Bind a string value
		static void bindValue(sqlite3_stmt* stmt, int index, const std::string& value) {
			sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_STATIC);
		}

		// Bind a vector of strings
		static void bindValue(sqlite3_stmt* stmt, int index, const std::vector<std::string>& container) {
			auto serializedData = serializeVectorString(container);

			sqlite3_bind_blob(stmt, index, serializedData.data(), static_cast<int>(serializedData.size()), SQLITE_TRANSIENT);
		}

		// Bind a vector of integers
		static void bindValue(sqlite3_stmt* stmt, int index, const std::vector<int>& container) {
			auto serializedData = serializeVectorInt(container);

			sqlite3_bind_blob(stmt, index, serializedData.data(), static_cast<int>(serializedData.size()), SQLITE_TRANSIENT);
		}

		// Bind a vector of tuples of two integers
		static void bindValue(sqlite3_stmt* stmt, int index, const std::vector<std::tuple<int, int>>& container) {
			auto serializedData = serializeVectorTuple(container);

			sqlite3_bind_blob(stmt, index, serializedData.data(), static_cast<int>(serializedData.size()), SQLITE_TRANSIENT);
		}
	};


	class SqliteDataSetter : ValueBinder {
	public:
		static int insertRow(const std::string db_name, const std::string& tableName, const std::string& rowKeyColumnName, std::string rowKeyValue) {
			int result;
			sql_access db(db_name);
			//std::string query = "INSERT INTO " + tableName + " (" + rowKeyColumnName + ") VALUES ('" + rowKeyValue + "')";

			std::string query = "INSERT INTO " + tableName + " (" + rowKeyColumnName + ") VALUES (?)";
			auto statement = db.GetStatement(query);

			bindValue(statement, 1, rowKeyValue);

			result = db.Query(statement);
			if (result != 0)
			{
				return 1;
			}

			return 0;
		}

		static int removeRow(const std::string db_name, const std::string& tableName, const std::string& rowKeyColumnName, std::string rowKeyValue) {
			int result;
			sql_access db(db_name);
			std::string query = "DELETE FROM " + tableName + " WHERE " + rowKeyColumnName + " = ?";

			auto statement = db.GetStatement(query);

			bindValue(statement, 1, rowKeyValue);

			result = db.Query(statement);
			if (result != 0)
			{
				return 1;
			}

			return 0;
		}

		/// <summary>
		/// Helperfunction that Stores Data into a Sqlite database. deduces datatype
		/// </summary>
		/// <returns>int, should be 0</returns>
		template <typename ValueType, typename IdentifierType>
		static int storeDataHelper(const std::string db_name, const std::string& tableName, const std::string& valueColumnName, const ValueType& value,
			const std::string& identifierColumnName, const IdentifierType& identifierValue) {

			int result;
			sql_access db(db_name);
			std::string query = "UPDATE " + tableName + " SET " + valueColumnName + " = ? WHERE " + identifierColumnName + " = ?";

			auto stmt = db.GetStatement(query);
			bindValue(stmt, 1, value);  // Bind the value
			bindValue(stmt, 2, identifierValue);  // Bind the identifier value

			return result = db.Query(stmt);
		}

	public:
		// Overloaded functions to store data into database

		/// <summary>
		/// Stores data into database
		/// </summary>
		/// <param name="value">string</param>
		/// <returns>int, should be 0</returns>
		static int storeData(const std::string db_name, const std::string& tableName, const std::string& valueColumnName, const std::string& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeDataHelper(db_name, tableName, valueColumnName, value, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores data into database
		/// </summary>
		/// <param name="value">int</param>
		/// <returns>int, should be 0</returns>
		static int storeData(const std::string db_name, const std::string& tableName, const std::string& valueColumnName, int value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeDataHelper(db_name, tableName, valueColumnName, value, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores data into database
		/// </summary>
		/// <param name="value">vector(string)</param>
		/// <returns>int, should be 0</returns>
		static int storeData(const std::string db_name, const std::string& tableName, const std::string& valueColumnName, const std::vector<std::string>& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeDataHelper(db_name, tableName, valueColumnName, value, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores data into database
		/// </summary>
		/// <param name="value">vector(int)</param>
		/// <returns>int, should be 0</returns>
		static int storeData(const std::string db_name, const std::string& tableName, const std::string& valueColumnName, const std::vector<int>& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeDataHelper(db_name, tableName, valueColumnName, value, identifierColumnName, identifierValue);
		}

		/// <summary>
		/// Stores data into database
		/// </summary>
		/// <param name="value">tuple(int,int)</param>
		/// <returns>int, should be 0</returns>
		static int storeData(const std::string db_name, const std::string& tableName, const std::string& valueColumnName,
			const std::vector<std::tuple<int, int>>& value,
			const std::string& identifierColumnName, const std::string& identifierValue) {
			return storeDataHelper(db_name, tableName, valueColumnName, value, identifierColumnName, identifierValue);
		}
	};

	// Retrieve Data from database
	class SqliteDataGetter : sql_access, DataDeserializer, ValueBinder {
	private:
		// int
		static int retrieveData(sqlite3_stmt* statement, int& data, int columnIndex) {
			data = sqlite3_column_int(statement, columnIndex);
			return 0;
		}

		// string
		static int retrieveData(sqlite3_stmt* statement, std::string& data, int columnIndex) {
			const unsigned char * textData = sqlite3_column_text(statement, columnIndex);
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

			data = deserializeVectorInt(blobData);
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

			data = deserializeVectorString(blobData);
			return 0;
		}

		// vector tuple int int
		static int retrieveData(sqlite3_stmt* statement, std::vector<std::tuple<int,int>>& data, int columnIndex) {
			const std::vector<char> blobData(
				static_cast<const char*>(sqlite3_column_blob(statement, columnIndex)),
				static_cast<const char*>(sqlite3_column_blob(statement, columnIndex)) +
				sqlite3_column_bytes(statement, columnIndex));
			if (blobData.empty()) {
				return 1;
			}

			data = deserializeVectorTuple(blobData);
			return 0;
		}

		/// <summary>
		/// helperfunction to load data from database
		/// </summary>
		/// <typeparam name="ValueType">string, int or containertypes</typeparam>
		/// <typeparam name="IdentifierType">string, int or containertypes</typeparam>
		/// <param name="tableName">name of table in database</param>
		/// <param name="valueColumnName">name of column to store data into</param>
		/// <param name="identifierColumnName">name of column to determine row</param>
		/// <param name="identifierValue">value of field in column to determine row</param>
		/// <returns>Value from specified field</returns>
		template <typename ValueType, typename IdentifierType>
		static int loadDataHelper(const std::string& db_name, const std::string& tableName, const std::string& valueColumnName,
			const std::string& identifierColumnName, const IdentifierType& identifierValue, std::vector<ValueType>& dataStore) {
			sql_access db(db_name);
			
			int result;
			int sql_result;
			std::string query = "SELECT " + valueColumnName + " FROM " + tableName +
				" WHERE " + identifierColumnName + " = ?";

			auto stmt = db.GetStatement(query);
			bindValue(stmt, 1, identifierValue);  // Bind the identifier value
			
			do {

			sql_result = sqlite3_step(stmt);

			ValueType data;

			result = retrieveData(stmt, data, 0);  // Assuming the value is in the first column (index 0)
			if (result != 0) {
				return 1;
			}

			dataStore.push_back(data);
			
			} while (sql_result != SQLITE_DONE);

			sqlite3_finalize(stmt);
			return 0;
		}

	public:
		// Overload for int
		template<typename identifier_t>
		static int LoadData(const std::string& db_name, const std::string& tableName, const std::string& valueColumnName,
			const std::string& identifierColumnName, const identifier_t& identifierValue, std::vector<int>& dataStore) {

			int result = loadDataHelper(db_name, tableName, valueColumnName, identifierColumnName, identifierValue, dataStore);

			return 0;
		}

		// Overload for string
		template<typename identifier_t>
		static int LoadData(const std::string& db_name, const std::string& tableName, const std::string& valueColumnName,
			const std::string& identifierColumnName, const identifier_t& identifierValue, std::vector <std::string>& dataStore) {
			int result = loadDataHelper(db_name, tableName, valueColumnName, identifierColumnName, identifierValue, dataStore);
			return 0;
		}

		// Overload for vector<int>
		template<typename identifier_t>
		static int LoadData(const std::string& db_name, const std::string& tableName, const std::string& valueColumnName,
			const std::string& identifierColumnName, const identifier_t& identifierValue, std::vector <std::vector<int>>& dataStore) {
			int result = loadDataHelper(db_name, tableName, valueColumnName, identifierColumnName, identifierValue, dataStore);
			return 0;
		}

		// Overload for vector<string>
		template<typename identifier_t>
		static int LoadData(const std::string& db_name, const std::string& tableName, const std::string& valueColumnName,
			const std::string& identifierColumnName, const identifier_t& identifierValue, std::vector<std::vector<std::string>>& dataStore) {
			int result = loadDataHelper(db_name, tableName, valueColumnName, identifierColumnName, identifierValue, dataStore);
			return 0;
		}

		// Overload for vector<tuple<int, int>>
		template<typename identifier_t>
		static int LoadData(const std::string& db_name, const std::string& tableName, const std::string& valueColumnName,
			const std::string& identifierColumnName, const identifier_t& identifierValue,
			std::vector<std::vector<std::tuple<int, int>>>& dataStore) {
			int result = loadDataHelper(db_name, tableName, valueColumnName, identifierColumnName, identifierValue, dataStore);
			return 0;
		}
	};

	class DatabaseChanger : sql_access, ValueBinder {
	public:
		static inline const std::string V_CHAR = "VARCHAR";
		static inline const std::string INT = "INT";
		static inline const std::string BLOB = "BLOB";

		static int CreateTableWithPrimaryKey(std::string db_name, std::string tableName, std::string primaryKeyName, const std::string primaryKeyType) {
			int result;
			sql_access db(db_name);
			std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " (" + primaryKeyName + " " + primaryKeyType + " PRIMARY KEY)";


			result = db.Query(db.GetStatement(query));
			if (result != 0)
			{
				return 1;
			}

			return 0;
		}

		static int CreateTableWithForeignKey(std::string db_name, std::string tableName, std::string foreignKeyColumnName, std::string foreignKeyColumnType, std::string primaryKeyTableName, std::string primaryKeyColumnName) {
			int result;
			sql_access db(db_name);
			std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " (" + foreignKeyColumnName + " " + foreignKeyColumnType + " FOREIGN KEY(" + foreignKeyColumnName + ") REFERENCES " + primaryKeyTableName + "(" + primaryKeyColumnName + "))";

			result = db.Query(db.GetStatement(query));
			if (result != 0)
			{
				return 1;
			}

			return 0;
		}

		static int AddColumn(std::string db_name, std::string table_name, std::string columnName, std::string columnType) {
			int result;
			sql_access db(db_name);
			std::string query = "ALTER TABLE " + table_name + " ADD COLUMN " + columnName + " " + columnType;


			result = db.Query(db.GetStatement(query));
			if (result != 0)
			{
				return 1;
			}

			return 0;
		}
	};

} // namespace SqliteHandler