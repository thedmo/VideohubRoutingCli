#include <catch2/catch.hpp>
#include <sql_access_handler.hpp>
#include <filesystem>
#include <iostream>

const std::string DBNAME = "SqlHandlerDataBase";
const std::string TABLE1 = "Table1";
const std::string TABLE2 = "Table2";
const std::string DB_FILENAME = (DBNAME + ".db");

class TestValues {
public:
	static inline std::string row1 = "first";
	static inline std::string row2 = "second";
	static inline std::string row3 = "third";
	static inline int count1 = 5;
	static inline int count2 = 7;
	static inline int count3 = 9;
	static inline std::vector<int> intDataSet1{0, 1, 2};
	static inline std::vector<int> intDataSet2{3, 4, 5};
	static inline std::vector<int> intDataSet3{6, 7, 8};
	static inline std::vector<std::string> stringDataSet1 {"one", "two", "three"};
	static inline std::vector<std::string> stringDataSet2 {"four", "five", "six"};
	static inline std::vector<std::string> stringDataSet3 {"seven", "eight", "nine"};
	static inline std::vector<std::tuple<int, int>> tupleDataSet1 {{0, 1}, { 2, 3 }, { 4, 5 }};
	static inline std::vector<std::tuple<int, int>> tupleDataSet2 {{6, 7}, { 8, 9 }, { 10, 11 }};
	static inline std::vector<std::tuple<int, int>> tupleDataSet3 {{12, 13}, { 14, 15 }, { 16, 17 }};

	static inline std::string
		address = "deviceAddress",
		count = "channelCount",
		routes = "routes",
		names = "channelNames",
		marked = "markedRoutes";
};

void InitializeFile() {
	if (std::filesystem::exists(DB_FILENAME)) {
		return;
	}

	sql_access db(DBNAME);
}

void Clean() {
	if (!std::filesystem::exists(DB_FILENAME))
	{
		return;
	}

	try
	{
		std::error_code ec;
		std::filesystem::remove(DB_FILENAME, ec);
		if (ec)
		{
			std::cout << ec.message() << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}

using namespace SqliteHandler;

// TESTS

TEST_CASE("Create Tables", "[SqlAccessHandler]") {
	int result;

	result = DatabaseChanger::CreateTableWithPrimaryKey(DBNAME, TABLE1, TestValues::address, DatabaseChanger::V_CHAR);
	result = DatabaseChanger::CreateTableWithForeignKey(DBNAME, TABLE2, TestValues::address, DatabaseChanger::V_CHAR, TABLE1, TestValues::address);

	REQUIRE(result == 0);
}


TEST_CASE("Adding columns") {
	int result;

		result = DatabaseChanger::AddColumn(DBNAME, TABLE1, TestValues::count, DatabaseChanger::INT);
		REQUIRE(result == 0);

		result = DatabaseChanger::AddColumn(DBNAME, TABLE1, TestValues::routes, DatabaseChanger::BLOB);
		REQUIRE(result == 0);

		result = DatabaseChanger::AddColumn(DBNAME, TABLE1, TestValues::names, DatabaseChanger::BLOB);
		REQUIRE(result == 0);

		result = DatabaseChanger::AddColumn(DBNAME, TABLE1, TestValues::marked, DatabaseChanger::BLOB);
		REQUIRE(result == 0);
}

TEST_CASE("INSERTING rows") {
	int result;
		result = SqliteDataSetter::insertRow(DBNAME, TABLE1, TestValues::address, TestValues::row1);
		REQUIRE(result == 0);

		result = SqliteDataSetter::insertRow(DBNAME, TABLE1, TestValues::address, TestValues::row2);
		REQUIRE(result == 0);

		result = SqliteDataSetter::insertRow(DBNAME, TABLE1, TestValues::address, TestValues::row3);
		REQUIRE(result == 0);
}

TEST_CASE("Storing data in fields") {
	int result;
		result = SqliteDataSetter::storeData(DBNAME, TABLE1, TestValues::count, TestValues::count1, TestValues::address, TestValues::row1);
		REQUIRE(result == 0);

		result = SqliteDataSetter::storeData(DBNAME, TABLE1, TestValues::routes, TestValues::intDataSet1, TestValues::address, TestValues::row1);
		REQUIRE(result == 0);

		result = SqliteDataSetter::storeData(DBNAME, TABLE1, TestValues::names, TestValues::stringDataSet1, TestValues::address, TestValues::row1);
		REQUIRE(result == 0);

		result = SqliteDataSetter::storeData(DBNAME, TABLE1, TestValues::marked, TestValues::tupleDataSet1, TestValues::address, TestValues::row1);
		REQUIRE(result == 0);
}

TEST_CASE("Loading Data from database") {
	int result;

	SECTION("LOADING int from row 1") {
		std::vector<int> compareInt;
		result = SqliteDataGetter::LoadData(DBNAME, TABLE1, TestValues::count, TestValues::address, TestValues::row1, compareInt);
		REQUIRE(compareInt[0] == TestValues::count1);
	}

	SECTION("LOADING string from row 1") {
		std::vector<std::string> compareString;
		result = SqliteDataGetter::LoadData(DBNAME, TABLE1, TestValues::address, TestValues::address, TestValues::row1, compareString);
		REQUIRE(compareString[0] == TestValues::row1);
	}

	SECTION("Loading int Vector from Row 1") {
		std::vector<std::vector<int>> compareVecInt { TestValues::intDataSet3 };
		compareVecInt.clear();
		result = SqliteDataGetter::LoadData(DBNAME, TABLE1, TestValues::routes, TestValues::address, TestValues::row1, compareVecInt);
		REQUIRE(compareVecInt[0] == TestValues::intDataSet1);
	}

	SECTION("Loading string vector from row 1") {
		std::vector<std::vector<std::string>> compareVecString { TestValues::stringDataSet3 };
		compareVecString.clear();
		result = SqliteDataGetter::LoadData(DBNAME, TABLE1, TestValues::names, TestValues::address, TestValues::row1, compareVecString);
		REQUIRE(compareVecString[0] == TestValues::stringDataSet1);
	}

	SECTION("Loading tuple Vector from row 1") {
		std::vector<std::vector<std::tuple<int, int>>> compareVecTuple  { TestValues::tupleDataSet3 };
		compareVecTuple.clear();
		result = SqliteDataGetter::LoadData(DBNAME, TABLE1, TestValues::marked, TestValues::address, TestValues::row1, compareVecTuple);
		REQUIRE(compareVecTuple[0] == TestValues::tupleDataSet1);
	}
}

TEST_CASE("Removing rows from table"){
	int result;

	SECTION("REMOVING rows") {
		result = SqliteDataSetter::removeRow(DBNAME, TABLE1, TestValues::address, TestValues::row1);
		REQUIRE(result == 0);		

		result = SqliteDataSetter::removeRow(DBNAME, TABLE1, TestValues::address, TestValues::row2);
		REQUIRE(result == 0);		

		result = SqliteDataSetter::removeRow(DBNAME, TABLE1, TestValues::address, TestValues::row3);
		REQUIRE(result == 0);
	}
}