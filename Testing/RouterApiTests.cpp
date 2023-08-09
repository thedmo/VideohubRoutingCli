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

	Vapi::SelectRouter("127.0.0.1");

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

TEST_CASE("Rename destinationchannel of connected device") {
	int result = 0;

	std::unique_ptr<device_data> device;
	std::string compareString = "ChannelTest";

	InitializeStorage();

	result = Vapi::AddRouter("127.0.0.1");
	REQUIRE(result == 0);

	result = Vapi::RenameDestination(0, "ChannelTest");
	REQUIRE(result == 0);

	result = DataHandler::GetDataOfSelectedDevice(device);
	REQUIRE(result == 0);

	bool isSame = device->destinationsLabelsList[0] == compareString;

	REQUIRE(isSame);
}

TEST_CASE("Prepare new routes for connected device") {
	int result = 0;

	InitializeStorage();
	AddDevicesToStorage();
	Vapi::SelectRouter("127.0.0.1");

	result = Vapi::PrepareNewRoute(0, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(1, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(2, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(3, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(4, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(5, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(6, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(7, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(8, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(9, 9);
	REQUIRE(result == 0);

	auto device = std::make_unique<device_data>();
	result = DataHandler::GetDataOfSelectedDevice(device);
	REQUIRE(result == 0);

	auto routing = device->routesPreparedList;

	auto route = routing[0];
	REQUIRE(route.second == 9);

	route = routing[1];
	REQUIRE(route.second == 9);

	route = routing[2];
	REQUIRE(route.second == 9);

	route = routing[3];
	REQUIRE(route.second == 9);

	route = routing[4];
	REQUIRE(route.second == 9);

	route = routing[5];
	REQUIRE(route.second == 9);

	route = routing[6];
	REQUIRE(route.second == 9);

	route = routing[7];
	REQUIRE(route.second == 9);

	route = routing[8];
	REQUIRE(route.second == 9);

	route = routing[9];
	REQUIRE(route.second == 9);
}

TEST_CASE("take prepared routes on connected device") {
	int result = 0;

	InitializeStorage();
	AddDevicesToStorage();
	Vapi::SelectRouter("127.0.0.1");

	// Preparation
	result = Vapi::PrepareNewRoute(0, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(1, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(2, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(3, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(4, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(5, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(6, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(7, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(8, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(9, 9);
	REQUIRE(result == 0);


	result = Vapi::TakePreparedRoutes();
	REQUIRE(result == 0);


	// Take new routes
	auto device = std::make_unique<device_data>();
	result = DataHandler::GetDataOfSelectedDevice(device);
	REQUIRE(result == 0);

	auto routing = device->routesList;

	auto route = routing[0];
	REQUIRE(route.second == 9);

	route = routing[1];
	REQUIRE(route.second == 9);

	route = routing[2];
	REQUIRE(route.second == 9);

	route = routing[3];
	REQUIRE(route.second == 9);

	route = routing[4];
	REQUIRE(route.second == 9);

	route = routing[5];
	REQUIRE(route.second == 9);

	route = routing[6];
	REQUIRE(route.second == 9);

	route = routing[7];
	REQUIRE(route.second == 9);

	route = routing[8];
	REQUIRE(route.second == 9);

	route = routing[9];
	REQUIRE(route.second == 9);


	// Reset to initial values
	result = Vapi::PrepareNewRoute(0, 0);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(1, 1);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(2, 2);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(3, 3);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(4, 4);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(5, 5);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(6, 6);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(7, 7);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(8, 8);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(9, 9);
	REQUIRE(result == 0);

	result = Vapi::TakePreparedRoutes();
	REQUIRE(result == 0);
}

TEST_CASE("Lock routes of connected device and the unlock again") {
	int result = 0;

	InitializeStorage();
	AddDevicesToStorage();
	std::unique_ptr<device_data> device;

	SECTION("Locking") {
		result = Vapi::LockRoute(0);
		REQUIRE(result == 0);

		result = Vapi::LockRoute(10);
		REQUIRE(result == 0);

		result = Vapi::LockRoute(15);
		REQUIRE(result == 0);

		result = Vapi::LockRoute(40);
		REQUIRE(result == 1);

		result = DataHandler::GetDataOfSelectedDevice(device);
		REQUIRE(result == 0);

		REQUIRE(device->locksList[0] == "L");
		REQUIRE(device->locksList[10] == "L");
		REQUIRE(device->locksList[15] == "L");
	}

	SECTION("Unlocking") {
		result = Vapi::UnlockRoute(0);
		REQUIRE(result == 0);

		result = Vapi::UnlockRoute(10);
		REQUIRE(result == 0);

		result = Vapi::UnlockRoute(15);
		REQUIRE(result == 0);

		result = Vapi::UnlockRoute(40);
		REQUIRE(result == 1);

		result = DataHandler::GetDataOfSelectedDevice(device);
		REQUIRE(result == 0);

		REQUIRE(device->locksList[0] == "U");
		REQUIRE(device->locksList[10] == "U");
		REQUIRE(device->locksList[15] == "U");
	}
}

TEST_CASE("mark routes form connected device to save as routing in storage") {
	int result = 0;
	std::unique_ptr<device_data> device;

	InitializeStorage();
	AddDevicesToStorage();

	result = Vapi::MarkRoutes2({ 0,1,2,3,4 });
	REQUIRE(result == 0);

	result = DataHandler::GetDataOfSelectedDevice(device);
	REQUIRE(result == 0);

	REQUIRE(device->routesMarkedList[0] == std::pair{0,0});
	REQUIRE(device->routesMarkedList[1] == std::pair{1,1});
	REQUIRE(device->routesMarkedList[2] == std::pair{2,2});
	REQUIRE(device->routesMarkedList[3] == std::pair{3,3});
	REQUIRE(device->routesMarkedList[4] == std::pair{4,4});

	result = Vapi::MarkRoutes2({ 40, 41, 42 });
	REQUIRE(result == 1);
}

TEST_CASE("save marked routes as routing in storage") {
	int result = 0;

	InitializeStorage();
	AddDevicesToStorage();

	result = Vapi::MarkRoutes2({ 0,1,2,3,4 });
	REQUIRE(result == 0);

	result = Vapi::SaveRoutes("TestRoutes1");
	REQUIRE(result == 0);

	DataHandler::RoutingsList routings;
	result = DataHandler::GetRoutesFromSelected(routings);

	REQUIRE(routings[0].first == "TestRoutes1");
	REQUIRE(routings[0].second[0] == std::pair{0,0});
	REQUIRE(routings[0].second[1] == std::pair{1,1});
	REQUIRE(routings[0].second[2] == std::pair{2,2});
}

TEST_CASE("get listing of saved routings from storage") {
	int result = 0;

	InitializeStorage();
	AddDevicesToStorage();

	result = Vapi::MarkRoutes2({ 0,1,2,3,4 });
	REQUIRE(result == 0);

	result = Vapi::SaveRoutes("TestRoutes1");
	REQUIRE(result == 0);

	std::vector< std::pair<std::string, std::vector<std::pair<int, int>>>> routingsList;
	result = Vapi::GetSavedRoutes(routingsList);
	REQUIRE(result == 0);

	REQUIRE(routingsList[0].first == "TestRoutes1");
	REQUIRE(routingsList[0].second[0] == std::pair{0, 0});
	REQUIRE(routingsList[0].second[1] == std::pair{1, 1});
	REQUIRE(routingsList[0].second[2] == std::pair{2, 2});
}

TEST_CASE("load saved routing from storage and send it to connected device") {
	int result = 0;
	std::unique_ptr<device_data> connectedDevice;
	std::string ip = "127.0.0.1";

	InitializeStorage();
	AddDevicesToStorage();

	result = Vapi::PrepareNewRoute(0, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(1, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(2, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(3, 9);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(4, 9);
	REQUIRE(result == 0);

	result = Vapi::TakePreparedRoutes();
	REQUIRE(result == 0);

	result = Vapi::MarkRoutes2({ 0,1,2,3,4 });
	REQUIRE(result == 0);

	result = Vapi::SaveRoutes("TestRoutes1");
	REQUIRE(result == 0);
	
	result = Vapi::LoadRoutes("TestRoutes1");
	REQUIRE(result == 0);

	result = Vapi::GetStatus(ip, connectedDevice);
	REQUIRE(result == 0);

	REQUIRE(connectedDevice->routesList[0] == std::pair{0,9});
	REQUIRE(connectedDevice->routesList[1] == std::pair{1,9});
	REQUIRE(connectedDevice->routesList[2] == std::pair{2,9});
	REQUIRE(connectedDevice->routesList[3] == std::pair{3,9});
	REQUIRE(connectedDevice->routesList[4] == std::pair{4,9});

	result = Vapi::PrepareNewRoute(0, 0);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(1, 1);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(2, 2);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(3, 3);
	REQUIRE(result == 0);

	result = Vapi::PrepareNewRoute(4, 4);
	REQUIRE(result == 0);

	result = Vapi::TakePreparedRoutes();
	REQUIRE(result == 0);
}