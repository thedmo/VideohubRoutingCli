#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <filesystem>
#include <sql_access.hpp>



// INITIALIZATION



const std::string TEST_DB_NAME = "test";

/// <summary>
/// initializes test database with a table for testing, insert some data and returns object for communication with it
/// </summary>
/// <returns>database as sql_access object</returns>
sql_access initializeDatabase() {
	sql_access db(TEST_DB_NAME);
	std::string query = "CREATE TABLE IF NOT EXISTS " + TEST_DB_NAME + " (id INT PRIMARY KEY, name VARCHAR, some_count INT, text1 VARCHAR, text2 VARCHAR)";
	db.Query(db.GetStatement(query));

	auto query_str = "INSERT INTO " + TEST_DB_NAME + " (id, name, some_count, text1, text2) VALUES (1, 'one', 4, 'text1_1', 'text2_1')";
	db.Query(db.GetStatement(query_str));
	query_str = "INSERT INTO " + TEST_DB_NAME + " (id, name, some_count, text1, text2) VALUES (2, 'two', 5, 'text1_2', 'text2_2')";
	db.Query(db.GetStatement(query_str));
	query_str = "INSERT INTO " + TEST_DB_NAME + " (id, name, some_count, text1, text2) VALUES (3, 'three', 6, 'text1_3', 'text2_3')";
	db.Query(db.GetStatement(query_str));

	return db;
}

/// <summary>
/// Drops test table from database for clean start of next test
/// </summary>
void cleanUpDatabase() {
	sql_access db(TEST_DB_NAME);
	std::string query = "DROP TABLE " + TEST_DB_NAME;
	db.Query(db.GetStatement(query));
}



// TESTS



TEST_CASE("SqliteInterface constructor creates database file", "[sql_access]") {
	try
	{
	auto db = initializeDatabase();
	REQUIRE(std::filesystem::exists(TEST_DB_NAME + ".db"));

	}
	catch (const std::exception& e)
	{
		// show message for catch 2 with exception
		std::string err_msg = e.what();
		INFO(err_msg)
	}

	cleanUpDatabase();
}

TEST_CASE("GetStatement returns a valid sqlite3_stmt pointer", "[GetStatement]") {
	auto db = initializeDatabase();

	auto query_str = "SELECT * FROM " + TEST_DB_NAME;

	auto statement = db.GetStatement(query_str);
	REQUIRE(statement != nullptr);

	cleanUpDatabase();
}

TEST_CASE("Query returns a valid error code", "[Query]") {
	auto db = initializeDatabase();

	auto query_str = "SELECT * FROM " + TEST_DB_NAME;
	auto statement = db.GetStatement(query_str);
	auto result = db.Query(statement);

	REQUIRE(result == sql_access::SQL_ACCESS_OK);

	cleanUpDatabase();
}

TEST_CASE("GetLastQueryResult returns a valid QueryResult", "[GetLastQueryResult]") {
	auto db = initializeDatabase();

	auto query_str = "SELECT * FROM " + TEST_DB_NAME + " WHERE id = 1";
	auto statement = db.GetStatement(query_str);
	db.Query(statement);

	REQUIRE(db.GetLastQueryResult().size() == 2);
	REQUIRE(db.GetLastQueryResult()[1][0] == "1");
	REQUIRE(db.GetLastQueryResult()[1][1] == "one");
	REQUIRE(db.GetLastQueryResult()[1][2] == "4");
	REQUIRE(db.GetLastQueryResult()[1][3] == "text1_1");
	REQUIRE(db.GetLastQueryResult()[1][4] == "text2_1");

	cleanUpDatabase();
}

TEST_CASE("GetLastRowNum returns a valid row number", "[GetLastRowNum]") {
	auto db = initializeDatabase();

	auto query_str = "SELECT * FROM " + TEST_DB_NAME + " WHERE id = 1";
	auto statement = db.GetStatement(query_str);
	db.Query(statement);

	REQUIRE(db.GetLastRowNum() == 1);

	cleanUpDatabase();
}

TEST_CASE("GetLastErrorMsg returns a valid error message", "[GetLastErrorMsg]") {
	auto db = initializeDatabase();

	auto query_str = "SELECT * FROM " + TEST_DB_NAME + " WHERE id = 1";
	auto statement = db.GetStatement(query_str);
	db.Query(statement);

	REQUIRE(db.GetLastErrorMsg() == "not an error");

	cleanUpDatabase();
}

TEST_CASE("BindValues returns a valid sqlite3_stmt pointer", "[BindValues]") {
	auto db = initializeDatabase();

	auto query_str = "INSERT INTO " + TEST_DB_NAME + " (id, name, some_count, text1, text2) VALUES (?, ?, ?, ?, ?)";
	std::vector<std::string> args = { std::to_string(4), "some_name", std::to_string(7), "text1_4", "text2_4" };

	auto bind_stmt = db.BindValues(args, db.GetStatement(query_str));
	REQUIRE(bind_stmt != nullptr);

	cleanUpDatabase();
}
