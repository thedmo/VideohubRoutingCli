#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

// #include "../includes/lib1/functions.hpp"
#include <SqliteInterface.hpp>

// Write UNIT Test for GetDevices function result should be 1. 

TEST_CASE("GetDevices returns 1", "[GetDevices]") {
  vdb db;
  std::string device_str;
  REQUIRE(db.GetDevices(device_str) == 1);
}
