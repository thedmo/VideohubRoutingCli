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

const std::string colid = "colid";
const std::string colStr = "colStr";
const std::string colInt = "colInt";
const std::string colVecPair = "colVecPair";
const std::string colVecString = "colVecString";
const std::string colVecInt = "colVecInt";

const std::string row1 = "row1";
const std::string row2 = "row2";
const std::string row3 = "row3";

const std::string str1 = "one";
const std::string str2 = "two";
const std::string str3 = "three";

const int int1 = 1;
const int int2 = 2;
const int int3 = 3;
const int int4 = 4;
const int int5 = 5;
const int int6 = 6;

const std::vector <std::string> strVec1 = { str1, str2, str3 };
const std::vector <int> intVec1 = { int1,int2,int3 };
const std::vector <std::pair<int, int>> pairVec1 = { { int1, int2 },{ int3, int4 },{ int5, int6 } };

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
		t2,
		colid,
		TEXT
	);

	REQUIRE(result == 0);
}

TEST_CASE("Add Text Column") {
	int result = SqliteHandler::DbMod::AddColumn(
		DB,
		t2,
		colStr,
		TEXT
	);

	REQUIRE(result == 0);
}

TEST_CASE("Add Int Column") {
	int result = SqliteHandler::DbMod::AddColumn(
		DB,
		t2,
		colInt,
		INTEGER
	);

	REQUIRE(result == 0);
}

TEST_CASE("Add some Blob Columns") {
	int result = SqliteHandler::DbMod::AddColumn(
		DB,
		t2,
		colVecPair,
		BLOB
	);

	REQUIRE(result == 0);

	result = SqliteHandler::DbMod::AddColumn(
		DB,
		t2,
		colVecString,
		BLOB
	);

	REQUIRE(result == 0);

	result = SqliteHandler::DbMod::AddColumn(
		DB,
		t2,
		colVecInt,
		BLOB
	);

	REQUIRE(result == 0);
}

TEST_CASE("Add Table with foreign reference") {
	int result = SqliteHandler::DbMod::CreateTableWithForeignKey(
		DB,
		t3,
		colid,
		TEXT,
		colid,
		TEXT
	);

	REQUIRE(result == 0);
}

TEST_CASE("Add Rows into table") {
	int result = SqliteHandler::DbMod::InsertRow(
		DB,
		t2,
		colid,
		row1
	);
	REQUIRE(result == 0);

	result = SqliteHandler::DbMod::InsertRow(
		DB,
		t2,
		colid,
		row2
	);
	REQUIRE(result == 0);

	result = SqliteHandler::DbMod::InsertRow(
		DB,
		t2,
		colid,
		row3
	);
	REQUIRE(result == 0);

}

TEST_CASE("update text value in field") {
	int result = SqliteHandler::DataSetter::StoreData(
		DB,
		t2,
		colStr,
		str1,
		colid,
		row1
	);

	REQUIRE(result == 0);
}

TEST_CASE("update Textvalue in Int Field, should return error") {
	int result = SqliteHandler::DataSetter::StoreData(
		DB,
		t2,
		colInt,
		str1,
		colid,
		row1
	);
	REQUIRE(result == 1);
}

TEST_CASE("Update Textvalue in non existent column, should return error") {
	int result = SqliteHandler::DataSetter::StoreData(
		DB,
		t2,
		"col50",
		str1,
		colid,
		row1
	);
	REQUIRE(result == 1);
}

TEST_CASE("Update Textvalue in non existent row, should return error") {
	int result = SqliteHandler::DataSetter::StoreData(
		DB,
		t2,
		"col50",
		str1,
		colid,
		"row50"
	);
	REQUIRE(result == 1);
}

TEST_CASE("Update int value in field") {
	int result = 0;

	result = SqliteHandler::DataSetter::StoreData(
		DB,
		t2,
		colInt,
		int2,
		colid,
		row1
	);

	REQUIRE(result == 0);
}

TEST_CASE("Update vector with pairs value in field") {
	int result = 0;


	result = SqliteHandler::DataSetter::StoreData(
		DB,
		t2,
		colVecPair,
		pairVec1,
		colid,
		row1
	);
	REQUIRE(result == 0);
}

TEST_CASE("Update blob field with vector containing strings") {
	int result = 0;

	result = SqliteHandler::DataSetter::StoreData(
		DB,
		t2,
		colVecString,
		strVec1,
		colid,
		row1
	);
	REQUIRE(result == 0);
}

TEST_CASE("Update blob field with vector containing integers") {
	int result = 0;

	result = SqliteHandler::DataSetter::StoreData(
		DB,
		t2,
		colVecInt,
		intVec1,
		colid,
		row1
	);
	REQUIRE(result == 0);
}

TEST_CASE("Remove existing Row, should return 0") {
	int result;

	result = SqliteHandler::DbMod::RemoveRow(
		DB,
		t2,
		colid,
		row2
	);

	REQUIRE(result == 0);
}

TEST_CASE("Remove inexistent Row, should return errorcode") {
	int result;

	result = SqliteHandler::DbMod::RemoveRow(
		DB,
		t2,
		colid,
		"row50"
	);

	REQUIRE(result == 1);
}

TEST_CASE("Remove existing Row by value of non existent column, should return errorcode") {
	int result;

	result = SqliteHandler::DbMod::RemoveRow(
		DB,
		t2,
		"col50",
		row2
	);

	REQUIRE(result == 1);
}

TEST_CASE("Load String from table in database") {
	int result = 0;
	std::vector<std::string> data;

	result = DataGetter::LoadData(DB, t2, colStr, data);

	REQUIRE(data[0] == str1);
}

TEST_CASE("Load Int from table in database") {
	int result = 0;
	std::vector<int> data;

	result = DataGetter::LoadData(DB, t2, colInt, data);

	REQUIRE(data[0] == int2);
}

TEST_CASE("Load vector with integers from table in database") {
	int result = 0;
	std::vector < std::vector<int> > data;

	result = DataGetter::LoadData(DB, t2, colVecInt, data);

	REQUIRE(data[0] == intVec1);
}

TEST_CASE("Load vector with string from table in database") {
	int result = 0;
	std::vector < std::vector<std::string> > data;

	result = DataGetter::LoadData(DB, t2, colVecString, data);

	REQUIRE(data[0] == strVec1);
}

TEST_CASE("Load vector with pairs from table in database") {
	int result = 0;
	std::vector < std::vector<std::pair<int,int>> > data;

	result = DataGetter::LoadData(DB, t2, colVecPair, data);

	REQUIRE(data[0] == pairVec1);
}

TEST_CASE("Load string from column with int. should return errorcode") {
	int result = 0;
	std::vector<int> data;

	result = DataGetter::LoadData(DB, t2, colStr, data);

	REQUIRE(result == 1);
}