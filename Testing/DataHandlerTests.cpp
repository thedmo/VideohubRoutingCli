#include <Catch2/catch.hpp>
#include <DataHandler.hpp>

class DHTestValues {
public:
	static inline std::string
		ip1 = "127.0.0.1",
		ip2 = "127.0.0.2",
		ip3 = "127.0.0.3",

		name1 = "Smart1",
		name2 = "Smart2",
		name3 = "Smart3",

		version1 = "v1.1",
		version2 = "v1.2",
		version3 = "v1.3",

		routesName1 = "routing1",
		routesName2 = "routing2",
		routesName3 = "routing3";

	static inline int
		sourceCount1 = 20,
		sourceCount2 = 40,
		sourceCount3 = 60,

		destinationCount1 = 20,
		destinationCount2 = 40,
		destinationCount3 = 60;

	static inline std::vector<std::string>
		sourceLabelsList1 = { "one, two, three" },
		sourceLabelsList2 = { "four, five, six" },
		sourceLabelsList3 = { "seven, eight, nine" },

		destinationsLabelsList1 = { "one, two, three" },
		destinationsLabelsList2 = { "four, five, six" },
		destinationsLabelsList3 = { "seven, eight, nine" },

		locksList1 = { "lone", "ltwo", "lthree" },
		locksList2 = { "lone", "ltwo", "lthree" },
		locksList3 = { "lone", "ltwo", "lthree" };

	static inline std::vector<std::pair<int, int>>
		routesMarkedList1 = { std::pair{1,2}, std::pair{3,4}, std::pair{5,6      } },
		routesMarkedList2 = { std::pair{7,8}, std::pair{9,10}, std::pair{11,12   } },
		routesMarkedList3 = { std::pair{13,14}, std::pair{15,16}, std::pair{17,18} },

		routesPreparedList1 = { std::pair{1,2}, std::pair{3,4}, std::pair{5,6      } },
		routesPreparedList2 = { std::pair{7,8}, std::pair{9,10}, std::pair{11,12   } },
		routesPreparedList3 = { std::pair{13,14}, std::pair{15,16}, std::pair{17,18} },

		routesList1 = { std::pair{1,2}, std::pair{3,4}, std::pair{5,6      } },
		routesList2 = { std::pair{7,8}, std::pair{9,10}, std::pair{11,12   } },
		routesList3 = { std::pair{13,14}, std::pair{15,16}, std::pair{17,18} };


	static inline std::unique_ptr<device_data> device1 = std::make_unique<device_data>();
	static inline std::unique_ptr<device_data> device2 = std::make_unique<device_data>();
	static inline std::unique_ptr<device_data> device3 = std::make_unique<device_data>();

	static inline DataHandler::Routing routing1 = { routesName1, routesList1 };
	static inline DataHandler::Routing routing2 = { routesName2, routesList2 };
	static inline DataHandler::Routing routing3 = { routesName3, routesList3 };

	static inline DataHandler::RoutingsList routings1 = { routing1, routing2, routing3 };

	static void InitializeDevices() {
		device1->ip = ip1;
		device1->name = name1;
		device1->version = version1;
		device1->source_count = sourceCount1;
		device1->destination_count = destinationCount1;
		device1->sourceLabelsList = sourceLabelsList1;
		device1->destinationsLabelsList = destinationsLabelsList1;
		device1->locksList = locksList1;
		device1->routesMarkedList = routesMarkedList1;
		device1->routesPreparedList = routesPreparedList1;
		device1->routesList = routesList1;

		device2->ip = ip2;
		device2->name = name2;
		device2->version = version2;
		device2->source_count = sourceCount2;
		device2->destination_count = destinationCount2;
		device2->sourceLabelsList = sourceLabelsList2;
		device2->destinationsLabelsList = destinationsLabelsList2;
		device2->locksList = locksList2;
		device2->routesMarkedList = routesMarkedList2;
		device2->routesPreparedList = routesPreparedList2;
		device2->routesList = routesList2;

		device3->ip = ip3;
		device3->name = name3;
		device3->version = version3;
		device3->source_count = sourceCount3;
		device3->destination_count = destinationCount3;
		device3->sourceLabelsList = sourceLabelsList3;
		device3->destinationsLabelsList = destinationsLabelsList3;
		device3->locksList = locksList3;
		device3->routesMarkedList = routesMarkedList3;
		device3->routesPreparedList = routesPreparedList3;
		device3->routesList = routesList3;
	}
};

void Initialize() {
	DHTestValues::InitializeDevices();
	DataHandler::InitializeStorage();
}

void DropTablesDataHandler() {
	int result = 0;

	result = SqlCom::ConnectToDatabase(DataHandler::DB_NAME);

	std::string queryStr = "DROP TABLE IF EXISTS " + DataHandler::DEVICES_TABLE + ";";
	auto statement = SqlCom::GetStatement(queryStr, result);
	result = SqlCom::Query(statement);

	queryStr = "DROP TABLE IF EXISTS " + DataHandler::ROUTINGS_TABLE + ";";
	statement = SqlCom::GetStatement(queryStr, result);
	result = SqlCom::Query(statement);

	result = SqlCom::CloseConnection();
}

