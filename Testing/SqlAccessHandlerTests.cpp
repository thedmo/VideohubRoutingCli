#include <TestHeader.h>
#include <catch2/catch.hpp>
#include <sql_access_handler.hpp>

using namespace SqliteHandler;

const std::string DB = "SqlHandlerTestDatabase";

const std::string t1 = "TestTable1";
const std::string t2 = "TestTable2";
const std::string t3 = "TestTable3";

const std::string INTEGER = "INT";
const std::string TEXT = "VARCHAR";
const std::string BLOB = "BLOB";

std::string col1 = "col1";
std::string col2 = "col2";
std::string col3 = "col3";
std::string col4 = "col4";
std::string col5 = "col5";
std::string col6 = "col6";

std::string row1 = "row1";
std::string row2 = "row2";
std::string row3 = "row3";

std::string str1 = "one";
std::string str2 = "two";
std::string str3 = "three";

std::vector <std::string> strVec1 = { str1, str2, str3 };
std::vector <int> intVec1 = { 1,2,3 };
std::vector <std::pair<int, int>> pairVec1 = { { 1, 2 },{ 3, 4 },{ 5, 6 } };

void DropTables() {
	int result = 0;

	result = SqliteHandler::SqlCom::ConnectToDatabase(DB);

	std::string queryStr = "DROP TABLE IF EXISTS TestTable1;";
	auto statement = SqliteHandler::SqlCom::GetStatement(queryStr, result);
	result = SqliteHandler::SqlCom::Query(statement);

	queryStr = "DROP TABLE IF EXISTS TestTable2;";
	statement = SqliteHandler::SqlCom::GetStatement(queryStr, result);
	result = SqliteHandler::SqlCom::Query(statement);

	queryStr = "DROP TABLE IF EXISTS TestTable3;";
	statement = SqliteHandler::SqlCom::GetStatement(queryStr, result);
	result = SqliteHandler::SqlCom::Query(statement);

	result = SqliteHandler::SqlCom::CloseConnection();
}

TEST_CASE("Connect to Database") {
	int result = SqliteHandler::SqlCom::ConnectToDatabase("SqlHandlerTestDatabase");
	REQUIRE(result == 0);
	SqliteHandler::SqlCom::CloseConnection();
}

TEST_CASE("prepare statement") {
	DropTables();

	int result = SqliteHandler::SqlCom::ConnectToDatabase("SqlHandlerTestDatabase");
	auto statement = SqliteHandler::SqlCom::GetStatement("CREATE TABLE IF NOT EXISTS TestTable1 (testColumn VARCHAR PRIMARY KEY);", result);

	REQUIRE(result == 0);
	SqliteHandler::SqlCom::CloseConnection();
}

TEST_CASE("Test Query function") {
	DropTables();
	int result = SqliteHandler::SqlCom::ConnectToDatabase("SqlHandlerTestDatabase");
	auto statement = SqliteHandler::SqlCom::GetStatement("CREATE TABLE IF NOT EXISTS TestTable1 (testColumn VARCHAR PRIMARY KEY);", result);
	REQUIRE(result == 0);
	result = SqliteHandler::SqlCom::Query(statement);

	REQUIRE(result == 0);
	SqliteHandler::SqlCom::CloseConnection();
}

TEST_CASE("Add Table Primary function") {

	int result = SqliteHandler::DbMod::CreateTableWithPrimaryKey(
		DB,
		"TestTable2",
		"col1",
		"VARCHAR"
	);

	REQUIRE(result == 0);
}

TEST_CASE("Add Text Column") {
	int result = SqliteHandler::DbMod::AddColumn(
		DB,
		"TestTable2",
		"col2",
		"VARCHAR"
	);

	REQUIRE(result == 0);
}

TEST_CASE("Add Int Column") {
	int result = SqliteHandler::DbMod::AddColumn(
		DB,
		"TestTable2",
		"col3",
		"INT"
	);

	REQUIRE(result == 0);
}

TEST_CASE("Add some Blob Columns") {
	int result = SqliteHandler::DbMod::AddColumn(
		DB,
		"TestTable2",
		"col4",
		"BLOB"
	);

	REQUIRE(result == 0);

	result = SqliteHandler::DbMod::AddColumn(
		DB,
		"TestTable2",
		"col5",
		"BLOB"
	);

	REQUIRE(result == 0);

	result = SqliteHandler::DbMod::AddColumn(
		DB,
		"TestTable2",
		"col6",
		"BLOB"
	);

	REQUIRE(result == 0);
}

