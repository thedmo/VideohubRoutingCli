//#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <filesystem>

#include <TelnetClient.hpp>
#include <iostream>
#include <config.h>

TEST_CASE("TelnetClient constructor creates a valid TelnetClient object. Should pass", "[TelnetClient]") {
	int result;
	std::string response;





	// TODO put into a separate class
	std::string path = VHUB_MOCK_DIR;
	std::string mock_path = path + "/BmdVideoHub.exe";
	std::string command = "start " + mock_path;
	INFO(command);
	// Start mockup server and wait a second for it to start
	std::system(command.c_str());
	std::this_thread::sleep_for(std::chrono::seconds(1));




	TelnetClient client("127.0.0.1", 9990, response, result);
	
	REQUIRE(result == 0);
}