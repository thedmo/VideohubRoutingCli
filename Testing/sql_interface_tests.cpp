#include <catch2/catch.hpp>
#include <SqliteInterface.hpp>
#include <filesystem>

#include <MockupDeviceData.h>



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


TEST_CASE("Insert a new device into database") {

	cleanUpInterfaceDatabase();

	auto mockupDevice = mockup::GetMockupDevice();

	vdb database;

	int result = database.insert_device_into_db(mockupDevice);

	if (result != 0) {
		std::cout << "Test failing: ";
		for (std::string msg : database.GetErrorMessages()) {
			std::cout << msg << std::endl;

		}
	}

	REQUIRE(result == 0);
}

TEST_CASE("GetDevices returns 0, if not, database or Devices table missing or general sql error", "[GetDevices]") {
	vdb db;
	std::string device_str;
	REQUIRE(db.GetDevices(device_str) == 0);
}