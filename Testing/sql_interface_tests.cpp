#include <catch2/catch.hpp>
#include <SqliteInterface.hpp>





// INITIALIZATION


/// <summary>
/// Drops router Table from database for a clean test
/// </summary>
void cleanUpInterfaceDatabase() {
	sql_access db("router");
	std::string query = "DROP TABLE routers";
	auto result = db.Query(db.GetStatement(query));

	if (result != 0) {
		std::cout << "Could not drop database: " + db.GetLastErrorMsg() << std::endl;
	}
}



// TESTS



TEST_CASE("GetDevices returns 0, if not, database or Devices table missing or general sql error", "[GetDevices]") {
	vdb db;
	std::string device_str;
	REQUIRE(db.GetDevices(device_str) == 0);
}