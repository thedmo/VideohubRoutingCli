#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <filesystem>

#include <SqliteInterface.hpp>
#include <sql_access.hpp>

TEST_CASE("SqliteInterface constructor creates database file", "[sql_access]") {

  std::shared_ptr<sql_access> db = std::make_shared<sql_access>("test");

  REQUIRE(std::filesystem::exists("test.db"));
}


TEST_CASE("GetDevices returns 0, if not, database or Devices table missing or general sql error", "[GetDevices]") {
  vdb db;
  std::string device_str;
  REQUIRE(db.GetDevices(device_str) == 0);
}

