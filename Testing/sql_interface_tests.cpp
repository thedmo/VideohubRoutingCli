#include <catch2/catch.hpp>
#include <SqliteInterface.hpp>





// INITIALIZATION





// TESTS



TEST_CASE("GetDevices returns 0, if not, database or Devices table missing or general sql error", "[GetDevices]") {
	vdb db;
	std::string device_str;
	REQUIRE(db.GetDevices(device_str) == 0);
}