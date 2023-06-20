//#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
//#include <filesystem>
//#include <config.h>

#include <TelnetClient.hpp>
#include <iostream>

#include "MockupServer.h"

TEST_CASE("TelnetClient constructor creates a valid TelnetClient object. Should pass", "[TelnetClient]") {
	int result;
	std::string response;

	VhubMockup::StartServer();
	TelnetClient client("127.0.0.1", 9990, response, result);
	
	REQUIRE(result == 0);
}