TEST_CASE("Add Table with foreign reference") {
	int result = SqliteHandler::DbMod::CreateTableWithForeignKey(
		DB,
		"TestTable3",
		"col1",
		"VARCHAR",
		"col1",
		"VARCHAR"
	);

	REQUIRE(result == 0);
}

TEST_CASE("Add Rows into table") {
	int result = SqliteHandler::DataSetter::InsertRow(
		DB,
		"TestTable2",
		"col1",
		"row1"
	);
	REQUIRE(result == 0);

	result = SqliteHandler::DataSetter::InsertRow(
		DB,
		"TestTable2",
		"col1",
		"row2"
	);
	REQUIRE(result == 0);

	result = SqliteHandler::DataSetter::InsertRow(
		DB,
		"TestTable2",
		"col1",
		"row3"
	);
	REQUIRE(result == 0);

}

TEST_CASE("update text value in field") {
	int result = SqliteHandler::DataSetter::StoreData(
		DB,
		"TestTable2",
		"col2",
		str1,
		"col1",
		"row1"
	);

	REQUIRE(result == 0);
}

TEST_CASE("update Textvalue in Int Field, should return error") {
	int result = SqliteHandler::DataSetter::StoreData(
		DB,
		"TestTable2",
		"col3",
		str1,
		"col1",
		"row1"
	);
	REQUIRE(result == 1);
}

TEST_CASE("Update Textvalue in non existent column, should return error") {
	int result = SqliteHandler::DataSetter::StoreData(
		DB,
		"TestTable2",
		"col50",
		str1,
		"col1",
		"row1"
	);
	REQUIRE(result == 1);
}

TEST_CASE("Update Textvalue in non existent row, should return error") {
	int result = SqliteHandler::DataSetter::StoreData(
		DB,
		"TestTable2",
		"col50",
		str1,
		"col1",
		"row50"
	);
	REQUIRE(result == 1);
}

TEST_CASE("Update int value in field") {
	int result = 0;

	result = SqliteHandler::DataSetter::StoreData(
		DB,
		"TestTable2",
		"col3",
		40,
		"col1",
		"row1"
	);

	REQUIRE(result == 0);
}

TEST_CASE("Update vector with pairs value in field") {
	int result = 0;

	std::vector<std::pair<int, int>> testPairs {
		{ 1, 2 },
		{ 3, 4 },
		{ 5, 6 }
	};

	result = SqliteHandler::DataSetter::StoreData(
		DB,
		"TestTable2",
		"col4",
		testPairs,
		"col1",
		"row1"
	);
	REQUIRE(result == 0);
}

TEST_CASE("Update blob field with vector containing strings") {
	int result = 0;

	std::vector<std::string> testStrVec {
		"one",
			"two",
			"three"
	};

	result = SqliteHandler::DataSetter::StoreData(
		DB,
		"TestTable2",
		"col4",
		testStrVec,
		"col1",
		"row1"
	);
	REQUIRE(result == 0);
}

TEST_CASE("Update blob field with vector containing integers") {
	int result = 0;

	std::vector<int> testIntVec {
		1,
			2,
			3
	};

	result = SqliteHandler::DataSetter::StoreData(
		DB,
		"TestTable2",
		"col4",
		testIntVec,
		"col1",
		"row1"
	);
	REQUIRE(result == 0);
}

TEST_CASE("Remove existing Row, should return 0") {
	int result;

	result = SqliteHandler::DataSetter::RemoveRow(
		DB,
		"TestTable2",
		"col1",
		"row2"
	);

	REQUIRE(result == 0);
}

TEST_CASE("Remove inexistent Row, should return errorcode") {
	int result;

	result = SqliteHandler::DataSetter::RemoveRow(
		DB,
		"TestTable2",
		"col1",
		"row50"
	);

	REQUIRE(result == 1);
}

TEST_CASE("Remove existing Row by value of non existent column, should return errorcode") {
	int result;

	result = SqliteHandler::DataSetter::RemoveRow(
		DB,
		"TestTable2",
		"col50",
		"row2"
	);

	REQUIRE(result == 1);
}

// Load data from DB
TEST_CASE("Load String from table in database") {
	int result = 0;
	std::vector<std::string> data;

	result = DataGetter::LoadData(DB, t2, col2, data);

	REQUIRE(data[0] == str1);
}