void RemoveDevicesFromTable(std::unique_ptr<device_data>& device) {
	DataHandler::RemoveDevice(device);
}

int AddSelectUpdateDevice(std::unique_ptr<device_data>& device) {
	int result = 0;

	result = DataHandler::AddDevice(device);
	if (result) return 1;

	result = DataHandler::SelectDevice(device);
	if (result) return 1;

	result = DataHandler::UpdateSelectedDeviceData(device);
	if (result) return 1;

	return 0;
}

int AddRoutingsToSelectedDevice() {
	int result = 0;

	result = DataHandler::SelectDevice(DHTestValues::device1);
	if (result) return 1;

	result = DataHandler::StoreRoutingForSelected(DHTestValues::routing1);
	if (result) return 1;

	result = DataHandler::StoreRoutingForSelected(DHTestValues::routing2);
	if (result) return 1;

	result = DataHandler::StoreRoutingForSelected(DHTestValues::routing3);
	if (result) return 1;

	return 0;
}

int InitializeAndFillRouterDb() {
	DropTablesDataHandler();
	Initialize();
	AddSelectUpdateDevice(DHTestValues::device1);
	AddSelectUpdateDevice(DHTestValues::device2);
	AddSelectUpdateDevice(DHTestValues::device3);

	return 0;
}

TEST_CASE("Add New Devices to database") {

	DropTablesDataHandler();
	Initialize();

	REQUIRE(DataHandler::AddDevice(DHTestValues::device1) == 0);
	REQUIRE(DataHandler::AddDevice(DHTestValues::device2) == 0);
	REQUIRE(DataHandler::AddDevice(DHTestValues::device3) == 0);
}

TEST_CASE("Add already existing device to storage") {
	InitializeAndFillRouterDb();

	REQUIRE(DataHandler::AddDevice(DHTestValues::device1) == 1);
}

TEST_CASE("Remove existing device") {
	InitializeAndFillRouterDb();

	REQUIRE(DataHandler::RemoveDevice(DHTestValues::device3) == 0);
}

TEST_CASE("Selects entry in storage") {
	int result = 0;

	result = DataHandler::SelectDevice(DHTestValues::device1);

	REQUIRE(result == 0);
}

TEST_CASE("Update data of entry in storage") {
	int result = 0;

	DropTablesDataHandler();
	Initialize();
	DataHandler::AddDevice(DHTestValues::device1);
	DataHandler::AddDevice(DHTestValues::device2);
	DataHandler::AddDevice(DHTestValues::device3);

	DataHandler::SelectDevice(DHTestValues::device1);
	result = DataHandler::UpdateSelectedDeviceData(DHTestValues::device1);
	REQUIRE(result == 0);

	DataHandler::SelectDevice(DHTestValues::device2);
	result = DataHandler::UpdateSelectedDeviceData(DHTestValues::device2);
	REQUIRE(result == 0);

	DataHandler::SelectDevice(DHTestValues::device3);
	result = DataHandler::UpdateSelectedDeviceData(DHTestValues::device3);
	REQUIRE(result == 0);
}

TEST_CASE("Get List of entries") {
	int result = 0;
	std::vector<std::unique_ptr<device_data>> deviceDataList;

	InitializeAndFillRouterDb();

	result = DataHandler::GetEntries(deviceDataList);
	REQUIRE(result == 0);

	REQUIRE(deviceDataList[0]->ip == DHTestValues::device1->ip);
	REQUIRE(deviceDataList[1]->ip == DHTestValues::device2->ip);
	REQUIRE(deviceDataList[2]->ip == DHTestValues::device3->ip);
}

TEST_CASE("Get DataVector of Selected device") {
	int result = 0;
	std::unique_ptr<device_data> device;

	InitializeAndFillRouterDb();
	DataHandler::SelectDevice(DHTestValues::device1);

	result = DataHandler::GetDataOfSelectedDevice(device);

	bool isSame = (device->Equals(DHTestValues::device1.get()));

	REQUIRE(isSame);
}

TEST_CASE("Store routing in selected device") {
	int result = 0;

	InitializeAndFillRouterDb();
	DataHandler::SelectDevice(DHTestValues::device1);

	for (DataHandler::Routing routing : DHTestValues::routings1) {
		result = DataHandler::StoreRoutingForSelected(routing);
		if (result) break;
	}

	REQUIRE(result == 0);
}

TEST_CASE("Get routings from selected device") {
	int result = 0;

	InitializeAndFillRouterDb();
	DataHandler::SelectDevice(DHTestValues::device1);
	AddRoutingsToSelectedDevice();

	DataHandler::RoutingsList routings;
	DataHandler::GetRoutesFromSelected(routings);

	for (size_t i = 0; i < routings.size(); i++) {
		if (!(routings[i] == DHTestValues::routings1[i])) {
			result = 1;
			break;
		}
	}

	REQUIRE(result == 0);
}