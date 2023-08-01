#include <catch2/catch.hpp>
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

TEST_CASE("Sends Message to server and Checks, if expected answer gets returned, gets answer through GetLastDataDump() Method. should pass.", "[TelnetClient]") {
	int result;
	std::string response;

	VhubMockup::StartServer();
	TelnetClient client("127.0.0.1", 9990, response, result);

	response.clear();

	SECTION("Sending Message, should return 0") {
		result = client.SendMsgToServer("VIDEO OUTPUT ROUTING:\n\n");
		REQUIRE(result == 0);
	}
	SECTION("Checks receiving message, should return Acknowledge and routing data of a videohub device") {
		result = client.SendMsgToServer("VIDEO OUTPUT ROUTING:\n\n");
		REQUIRE(result == 0);

		response = client.GetLastDataDump();
		REQUIRE(response == "ACK\n\nVIDEO OUTPUT ROUTING:\n0 0\n1 1\n2 2\n3 3\n4 4\n5 5\n6 6\n7 7\n8 8\n9 9\n10 10\n11 11\n12 12\n13 13\n14 14\n15 15\n16 16\n17 17\n18 18\n19 19\n20 20\n21 21\n22 22\n23 23\n24 24\n25 25\n26 26\n27 27\n28 28\n29 29\n30 30\n31 31\n32 32\n33 33\n34 34\n35 35\n36 36\n37 37\n38 38\n39 39\n\n");
	}
}

TEST_CASE("Gets IP Address of connection") {
	int result;
	std::string response;

	VhubMockup::StartServer();
	TelnetClient client("127.0.0.1", 9990, response, result);

	std::string ip = client.GetIp();

	REQUIRE(ip == "127.0.0.1");
}