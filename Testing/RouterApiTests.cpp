#include <catch2/catch.hpp>
#include <RouterApi.hpp>
#include <SqliteInterface.hpp>
#include <DataHandler.hpp>
#include <MockupServer.h>

int InitializeStorage() {
	int result = 0;

	result = DataHandler::ClearStorageFile("RouterDb");
	if (result) return 1;

	result = DataHandler::ClearStorageFile("router");
	if (result) return 1;

	result = DataHandler::InitializeStorage();
	if (result) return 1;

	vdb dataBase;
	VhubMockup::StartServer();

	return 0;
}

int AddDevicesToStorage() {
	int result = 0;

	Vapi::AddRouter("127.0.0.1");

	std::unique_ptr<device_data> device;
	DataHandler::GetDataOfSelectedDevice(device);

	device->ip = "127.0.0.2";
	DataHandler::AddDevice(device);
	DataHandler::SelectDevice(device);
	DataHandler::UpdateSelectedDeviceData(device);

	std::unique_ptr<device_data> device2;
	DataHandler::GetDataOfSelectedDevice(device2);

	device2->ip = "127.0.0.3";
	DataHandler::AddDevice(device2);
	DataHandler::SelectDevice(device2);
	DataHandler::UpdateSelectedDeviceData(device2);

	return 0;
}

TEST_CASE("Cenvert line with two numbers on it into a std pair of two ints") {
	int result = 0;
	std::string routeLine = "2 3\n";
	std::pair<int, int> routePair;

	result = Converter::RouteLineToRoutePair(routeLine, routePair);

	REQUIRE(result == 0);
	REQUIRE(routePair.first == 2);
	REQUIRE(routePair.second == 3);
}

TEST_CASE("Extract infromation from line with number of channel and string after it") {
	int result = 0;
	std::string stringLine = "2 Channel One\n";
	std::string information;

	result = Converter::StringLineToData(stringLine, information);

	REQUIRE(result == 0);
	REQUIRE(information == "Channel One");
}

TEST_CASE("Get Status of device by ip address") {
	int result = 0;
	std::string ip = "127.0.0.1";
	std::unique_ptr<device_data> deviceData;

	result = Vapi::GetStatus(ip, deviceData);

	REQUIRE(deviceData->name == "XP 40x40");
}

TEST_CASE("Add new router to storage") {
	int result = 0;

	result = InitializeStorage();

	vdb dataBase;

	std::string ip = "127.0.0.1";
	result = Vapi::AddRouter(ip);
	REQUIRE(result == 0);
}

TEST_CASE("Select a Router from the devices list") {
	InitializeStorage();
	AddDevicesToStorage();

	int result = 0;



	REQUIRE(result == 0);
}