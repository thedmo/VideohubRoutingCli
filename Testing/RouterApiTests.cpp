#include <catch2/catch.hpp>
#include <RouterApi.hpp>
#include <SqliteInterface.hpp>
#include <DataHandler.hpp>
#include <MockupServer.h>

using namespace RouterModel;

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

int AddMockDeviceToStorage(std::string ip) {
	std::unique_ptr<device_data> device;
	DataHandler::GetDataOfSelectedDevice(device);
	device->ip = ip;


	// OLD
	vdb oldStorage;
	oldStorage.insert_device_into_db(device);
	// old


	// NEW
	DataHandler::AddDevice(device);
	DataHandler::SelectDevice(device);
	DataHandler::UpdateSelectedDeviceData(device);

	return 0;
}

int AddDevicesToStorage() {
	int result = 0;

	Vapi::AddRouter("127.0.0.1");

	AddMockDeviceToStorage("127.0.0.2");
	AddMockDeviceToStorage("127.0.0.3");
	AddMockDeviceToStorage("127.0.0.4");
	AddMockDeviceToStorage("127.0.0.5");
	AddMockDeviceToStorage("127.0.0.6");

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

	result = Vapi::SelectRouter("127.0.0.3");

	auto device = std::make_unique<device_data>();
	DataHandler::GetDataOfSelectedDevice(device);



	REQUIRE(device->ip == "127.0.0.3");
}

TEST_CASE("Get list of entries from storage") {
	InitializeStorage();
	AddDevicesToStorage();

	std::string compareString =
		"| 127.0.0.1            | XP 40x40             | 2.5                  |\n" \
		"| 127.0.0.2            | XP 40x40             | 2.5                  |\n" \
		"| 127.0.0.3            | XP 40x40             | 2.5                  |\n" \
		"| 127.0.0.4            | XP 40x40             | 2.5                  |\n" \
		"| 127.0.0.5            | XP 40x40             | 2.5                  |\n" \
		"| 127.0.0.6            | XP 40x40             | 2.5                  |\n";
	bool isSame;


	std::string devicesString;
	Vapi::GetDevices(devicesString);
	isSame = devicesString == compareString;
	REQUIRE(isSame);

	std::vector<std::string> stringList;
	Vapi::GetDevicesList(stringList);
	devicesString.clear();

	for (auto device : stringList) {
		device += '\n';
		devicesString += device;
	}
	isSame = devicesString == compareString;
	REQUIRE(isSame);
}

TEST_CASE("Remove selected Router from routers list") {
	int result = 0;
	std::vector<std::string> devicesList;
	std::string devicesListString;

	std::string compareString =
		"| 127.0.0.1            | XP 40x40             | 2.5                  |\n" \
		"| 127.0.0.2            | XP 40x40             | 2.5                  |\n" \
		"| 127.0.0.4            | XP 40x40             | 2.5                  |\n" \
		"| 127.0.0.5            | XP 40x40             | 2.5                  |\n" \
		"| 127.0.0.6            | XP 40x40             | 2.5                  |\n";

	InitializeStorage();
	AddDevicesToStorage();

	result = Vapi::SelectRouter("127.0.0.3");
	REQUIRE(result == 0);

	result = Vapi::RemoveSelectedRouter();
	REQUIRE(result == 0);

	result = Vapi::GetDevicesList(devicesList);
	for (auto device : devicesList) {
		device += '\n';
		devicesListString += device;
	}

	bool isSame = devicesListString == compareString;

	REQUIRE(isSame);
}

TEST_CASE("Rename sourcechannel of connected device") {
	int result = 0;
	std::unique_ptr<device_data> device;
	std::string compareString = "ChannelTest";

	InitializeStorage();

	result = Vapi::AddRouter("127.0.0.1");
	REQUIRE(result == 0);

	result = Vapi::RenameSource(0, "ChannelTest");
	REQUIRE(result == 0);

	result = DataHandler::GetDataOfSelectedDevice(device);
	REQUIRE(result == 0);

	bool isSame = device->sourceLabelsList[0] == compareString;

	REQUIRE(isSame);
}