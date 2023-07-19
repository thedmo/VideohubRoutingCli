#include <TestHeader.h>
#include <catch2/catch.hpp>
#include <sql_access_handler.hpp>

const std::string DB = "SqlHandlerTestDatabase";

TEST_CASE("Connect to Database") {
	int result = SqliteHandler::SqlCom::ConnectToDatabase("SqlHandlerTestDatabase");
	REQUIRE(result == 0);
	SqliteHandler::SqlCom::CloseConnection();
}

TEST_CASE("prepare statement") {
	int result = SqliteHandler::SqlCom::ConnectToDatabase("SqlHandlerTestDatabase");
	auto statement = SqliteHandler::SqlCom::GetStatement("CREATE TABLE IF NOT EXISTS TestTable1 (testColumn VARCHAR PRIMARY KEY);", result);

	REQUIRE(result == 0);
	SqliteHandler::SqlCom::CloseConnection();
}

TEST_CASE("Test Query function") {
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

TEST_CASE("Add Text Column function") {
	int result = SqliteHandler::DbMod::AddColumn(
		DB,
		"TestTable2",
		"col2",
		"VARCHAR"
	);

	REQUIRE(result == 0);
}

TEST_CASE("Add Int Column function") {
	int result = SqliteHandler::DbMod::AddColumn(
		DB,
		"TestTable2",
		"col3",
		"INT"
	);

	REQUIRE(result == 0);
}

TEST_CASE("Add Blob Column function") {
	int result = SqliteHandler::DbMod::AddColumn(
		DB,
		"TestTable2",
		"col4",
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
		"text1",
		"col1",
		"row1"
	);

	REQUIRE(result == 0);
}


// Funktioniert nicht, wird eingefügt, auch wenn Typ nicht stimmt -> Sqlite funktionalität
//TEST_CASE("update Textvalue in Int Field, should return error") {
//	int result = SqliteHandler::DataSetter::StoreData(
//		DB,
//		"TestTable2",
//		"col3",
//		"text1",
//		"col1",
//		"row1"
//	);
//	REQUIRE(result == 1);
//}

TEST_CASE("Update Textvalue in non existent column, should return error") {
	int result = SqliteHandler::DataSetter::StoreData(
		DB,
		"TestTable2",
		"col5",
		"text1",
		"col1",
		"row1"
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

TEST_CASE("Remove existing Row, should return 0") {
	int result;

	result = SqliteHandler::DataSetter::RemoveRow(
		DB,
		"TestTable2",
		"col1",
		"row1"
	);

	REQUIRE(result == 0);
}


// WEITER Check, ob andere Einträge noch vorhanden sind, returniert an sich 0, auch wenn der Eintrag nicht vorhanden ist
//TEST_CASE("Remove inexistent Row, should return 1") {
//	int result;
//
//	result = SqliteHandler::DataSetter::RemoveRow(
//		DB,
//		"TestTable2",
//		"col1",
//		"row50"
//	);
//
//	REQUIRE(result == 1);
//}

TEST_CASE("Remove existing Row by value of non existent column, should return 1") {
	int result;

	result = SqliteHandler::DataSetter::RemoveRow(
		DB,
		"TestTable2",
		"col50",
		"row2"
	);

	REQUIRE(result == 1);
}